# SkyrimSE-SmoothCam
SmoothCam adds a customizable frame-interpolated third-person camera to Skyrim Special Edition.

Notable features include:
* A raycast crosshair for archery and magic combat
* Selectable custom 3D crosshairs (Skyrim style and a dot style)
* Ability to use the HUD crosshair if not using one of the custom 3D styles
* Trajectory prediction and visualization for arrows
* Per-state offset control (X, Y, Z, FOV), further subdivided into groupings for different combat stances
* 22 different interpolation methods which may be chosen for 6 different smoothers (Primary, Local-Space, Separate Z, Offset, Zoom, FOV)
* Per-state and stance interpolator overrides allowing you to fine tune how the camera acts
* Distance clamping control to keep your character in frame while still being able to use lazy camera smoothing
* 6 preset slots for you to save and load completely different configurations on the fly

## Compatibility
SmoothCam is going to have issues with any other mod that tries to position the third-person camera (Other third-person mods, likely some lock-on mods, etc).

The following mods are supported:
* Improved Camera (Only with the reddit release build of Improved Camera beta 4) (Latest AE version claims compatibility)
* Immersive First Person View (Requires the optional file `IFPV Detector Plugin` on the download page)
* Alternate Conversation Camera (sort of)
* Archery Gameplay Overhaul
* True Directional Movement

## Runtime Requirements
SmoothCam requires SKSE64 and [Address Library](https://www.nexusmods.com/skyrimspecialedition/mods/32444). If you wish to use the MCM esp, SkyUI is also required. **SmoothCam only officially supports Skyrim Special Edition runtimes 1.5.97 and 1.6.x (anniversary edition, including 1.6.629)**, other versions are not tested for full functionality and are unsupported.

## Installing
If using one of the pre-compiled releases, use a mod manager and follow the prompts to select the DLL and plugin types you want. If doing a manual install, install either the SSE or AE version of the DLL and the contents of the `ExtraData` folder to `Data/SKSE/Plugins`. Copy the pex script to `Data/Scripts` and copy either the esp or esl variant of the plugin into `Data`.

If installing after a build, copy `SmoothCam.dll` to `Data/SKSE/Plugins` (along with the address library database file and `ExtraData/SmoothCam_FollowBones_Default.txt`), `SmoothCam.esp/esl` to `Data/` and `SmoothCamMCM.pex` to `Data/Scripts`. Enable the esp file if you wish to use the MCM, otherwise the module will generate a json file in the plugins folder you can edit to manually configure the camera.

## Building
Requisites:
* [buck2](https://buck2.build/)
* [7zip](https://www.7-zip.org/)
* [LDC](https://github.com/ldc-developers/ldc/releases)
* A Skyrim install with the Creation Kit and requisite scripts for MCM creation

The release packager will perform a full build, including MCM code generation, in one shot. You'll need Visual Studio 2022 Community, dub and DMD/LDC installed, along with buck2 and 7zip available on your PATH.
Run the complete release packager by running `./package` from the root of the repository. For individual components, read on.

### Module
### Option 1: BUCK2
Start by cloning the repository:
```
git clone https://github.com/mwilsnd/SkyrimSE-SmoothCam.git --recursive
```

From an admin-elevated powershell window, run the following to get a full build environment installed and ready to go:
```
cd SkyrimSE-SmoothCam
./scripts/bootstrap.ps1
```
*Note: This installs (via chocolatey) MSVC 2022 build tools, python, 7zip, dub, ldc, zstandard and fetches the correct version of buck2.*

And then from a normal user-level powershell window:
```
cd SkyrimSE-SmoothCam
python package.py
```
Which will fully build and construct SmoothCam.zip in the repository root for distribution and installation with your mod manager of choice.

You can build just the DLL like so:
```
cd SkyrimSE-SmoothCam
buck2 build --out build-out --config-file buck2/mode/[debug|release][_pre629] :SmoothCam[AE|SSE]
```
You'll find built artifacts in `./build-out`. You may (likely) need to invoke buck2 via `buck2/buck2.exe` from the repository root, if you relied on the bootstrap script to get up and running.

Full Examples:
* `buck2 build --out build-out --config-file buck2/mode/release :SmoothCamAE` produces `build-out/SmoothCamAE.dll`.
* `buck2 build --out build-out --config-file buck2/mode/release_pre629 :SmoothCamAE` produces `build-out/SmoothCamAEPre629.dll`.
* `buck2/buck2.exe build --out build-out --config-file buck2/mode/release :SmoothCamSSE` produces `build-out/SmoothCamSSE.dll`.

### Option 2: CMake
```
cd SkyrimSE-SmoothCam
cmake -B build -S . [-DSKYRIM_SUPPORT_AE=1]
cmake --build build -j16 --config Release
```

### Option 2.a: + Visual Studio
```
cd SkyrimSE-SmoothCam
cmake -B build -S . [-DSKYRIM_SUPPORT_AE=1] -G "Visual Studio 17 2022"
```
Then navigate to the build directory and open the `.sln` to build/debug using Visual Studio.

### MCM:
To build the MCM code generator (`paper`), you will need a D language compiler (DMD, LDC) and optionally VSCode with the code-d extension installed. If using code-d, build targets have already been created for you.
Compile the code generation tool and then navigate to `CodeGen/MCM`, run `run_preprocess.bat` to generate the output file `SmoothCamMCM.psc`. From there just compile the generated code like any normal papyrus script.

*Tip:* `package.py` builds `paper.exe` for you and generates the MCM script.

### Crosshair Models:
To build the crosshair model converter tool (`ModelBaker`), you will need a D language compiler (DMD, LDC) and optionally VSCode with the code-d extension installed. If using code-d, build targets have already been created for you. You will also need a copy of assimp which you have compiled as a static library. Make a folder in `ModelBaker` named `compiled_libs` (or `compiled_libs_debug` if making a debug build) and copy all assimp libraries to that location. Compile the converter tool and then navigate to `CodeGen/ModelBaker` and run `ModelBaker "path/to/input.ply" "output_name".

## FAQ
> I'm getting a fatal error message, or a message related to "REL/Relocation.h".

You either didn't install Address Library or you renamed SkyrimSE.exe. You cannot rename the game EXE if you want to use this mod. If you're sure you have Address Library installed and you haven't renamed the game EXE, other causes can include a newer (or much older) version of the game or another conflicting SKSE plugin which attempts to hook the same game locations as SmoothCam.

> My settings keep being reset when I start the game.

This usually means SmoothCam wasn't able to write your settings file to disk. This can be caused by OS-level filesystem permission issues, maybe some anti-virus software or other filesystem controls/disk quota management. If you installed SkyrimSE in "Program Files", you may be at risk of having this happen to you. If modding SkyrimSE, you should only install it in a custom location, outside of "Program Files". If using a mod manager like Vortex or Mod Organizer 2, make sure your mod manager is installed on the same disk as SkyrimSE.

> My crosshair/sneak meter flickers randomly.

If you have Archery Gameplay Overhaul installed, you need to disable "Bow Camera" and "Bow Crosshair" in AGO's MCM. This can also be caused by HUD mods which hide or move around elements like the crosshair. If possible, disabling these features should resolve the issue.

> I think SmoothCam is causing me to crash.

As of version 1.4, you can enable the general setting "Enable Crash Dump Handler". Select this setting and then restart the game. If you do crash while SmoothCam code is running, you will get a message box informing you of the crash and explaining where to find the crash dump file (SmoothCam_AppCrash.mdmp, in the same folder as SkyrimSE.exe). Make a new github issue and share this mini-dump file so I can try to help fix the problem. This won't catch every single possible cause of SmoothCam potentially causing a crash, but it should catch >99% of them. You can also share .NET Script Framework crash logs in your github issue.

> Under "Compatibility", Improved Camera beta4 shows as either "NOT DETECTED" or "VERSION MISTMATCH" but I have it installed?

You must install the correct version for compatibility features to work with Improved Camera correctly. Only the reddit release DLL of beta 4 is supported.

> Under "Compatibility", Immersive First Person View shows "NOT DETECTED" but I have it installed?

You need to install the optional `IFPV Detector Plugin` on the mod's download page.

> My camera got stuck in a strange state and I can't rotate my camera anymore, what gives?

This is a pretty rare issue, I've only had it happen once myself. You can try the general option "Force camera to thirdperson" which may work. The problem appears to be caused by the camera getting stuck in the "Furniture" camera state.

> The Plugin Info section shows "D3D11 Hooked" is FALSE, or "DLL Version" does not match "MCM Script Version"

This usually means the SKSE plugin failed to load. Check your SKSE log file for any error messages, along with SmoothCam's log file in the same folder. You can try installing the non-AVX version of SmoothCam (SSE version) which may resolve the issue if your CPU doesn't support the AVX instruction set. If the versions don't match, you might have an outdated version of either the script or DLL. Reinstalling the mod fresh should resolve that issue.

> The SmoothCam MCM looks correct and functional, but the camera isn't working.

Two causes: Either the general setting "Disable SmoothCam" is enabled, or you used the "coc" console command from the main menu without loading a save or starting a new game.

> Camera movement is very jittery when rotating around the player character.

This is most commonly caused by local-space interpolation when you have a low frame rate. You can eliminate the jitter by keeping local space interpolation enabled and setting the method to "linear" and both follow rates to "1.0".

> I used the "Load Next Preset" hotkey and now it doesn't work anymore.

Currently, you must set this keybinding in every preset. When you load your next preset if you haven't set this key binding it will be unloaded (or set to a different key).

> My camera randomly swapped to the other side of my character.

You likely pressed the "Shoulder Swap" key unintentionally. You can find this key binding under the misc section in the Following tab.

> I play on linux and the game crashes at startup.

Refer to issue #56 for a solution.
