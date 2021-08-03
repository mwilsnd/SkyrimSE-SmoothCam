# SkyrimSE-SmoothCam
SmoothCam adds a customizable frame-interpolated third-person camera to Skyrim Special Edition.

Notable features include:
* A raycast crosshair for archery and magic combat
* Selectable custom 3D crosshairs (Skyrim style and a dot style)
* Ability to use the HUD crosshair if not using one of the custom 3D styles
* Trajectory prediction and visualization for arrows
* Per-state offset control (X, Y, Z, FOV), further subdivided into groupings for different combat stances
* 22 different interpolation methods which may be chosen for 6 different smoothers (Primary, Local-Space, Separate Z, Offset, Zoom, FOV)
* Distance clamping control to keep your character in frame while still being able to use lazy camera smoothing
* 6 preset slots for you to save and load completely different configurations on the fly

## Compatibility
SmoothCam is going to have issues with any other mod that tries to position the third-person camera (Other third-person mods, likely some lock-on mods, etc).

The following mods are supported, with patches that can be enabled in the MCM:
* Improved Camera (Only with the reddit release build of Improved Camera beta 4)
* Immersive First Person View
* Alternate Conversation Camera
* Archery Gameplay Overhaul

True Directional Movement is compatible and fully supported.

## Runtime Requirements
SmoothCam requires SKSE64 and [Address Library](https://www.nexusmods.com/skyrimspecialedition/mods/32444). If you wish to use the MCM esp, SkyUI is also required. **SmoothCam only officially supports Skyrim Special Edition runtime 1.5.97**, other versions are not tested for full functionality and are unsupported.

#### Notice:
Depending on the age of your processor (~2011 and earlier), you might need to use the SSE version (Streaming SIMD Extensions). Most people can use the normal version without issue.

## Installing
If using one of the pre-compiled releases, use a mod manager and follow the prompts to select the DLL and plugin types you want. If doing a manual install, install either the AVX or SSE version of the DLL and the contents of the `ExtraData` folder to `Data/SKSE/Plugins`. Copy the pex script to `Data/Scripts` and copy either the esp or esl variant of the plugin into `Data`.

If installing after a build, copy `SmoothCam.dll` to `Data/SKSE/Plugins` (along with the address library database file and `ExtraData/SmoothCam_FollowBones_Default.txt`), `SmoothCam.esp/esl` to `Data/` and `SmoothCamMCM.pex` to `Data/Scripts`. Enable the esp file if you wish to use the MCM, otherwise the module will generate a json file in the plugins folder you can edit to manually configure the camera.

## Building
To build the project, clone the repo (remember to include `--recurse-submodules`), then run `make_vs2019.bat` or `make_vs2017.bat`.
The build scripts require `premake5` and `7-Zip` to be installed and available on the system PATH. Currently only Visual Studio Community 2017 and 2019 are supported for building.

Once premake has finished, open the generated Visual Studio solution, select `Debug` or `Release` and select `Build Solution` from the build dropdown.

Built files are placed in `SmoothCam/bin/<target>/SmoothCam`.

To generate the offset map, run `lua run.lua "path/to/skse/src"` from the directory `CodeGen/gen_addrmap`. You will need `offsets.txt` in the same folder, which can be exported from address library.

To build the papyrus script for the MCM, navigate to `CodeGen/MCM` and run `run_preprocess.bat` to generate the output file `SmoothCamMCM.psc`. From there just compile the generated code like any normal papyrus script.

To generate crosshair model file headers, navigate to `CodeGen/ModelBaker` and run `ModelBaker "path/to/input.ply" "output_name".

To compile the MCM tool (`paper`) and crosshair model converter (`ModelBaker`), you will need dub, DMD or LDC and optionally VSCode with the code-d extension. Both projects have build targets pre-configured for VSCode.
For `ModelBaker`, you will also need a copy of assimp which you have compiled as a static library. Make a folder in `ModelBaker` named `compiled_libs` (or `compiled_libs_debug` if making a debug build) and copy all assimp libraries to that location.
Crosshair models use vertex colors rather than textures, therefore use of the stanford ply model format as input to the converter tool is recommended.

## FAQ
> My crosshair/sneak meter flickers randomly.

If you have Archery Gameplay Overhaul installed, you need to disable "Bow Camera" and "Bow Crosshair" in AGO's MCM. Also enable the compatibility for AGO in SmoothCam's MCM. This can also be caused by HUD mods which hide or move around elements like the crosshair. If possible, disabling these features should resolve the issue.

> I think SmoothCam is causing me to crash.

As of version 1.4, you can enable the general setting "Enable Crash Dump Handler". Select this setting and then restart the game. If you do crash while SmoothCam code is running, you will get a message box informing you of the crash and explaining where to find the crash dump file (SmoothCam_AppCrash.mdmp, in the same folder as SkyrimSE.exe). Make a new github issue and share this mini-dump file so I can try to help fix the problem. This won't catch every single possible cause of SmoothCam potentially causing a crash, but it should catch >99% of them. You can also share .NET Script Framework crash logs in your github issue.

> Under "Compatibility Options", Improved Camera beta4 shows as either "NOT DETECTED" or "VERSION MISTMATCH" but I have it installed?

You must install the correct version for compatibility features to work with Improved Camera correctly. Only the reddit release DLL of beta 4 is supported.

> My camera got stuck in a strange state and I can't rotate my camera anymore, what gives?

This is a pretty rare issue, I've only had it happen once myself. You can try the general option "Force camera to thirdperson" which may work. The problem appears to be caused by the camera getting stuck in the "Furniture" camera state.

> The Plugin Info section shows "D3D11 Hooked" is FALSE, or "DLL Version" does not match "MCM Script Version"

This usually means the SKSE plugin failed to load. Check your SKSE log file for any error messages, along with SmoothCam's log file in the same folder. You can try installing the non-AVX version of SmoothCam (SSE version) which may resolve the issue if your CPU doesn't support the AVX instruction set. If the versions don't match, you might have an outdated version of either the script or DLL. Reinstalling the mod fresh should resolve that issue.

> The SmoothCam MCM looks correct and functional, but the camera isn't working.

Two causes: Either the general setting "Disable SmoothCam" is enabled, or you used the "coc" console command from the main menu without loading a save or starting a new game.

> Camera movement is very jittery when rotating around the player character.

This is most commonly caused by local-space interpolation when you have a low frame rate. You can eliminate the jitter by keeping local space interpolation enabled and setting the method to "linear" and follow rate to "1.0".

> I used the "Load Next Preset" hotkey and now it doesn't work anymore.

Currently, you must set this keybinding in every preset. When you load your next preset if you haven't set this key binding it will be unloaded (or set to a different key).

> My camera randomly swapped to the other side of my character.

You likely pressed the "Shoulder Swap" key unintentionally. You can find this key binding under the misc section in the Following tab.


## Configuration
The easiest way to configure the camera is by using the SmoothCam MCM menu. If you do not have SkyUI, or wish to manually edit the json settings, here is a description of each value:

Setting | Description
--- | ---
use3DBowAimCrosshair | Use the raycasting crosshair when aiming with a bow
use3DMagicCrosshair | Use the raycasting crosshair when aiming with combat magic
hideNonCombatCrosshair | Hide the crosshair when not in a combat stance
hideCrosshairMeleeCombat| Hide the crosshair when in a melee-only combat stance
enableCrosshairSizeManip | Enable size manipulation of the crosshair
crosshairNPCHitGrowSize | When the 3D crosshair is over an actor, grow the size of the crosshair by this amount
crosshairMinDistSize | Sets the size of the 3D crosshair when the player's aim ray is at the maximum distance
crosshairMaxDistSize | Sets the size of the 3D crosshair when the player's aim ray is at the minimum distance
useWorldCrosshair | Use the world-space 3D crosshair model rather than the HUD crosshair
worldCrosshairDepthTest | Enables depth-testing (occlusion) of the world-space crosshair
worldCrosshairType | The crosshair style to use when using the world-space crosshair
stealthMeterXOffset | Offset the stealth meter by this value along the X axis
stealthMeterYOffset | Offset the stealth meter by this value along the Y axis
offsetStealthMeter | Offset the stealth meter when the world-space crosshair is active
alwaysOffsetStealthMeter | Always offset the stealth meter
useArrowPrediction | Enable arrow trajectory prediction
drawArrowArc | When using arrow prediction, draws the predicted trajectory as an arc
arrowArcColor | The RGBA value to use for coloring the trajectory arc
maxArrowPredictionRange | The maximum range to allow trajectory prediction to run - Very long ranges can cause lag as trajectory computation is on the expensive side
disableDeltaTime | Removes frame-time from the interpolation math. This may or may not result in a smoother camera motion depending on your system and frame rate
nextPresetKey | Key code to use for cycling to the next preset. All presets need this key set or your key will be unloaded or set to something different when switching
shoulderSwapKey | Key code to toggle shoulder swapping
swapXClamping | When swapping shoulders, will also swap the X axis of your distance clamping configuration
modDisabled | When enabled, disables SmoothCam and allows the normal game camera to run
modToggleKey | Key code to use for toggling modDisabled
customZOffset | A custom Z height offset to use with the accompanying toggle key
applyZOffsetKey | Key code to toggle the customZOffset
enableCrashDumps | When enabled, SmoothCam will install a vectored exception handler at game startup and catch crashes that happen while SmoothCam code is present in the call stack. Writes a mini-dump (mdmp) to help find the cause.
compatIC | Enables Improved Camera beta 4 compatibility
compatIFPV | Enables Immersive First Person View compatibility
compatAGO | Enables Archery Gameplay Overhaul compatibility
compatACC | Enables Alternate Conversation Camera compatibility
enableInterp | Enables world position interpolation
currentScalar | Selectable scalar method to use for world interpolation
minCameraFollowDistance | The distance the camera follows the player from when at the lowest zoom level
minCameraFollowRate | The amount of camera latency when the camera is close to the player (lower = more latency)
maxCameraFollowRate | The amount of camera latency when the camera is far from the player (higher = less latency)
zoomMul | The maximum zoom-out distance from the player (added to MinFollowDistance)
zoomMaxSmoothingDistance | The distance from camera to player at which MaxCameraFollowRate is used for smoothing (when distance is small, MinCameraFollowRate is used, otherwise is mixed with MaxCameraFollowRate) This results in the camera being more "lazy" when close to the player and more snappy when further away
separateLocalInterp | Enables interpolation separately for character-local space
separateLocalScalar | Selectable scalar method to use for separate local interpolation
localScalarRate | The smoothing rate to use for separate local interpolation
separateZInterp | Enables interpolation separately for world position on the Z axis
separateZScalar | Selectable scalar method to use for separate Z interpolation
separateZMaxSmoothingDistance | The distance from camera to player at which separateZMaxFollowRate is used for smoothing (when distance is small, separateZMinFollowRate is used, otherwise is mixed with separateZMaxFollowRate) This results in the camera being more "lazy" when close to the player and more snappy when further away
separateZMinFollowRate | The amount of camera latency when the camera is close to the player (lower = more latency)
separateZMaxFollowRate | The amount of camera latency when the camera is far from the player (higher = less latency)
enableOffsetInterpolation | Enables interpolation for offset group transitions
offsetScalar | Selectable scalar method to use for offset interpolation
offsetInterpDurationSecs | Duration smoothing between changes in offset should take place
enableZoomInterpolation | Enables interpolation for zoom transitions
zoomScalar | Selectable scalar method to use for zoom interpolation
zoomInterpDurationSecs | Duration smoothing between changes in zoom should take place
enableFOVInterpolation | Enables interpolation for FOV transitions
fovScalar | Selectable scalar method to use for FOV interpolation
fovInterpDurationSecs | Duration smoothing between changes in FOV should take place
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