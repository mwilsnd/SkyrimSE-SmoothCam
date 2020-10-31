# SkyrimSE-SmoothCam
SmoothCam adds a frame-interpolated third-person camera to Skyrim Special Edition. After trying other third-person camera mods I decided to try and make something better, and this is the result. If I actually achieved my goal in making a better camera is up for you to decide.

## Compatibility
SmoothCam is going to have issues with any other mod that tries to position the third-person camera (Other third-person mods, likely some lock-on mods, etc).

The following mods are supported, with patches that can be enabled in the MCM:
* Improved Camera
* Immersive First Person View
* Alternate Conversation Camera
* Archery Gameplay Overhaul

## Runtime Requirements
SmoothCam requires SKSE64 and [Address Library](https://www.nexusmods.com/skyrimspecialedition/mods/32444). If you wish to use the MCM esp, SkyUI is also required.

#### Notice:
Depending on the age of your processor (~2011 and earlier), you might need to use the SSE version (Streaming SIMD Extensions). Most people can use the normal version without issue.

## Installing
If using one of the pre-compiled releases, just copy the `Data` folder in the archive into your skyrim folder or install via a mod manager.

If installing after a build, copy `SmoothCam.dll` to `Data/SKSE/Plugins` (along with the address library database file and `ExtraData/SmoothCam_FollowBones_Default.txt`), `SmoothCam.esp` to `Data/` and `SmoothCamMCM.pex` to `Data/Scripts`. Enable the esp file if you wish to use the MCM, otherwise the module will generate a json file in the plugins folder you can edit to manually configure the camera.

## Building
To build the project, clone the repo (remember to include `--recurse-submodules`), then run `make_vs2019.bat` or `make_vs2017.bat`.
The build scripts require `premake5` and `7-Zip` to be installed and available on the system PATH. Currently only Visual Studio Community 2017 and 2019 are supported for building.

Once premake has finished, open the generated Visual Studio solution, select `Debug` or `Release` and select `Build Solution` from the build dropdown.

Built files are placed in `SmoothCam/bin/<target>/SmoothCam`.

To build the papyrus script, you'll need `lua` on the system path. To run the code generation just run `MCM/run_preprocess.bat` which will generate `SmoothCamMCM.psc`.
From there just compile the generated code like any normal papyrus script.

To generate the offset map, run `lua run.lua "path/to/skse/src"` from the directory `CodeGen/gen_addrmap`. You will need `offsets.txt` in the same folder, which can be exported from address library.

To generate crosshair model file headers, navigate to `CodeGen/ModelBaker` and run `ModelBaker "path/to/input.ply" "output_name".

To build the model baker, you will need dub, DMD or LDC, a copy of assimp which you have compiled as a static library and optionally VSCode with the code-d extension. Make a folder in `ModelBaker` named `compiled_libs` (or `compiled_libs_debug` if making a debug build) and copy all assimp libraries to that location.

## Configuration
The easiest way to configure the camera is by using the SmoothCam MCM menu. If you do not have SkyUI, or wish to manually edit the json settings, here is a description of each value:

Setting | Description
--- | ---
enableInterp | Enables camera smoothing globally
use3DBowAimCrosshair | Enables the raycasted crosshair while aiming with a bow
use3DMagicCrosshair | Enables the raycasted crosshair when magic is equiped
hideNonCombatCrosshair | Hides the crosshair when no weapon is drawn
hideCrosshairMeleeCombat | Hides the crosshair when melee weapons are drawn
enableCrosshairSizeManip | Enable size manipulation of the crosshair
crosshairNPCHitGrowSize | When the 3D crosshair is over an NPC, grow the size of the crosshair by this amount
crosshairMinDistSize | Sets the size of the 3D crosshair when the player's aim ray is at the maximum distance
crosshairMaxDistSize | Sets the size of the 3D crosshair when the player's aim ray is at the minimum distance
useWorldCrosshair | Use a custom 3D rendered crosshair instead of the HUD crosshair
worldCrosshairDepthTest | Enable depth testing when using the world crosshair
worldCrosshairType | The style of crosshair to use when using the world crosshair mode
useArrowPrediction | Enable arrow trajectory prediction
drawArrowArc | Draw an arc when using arrow prediction
arrowArcColor | The color of the arrow arc
maxArrowPredictionRange | The maximum range to run arrow trajectory prediction code
disableDeltaTime | Disables frame time factoring in the smoothing math
compatIC | Enables compat code for dealing with issue when running the Improved Camera mod
compatACC | Enables compat code for dealing with issues when running the Alternate Conversation Camera mod
compatIFPV | Enables compat code for dealing with issues when running the Immersive First Person View mod
compatAGO | Enables compat code for dealing with issues when running the Archery Gameplay Overhaul mod
shoulderSwapKey | The key code used for swapping the X axis when pressed
swapXClamping | When swapping the shoulder/X axis, also flips the distance clamping X values
currentScalar | A value from 0 - 21: The scalar function to use for interpolation
minCameraFollowDistance | The distance the camera follows the player from when at the lowest zoom level
minCameraFollowRate | The amount of camera latency when the camera is close to the player (lower = more latency)
maxCameraFollowRate | The amount of camera latency when the camera is far from the player (higher = less latency)
zoomMul | The maximum zoom-out distance from the player (added to MinFollowDistance)
zoomMaxSmoothingDistance | The distance from camera to player at which MaxCameraFollowRate is used for smoothing (when distance is small, MinCameraFollowRate is used, otherwise is mixed with MaxCameraFollowRate) This results in the camera being more "lazy" when close to the player and more snappy when further away
separateLocalInterp | Enable separate local-space interpolation settings
separateLocalScalar | A value from 0 - 21: The scalar function to use for local-space interpolation
localScalarRate | The constant smoothing rate for local-space interpolation
separateZInterp | Enable the separate Z interpolation settings
separateZMaxSmoothingDistance | The distance from camera to player at which separateZMaxFollowRate is used for smoothing (when distance is small, separateZMinFollowRate is used, otherwise is mixed with separateZMaxFollowRate) This results in the camera being more "lazy" when close to the player and more snappy when further away
separateZMinFollowRate | The amount of camera latency when the camera is close to the player (lower = more latency)
separateZMaxFollowRate | The amount of camera latency when the camera is far from the player (higher = less latency)
separateZScalar | A value from 0 - 21: The scalar function to use for Z interpolation
enableOffsetInterpolation | Enable smoothing of camera offset state transitions
offsetScalar | A value from 0 - 21: The scalar method to use for offset transition smoothing
offsetInterpDurationSecs | The smoothing duration to use when the camera changes offsets (In seconds)
enableZoomInterpolation | Enable smoothing of camera zoom state transitions
zoomScalar | A value from 0 - 21: The scalar method to use for zoom transition smoothing
zoomInterpDurationSecs | The smoothing duration to use when the camera changes zoom distance (In seconds)
enableFOVInterpolation | Enable smoothing of camera fov state transitions
fovScalar | A value from 0 - 21: The scalar method to use for fov transition smoothing
fovInterpDurationSecs | The smoothing duration to use when the camera changes fov (In seconds)
cameraDistanceClampXEnable | Enable distance clamping on the local X axis
cameraDistanceClampXMin | The minimum value the camera position may reach on the X axis
cameraDistanceClampXMax | The maximum value the camera position may reach on the X axis
cameraDistanceClampYEnable | Enable distance clamping on the local Y axis
cameraDistanceClampYMin | The minimum value the camera position may reach on the Y axis
cameraDistanceClampYMax | The maximum value the camera position may reach on the Y axis
cameraDistanceClampZEnable | Enable distance clamping on the local Z axis
cameraDistanceClampZMin | The minimum value the camera position may reach on the Z axis
cameraDistanceClampZMax | The maximum value the camera position may reach on the Z axis
standing | The offset collection to use when standing
walking | The offset collection to use when walking
running | The offset collection to use when running
sprinting | The offset collection to use when sprinting
sneaking | The offset collection to use when sneaking
swimming | The offset collection to use when swimming
bowAim | The offset collection to use when aiming with a bow
sitting | The offset collection to use when sitting
horseback | The offset collection to use when on horseback
vampireLord | The offset collection to use when playing as a vampire lord
werewolf | The offset collection to use when playing as a werewolf
dragon | Not yet implemented

An offset group is just a collection of common values used for a camera state. Note that not all values are used for each state.

OffsetSetting | Description
--- | ---
sideOffset | The amount to move the camera to the right
upOffset | The amount to move the camera up
zoomOffset | The amount to zoom the camera by
fovOffset | The amount to offset the FOV by
combatRangedSideOffset | The amount to move the camera to the right during ranged combat
combatRangedUpOffset | The amount to move the camera up during ranged combat
combatRangedZoomOffset | The amount to zoom the camera by during ranged combat
combatRangedFOVOffset | The amount to offset the FOV by during ranged combat
combatMagicSideOffset | The amount to move the camera to the right during magic combat
combatMagicUpOffset | The amount to move the camera up during magic combat
combatMagicZoomOffset | The amount to zoom the camera by during magic combat
combatMagicFOVOffset | The amount to offset the FOV by during magic combat
combatMeleeSideOffset | The amount to move the camera to the right during melee combat
combatMeleeUpOffset | The amount to move the camera up during melee combat
combatMeleeZoomOffset | The amount to zoom the camera by during melee combat
combatMeleeFOVOffset | The amount to offset the FOV by during melee combat
horseSideOffset | The amount to move the camera to the right when on horseback
horseUpOffset | The amount to move the camera up when on horseback
horseZoomOffset | The amount to zoom the camera by when on horseback
horseFOVOffset | The amount to offset the FOV by when on horseback
interp | Enable smoothing during this state
interpRangedCombat | Enable smoothing during this state when in ranged combat
interpMagicCombat | Enable smoothing during this state when in magic combat
interpMeleeCombat | Enable smoothing during this state when in melee combat
interpHorseback | Enable smoothing during this state when on horseback