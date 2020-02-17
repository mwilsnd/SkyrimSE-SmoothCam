# SkyrimSE-SmoothCam
SmoothCam adds a frame-interpolated third-person camera to Skyrim Special Edition. After trying other third-person camera mods I decided to try and make something better, and this is the result. If I actually achieved my goal in making a better camera is up for you to decide.

## Building
To build the project, clone the repo (remember to include `--recurse-submodules`), then run `make_vs2019.bat` or `make_vs2017.bat`.
The build scripts require `premake5`, `git` and `7-Zip` to be installed and available on the system PATH. Currently only Visual Studio Community 2017 and 2019 are supported for building.

Once premake has finished, open the generated Visual Studio solution, select `Debug` or `Release` and select `Build Solution` from the build dropdown.

Built files are placed in `SmoothCam/bin/<target>/SmoothCam`. Be sure to also copy `hudmenu.swf` from the UI folder into `Data/interface` for the 3D crosshair to work. Copy the esp to `Data/` and the pex file to `Data/Scripts`.

To build the papyrus script, you'll need `lua` on the system path. To run the code generation just run `MCM/run_preprocess.bat` which will generate `SmoothCamMCM.psc`.
From there just compile the generated code like any normal papyrus script.

## Runtime Requirements
SmoothCam only requires SKSE64 (2.0.17 (runtime 1.5.97)). SkyUI is optional if using the MCM esp.
The default build settings use AVX instructions. If your processor does not support AVX, you will need to change the build scripts to use SSE instead.

## Installing
Copy `SmoothCam.dll` to `Data/SKSE/Plugins`, `hudmenu.swf` to `Data/interface`, `SmoothCam.esp` to `Data/` and `SmoothCamMCM.pex` to `Data/Scripts`. Enable the esp file if you wish to use the MCM, otherwise the module will generate a json file in the plugins folder you can edit to manually configure the camera.

## Known Issues
When interpolation is very slow, culling of geometry can be seen.

Floating quest markers do not respect the camera position - Interpolation causes them to lazily drift around the screen and camera offsets also offset the markers.

The crosshair is not perfect - This is still a work in progress.

Improved Camera issues - Switching POV on horseback is buggy. Moving from first person to thirdperson must be done via the `Switch POV` key, rather than scrolling the camera.

Improved Camera issues - Sitting is even more buggy. This might be wholly an Improved Camera issue, however once in first person while sitting you cannot re-enter third person.

## Configuration
The easiest way to configure the camera is by using the SmoothCam MCM menu. If you do not have SkyUI, or wish to manually edit the json settings, here is a description of each value:

Setting | Description
--- | ---
enableInterp | Set to 1 to enable camera smoothing, 0 if you don't like the smoothing but still want other features
currentScalar | A value from 0 - 21: The scalar function to use for interpolation
minCameraFollowDistance | The distance the camera follows the player from when at the lowest zoom level
minCameraFollowRate | The amount of camera latency when the camera is close to the player (lower = more latency)
maxCameraFollowRate | The amount of camera latency when the camera is far from the player (higher = less latency)
zoomMaxSmoothingDistance | The distance from camera to player at which MaxCameraFollowRate is used for smoothing (when distance is small, MinCameraFollowRate is used, otherwise is mixed with MaxCameraFollowRate) This results in the camera being more "lazy" when close to the player and more snappy when further away
zoomMul | The maximum zoom-out distance from the player (added to MinFollowDistance)
separateZInterp | Enable the separate Z interpolation settings
separateZScalar | A value from 0 - 21: The scalar function to use for Z interpolation
separateZMaxSmoothingDistance | The distance from camera to player at which separateZMaxFollowRate is used for smoothing (when distance is small, separateZMinFollowRate is used, otherwise is mixed with separateZMaxFollowRate) This results in the camera being more "lazy" when close to the player and more snappy when further away
separateZMinFollowRate | The amount of camera latency when the camera is close to the player (lower = more latency)
separateZMaxFollowRate | The amount of camera latency when the camera is far from the player (higher = less latency)
standingSideOffset | The amount of distance the camera should move to the right when in this state
standingUpOffset | The amount of distance the camera should move up when in this state
standingCombatSideOffset | The amount of distance the camera should move to the right when in this state
standingCombatUpOffset | The amount of distance the camera should move up when in this state
walkingSideOffset | The amount of distance the camera should move to the right when in this state
walkingUpOffset | The amount of distance the camera should move up when in this state
walkingCombatSideOffset | The amount of distance the camera should move to the right when in this state
walkingCombatUpOffset | The amount of distance the camera should move up when in this state
runningSideOffset | The amount of distance the camera should move to the right when in this state
runningUpOffset | The amount of distance the camera should move up when in this state
runningCombatSideOffset | The amount of distance the camera should move to the right when in this state
runningCombatUpOffset | The amount of distance the camera should move up when in this state
sprintingSideOffset | The amount of distance the camera should move to the right when in this state
sprintingUpOffset | The amount of distance the camera should move up when in this state
sprintingCombatSideOffset | The amount of distance the camera should move to the right when in this state
sprintingCombatUpOffset | The amount of distance the camera should move up when in this state
sneakingSideOffset | The amount of distance the camera should move to the right when in this state
sneakingUpOffset | The amount of distance the camera should move up when in this state
sneakingCombatSideOffset | The amount of distance the camera should move to the right when in this state
sneakingCombatUpOffset | The amount of distance the camera should move up when in this state
swimmingSideOffset | The amount of distance the camera should move to the right when in this state
swimmingUpOffset | The amount of distance the camera should move up when in this state
bowAimSideOffset | The amount of distance the camera should move to the right when in this state
bowAimUpOffset | The amount of distance the camera should move up when in this state
bowAimHorseSideOffset | The amount of distance the camera should move to the right when in this state
bowAimHorseUpOffset | The amount of distance the camera should move up when in this state
sittingSideOffset | The amount of distance the camera should move to the right when in this state
sittingUpOffset | The amount of distance the camera should move up when in this state
horsebackSideOffset | The amount of distance the camera should move to the right when in this state
horsebackUpOffset | The amount of distance the camera should move up when in this state
dragonSideOffset | The amount of distance the camera should move to the right when in this state
dragonUpOffset | The amount of distance the camera should move up when in this state
comaptIC_FirstPersonHorse | Enables compat code for dealing with issue when running the Improved Camera mod
comaptIC_FirstPersonDragon | Enables compat code for dealing with issue when running the Improved Camera mod
compatIC_FirstPersonSitting | Enables compat code for dealing with issue when running the Improved Camera mod