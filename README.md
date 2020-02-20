# SkyrimSE-SmoothCam
SmoothCam adds a frame-interpolated third-person camera to Skyrim Special Edition. After trying other third-person camera mods I decided to try and make something better, and this is the result. If I actually achieved my goal in making a better camera is up for you to decide.

## Compatibility
* SmoothCam is going to have issues with any other mod that tries to position the third-person camera (Other third-person mods, likely some lock-on mods, etc).

* For the raycasting 3D crosshair, __SmoothCam replaces__ `hudmenu.swf`, meaning any other mod that changes the hud will likely have issues. (If you have issues but still want to use this mod you can simply omit `hudmenu.swf` when installing - the mod will still function, just without the 3D crosshair).
Mod authors of conflicting mods can add support for the crosshair by adding a `SetCrosshairPosition` method inside `hudmenu/HUDMenu.as` (Refer to the changes I've made to `hudmenu` inside the UI folder).

* As I use ImprovedCamera myself, I've been slowly adding some compatibility patches for it over time. Note that currently interactions between both mods aren't perfect.

* Alternate Conversation Camera is supported, just make sure "Disable During Dialog" is selected in the MCM.

## Runtime Requirements
SmoothCam only requires SKSE64 (2.0.17 (runtime 1.5.97)). SkyUI is optional if using the MCM esp.

#### Notice:
Depending on the age of your processor (~2011 and earlier), you might need to use the SSE version (Streaming SIMD Extensions). Most people can use the normal version without issue.

## Installing
If using one of the pre-compiled releases, just copy the `Data` folder in the archive into your skyrim folder or install via a mod manager.
If installing after a build, copy `SmoothCam.dll` to `Data/SKSE/Plugins`, `hudmenu.swf` to `Data/interface`, `SmoothCam.esp` to `Data/` and `SmoothCamMCM.pex` to `Data/Scripts`. Enable the esp file if you wish to use the MCM, otherwise the module will generate a json file in the plugins folder you can edit to manually configure the camera.

## Building
To build the project, clone the repo (remember to include `--recurse-submodules`), then run `make_vs2019.bat` or `make_vs2017.bat`.
The build scripts require `premake5` and `7-Zip` to be installed and available on the system PATH. Currently only Visual Studio Community 2017 and 2019 are supported for building.

Once premake has finished, open the generated Visual Studio solution, select `Debug` or `Release` and select `Build Solution` from the build dropdown.

Built files are placed in `SmoothCam/bin/<target>/SmoothCam`. Be sure to also copy `hudmenu.swf` from the UI folder into `Data/interface` for the 3D crosshair to work. Copy the esp to `Data/` and the pex file to `Data/Scripts`.

To build the papyrus script, you'll need `lua` on the system path. To run the code generation just run `MCM/run_preprocess.bat` which will generate `SmoothCamMCM.psc`.
From there just compile the generated code like any normal papyrus script.

## Known Issues
When interpolation is very slow, culling of geometry can be seen.

Floating quest markers do not respect the camera position - Interpolation causes them to lazily drift around the screen and camera offsets also offset the markers.

The crosshair is still being refined, though as of alpha 1.1 is much improved.

Improved Camera issues - Switching POV on horseback is buggy. Moving from first person to thirdperson must be done via the `Switch POV` key, rather than scrolling the camera.

Improved Camera issues - Sitting is even more buggy. This might be wholly an Improved Camera issue, however once in first person while sitting you cannot re-enter third person.

## Configuration
The easiest way to configure the camera is by using the SmoothCam MCM menu. If you do not have SkyUI, or wish to manually edit the json settings, here is a description of each value:

Setting | Description
--- | ---
enableInterp | Enables camera smoothing globally
enable3DCrosshair | Enables the raycasted crosshair
hideNonCombatCrosshair | Hides the crosshair when no weapon is drawn
hideCrosshairMeleeCombat | Hides the crosshair when melee weapons are drawn
disableDeltaTime | Disables frame time factoring in the smoothing math
disableDuringDialog | Disables SmoothCam during character-NPC conversations
currentScalar | A value from 0 - 21: The scalar function to use for interpolation
comaptIC_FirstPersonHorse | Enables compat code for dealing with issue when running the Improved Camera mod
comaptIC_FirstPersonDragon | Enables compat code for dealing with issue when running the Improved Camera mod
compatIC_FirstPersonSitting | Enables compat code for dealing with issue when running the Improved Camera mod
minCameraFollowDistance | The distance the camera follows the player from when at the lowest zoom level
minCameraFollowRate | The amount of camera latency when the camera is close to the player (lower = more latency)
maxCameraFollowRate | The amount of camera latency when the camera is far from the player (higher = less latency)
zoomMul | The maximum zoom-out distance from the player (added to MinFollowDistance)
zoomMaxSmoothingDistance | The distance from camera to player at which MaxCameraFollowRate is used for smoothing (when distance is small, MinCameraFollowRate is used, otherwise is mixed with MaxCameraFollowRate) This results in the camera being more "lazy" when close to the player and more snappy when further away
separateZInterp | Enable the separate Z interpolation settings
separateZMaxSmoothingDistance | The distance from camera to player at which separateZMaxFollowRate is used for smoothing (when distance is small, separateZMinFollowRate is used, otherwise is mixed with separateZMaxFollowRate) This results in the camera being more "lazy" when close to the player and more snappy when further away
separateZMinFollowRate | The amount of camera latency when the camera is close to the player (lower = more latency)
separateZMaxFollowRate | The amount of camera latency when the camera is far from the player (higher = less latency)
separateZScalar | A value from 0 - 21: The scalar function to use for Z interpolation
standing | The offset collection to use when standing
walking | The offset collection to use when walking
running | The offset collection to use when running
sprinting | The offset collection to use when sprinting
sneaking | The offset collection to use when sneaking
swimming | The offset collection to use when swimming
bowAim | The offset collection to use when aiming with a bow
sitting | The offset collection to use when sitting
horseback | The offset collection to use when on horseback
dragon | Not yet implemented

An offset group is just a collection of common values used for a camera state. Note that not all values are used for each state.

OffsetSetting | Description
--- | ---
sideOffset | The amount to move the camera to the right
upOffset | The amount to move the camera up
combatRangedSideOffset | The amount to move the camera to the right during ranged combat
combatRangedUpOffset | The amount to move the camera up during ranged combat
combatMagicSideOffset | The amount to move the camera to the right during magic combat
combatMagicUpOffset | The amount to move the camera up during magic combat
combatMeleeSideOffset | The amount to move the camera to the right during melee combat
combatMeleeUpOffset | The amount to move the camera up during melee combat
horseSideOffset | The amount to move the camera to the right when on horseback
horseUpOffset | The amount to move the camera up when on horseback
interp | Enable smoothing during this state
interpRangedCombat | Enable smoothing during this state when in ranged combat
interpMagicCombat | Enable smoothing during this state when in magic combat
interpMeleeCombat | Enable smoothing during this state when in melee combat
interpHorseback | Enable smoothing during this state when on horseback