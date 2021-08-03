# Beta 1.4
* Bumped module and MCM version number to 13

**Fixes:**
* Merge pull request #48 from ersh1/master (Compatibility fix for True Directional Movement).
* Github issue #31, Fixed FOV issues with mods like Undeath and Triumvirate (Wildshape)
* Github issue #35, directory enumeration no longer throws on unicode paths.
* Fixed distance clamping causing the camera to jump when strafing while using True Directional Movement.
* Fixed FOV offsets being applied in the map menu.
* Fixed an issue where a setting string might already be in the string cache from another mod, but in the wrong case.
* Fixed an issue that caused the normal game crosshair to disappear.
* Fixed new game not hooking D3D11 until after saving and loading.
* Fixed Improved Camera patch causing some issues - Now requires the offical reddit release version (beta4) for the compatibility fix to work - sorry, but I have to read a memory address inside Improved Camera to achieve a functional compatibility mode. Fixes distance clamping causing the camera to jump around when the player moves off-screen.
* Invalidate crosshair enable status when exiting a dialog menu.
* Camera is more aggressive about moving to the desired goal position on both camera state transitions and after loading a new cell.
* Now dynamically reading arrow tilt angle during runtime - Mods that change these values should work correctly now.

**New Stuff:**
* Added an FOMOD installer and consolidated both versions in 1 download.
* Github issue request #30, Added a new key binding to cycle through saved presets (Keep in mind each saved preset needs this key bound, otherwise when you switch the loaded preset won't have your keybind saved).
* Github issue request #43, Added a new option and key binding to turn the camera on and off.
* Github issue request #44, Added a new setting and key binding to offset the camera height by a set amount via hotkey.
* Now factoring arrow draw time in the arrow trajectory computation. (Should help with inaccuracy reported when using the eagle eye perk)
* Added a new setting under the "General" page which allows you to force the camera state back to third-person, meant to fix rare instances where the camera gets stuck in a different state.
* Added a new setting under the "General" page which resets the crosshair back to sane settings, in an attempt to recover a missing crosshair reported by some users.
* Added new settings to offset the stealth meter/sneak crosshair by a configurable amount - either when the world-space crosshair is active, or at all times while sneaking.
* Crosshair now supports conjuration casting mode - crosshair changes color to a red tint when the player is unable to summon at the desired location.
* Now smoothing POV transition from third to first person.
* Added a custom crash handler which can be enabled via the MCM and will detect when a crash happens inside SmoothCam, creating a mini-dump file. If you have this happen, follow the instructions in the message box to report this issue and share the dump file so the crash may be investigated.
* Added EASTL as a source code/compile dependency.

**Changes:**
* Renamed the "Info" page in the MCM to the "General" page.
* Removed the compatibility page from the MCM, moved compat options to the "General" page.
* Slightly altered the raycasting logic for camera collisions (Fixes camera collision jitter).
* Adjustments to arrow prediction math.
* Increased the allowed size range of the crosshair in the MCM.
* Explicitly looking for the arrow node for the raycast origin now - Accounts for some mods that attach other effects to the arrow before being fired (maybe).
* New external debug console, for printing useful information (in debug builds).
* Update gen_addrmap to look at member_fn_0..N macros.
* Added offset caching, we can now unload the offset DB and free up some more ram.
* Refactored camera into the low-level camera, and Thirdperson+Firstperson cameras owned by the low-level camera.
* Refactored camera states.
* Rewrote the papyrus preprocessor in D, using a more proper tokenizer system.
* Some reverse engineering of Skyrim's renderer, updates to d3d_context.
* Added a shader cache.
* New abstracted detour utilities.
* Updated build scripts.

## Beta 1.3
* Bumped module and MCM version number to 12

**Fixes:**
* Fixes for camera rotation getting messed up when using Alternate Conversation Camera.
* Fixes for settings changing other values in the MCM - oversight in the MCM script design.
* Patches for Wrath of Nature, Apocalypse and Triumvirate failed to merge in last update, now patched for real(tm).
  * Spells not yet compatible: Apocalypse - Mind Vision
* Now looking at a different node for detecting IFPV first-person mode, should be more reliable.
* Extra sanity checking when trying to obtain an NiCamera reference.
* Fixed auto-complete errors using D3D10 enums rather than D3D11.

**New Stuff:**
* Added FOV settings to all offset groups.
* New offset groups for vampire lord and werewolf.
* Added back the npc hit-size crosshair option, now working properly.
* New compat option for Archery Gameplay Overhaul when using 3D crosshair features (You will still need to disable AGO's camera features).
* New compat option for Alternate Conversation Camera. Removed "Disable during dialog" option.
* Reduced Improved Camera compat options to a single on/off, using method similar to IFPV for compat checks.

**Changes:**
* Now only enabling magic offsets for "combat" magic (Ice spike, flames, and so on) - Spells like healing and wards no longer count.
* Enchanted items no longer count as "magic" combat - unless the item is a staff or you also have a "combat" spell equipped.
* Worked some more on horse->thirdperson transition
* Code refactoring and cleanup, general improvements to overall camera behavior.
* Added a warning message when no bones were able to be loaded - In this case the camera will insert a fallback bone and let the game continue.
* Added a warning message when the mod fails to hook directX. Now showing D3D hook status in the info page of the MCM.
* Added extra information to detour failure messages, to indicate more clearly where the problem function is.
* Added more code generation utilities for the MCM.
* Missed a few WITH_D2D guards on some files.
* Added an extra debug overlay for showing in-depth camera state.
* Updated to SKSE64 2.0.19 (SmoothCam will still work fine for users on older SKSE versions).
* Updated Microsoft Detours to master.
* Updated GLM to master.
* Updated nlohmann/json to master.
* Updated PolyHook 2 to master.
* Updated build scripts.

## Beta 1.2
* Bumped module and MCM version number to 11

**Fixes:**
* Added some extra sanity checking in crosshair update code to try and prevent a crash when using bows.
* Fixed mouse buttons not responding to the shoulder switch command.
* Fixed broken ranged kill moves.
* Fixed sneaking offsets taking precedence over bow-aiming offsets.
* Staves now correctly enable the magic crosshair and offset modes.
* Fixes for Wrath of Nature, Apocalypse and Triumvirate (And possibly others).
* Better IFPV compat.
* Made the HUD code less aggressive about setting crosshair parameters, should hopefully be more compatible with HUD mods.
* Added loading-screen detection to prevent manipulation of the HUD during cell transitions - Might reduce the chances of a strange Scaleform crash from happening.
* Fixed camera jitter when closing the tween (inventory) menu.
* Reduced extreme position change when dismounting horse - now preventing the transition camera from running.
* Better detection of POV slide mode (Holding F while in third-person), slide mode no longer interferes with the 3D crosshair position.
* Corrections to crosshair trajectory patch when on horseback.
* Corrected transitions to free cam mode having the wrong position/rotation (maybe).

**New Stuff:**
* Added a new system to allow users to specify a list of bone names for the camera to search for, useful for mods that replace the player skeleton with one using names that don't match the regular skeleton. A more detailed description can be found in the new file "SmoothCam_FollowBones_Default.txt".
* Added smoothing between states when the old state has interpolation enabled and the new one doesn't.
* Added arrow trajectory prediction to the 3D crosshair (Will now account for gravity over longer distances). This option is separate from the normal raycasting method - If you feel like this option is cheating, you can simply ignore it.
* Added an arrow trajectory visualization option, drawing an arc that shows where your arrows should go (Requires enabling new option 'EnableArrowPrediction' combined with the 3D ranged crosshair being enabled, Color can be changed in MCM).
* New 3D crosshair system, independent from the HUD crosshair - Draws genuine 3D crosshair models in game space. A few different styles are included. You can either use this new system or the HUD crosshair - by default, the HUD crosshair is used.
* Added a D utility to convert a 3D mesh into a text-based hex array, for pasting in header files (Note: Building this is not required in any way to build the mod - If you wish to build this tool, at a minimum you'll need dub, DMD, CMake and a built copy of assimp, statically linked).
* Debug overlay (with correct build defines set in pch) to help visualize issues.

**Changes:**
* There has been some confusion about the inclusion of the address library binary file - As of 1.2b, SmoothCam no longer includes this file for you - It is now a genuine dependency.
* Removed the crosshair size hit character setting (for now), until we can better determine if a ray hit a character.
* Located the dxgi swapchain global, dropped the memory scanner.
* Dropped old debug drawing code, wrote new D3D utility code for more general purpose use.
* Random code base housekeeping. Consolidated MCM code generator with rest of the generators.

## Beta 1.1
* Bumped module and MCM version number to 10
* Added a compatibility patch for Immersive First Person View.
* Tweaked Improved Camera compatibility when on horseback.
* Remove dynamic casting for camera state method invocations, not really required.
* Config now reads fNearDistance, in case the player changes it - changing it isn't really advised though if you use Immersive First Person View (We check the expected near value against what it actually is, combined with a distance check, to try and figure out if we are in IFPV's hacked first person mode).
* Config now reads fMinCurrentZoom rather than use the hard coded value.
* Adjusted config defaults to no longer enable compat options - these should be opt-in.
* Using new raycasting method for the crosshair, dropped the jank custom intersection test.
* Fix arrow and magic projectiles spawning behind the player when using the archery patch with SSE Engine Fixes.
* Changed some of the interpolation math to FP64 to gain a bit more precision (might not end up being necessary).
* Switched to SKSE's ITimer over using naked qpc.
* Updated config defaults and added a new MCM option to restore default values.
* As some users have reported local space smoothing causing jitter, the local space smoothing rate now defaults to 1, making it opt-in. Jitter is mostly caused by a sub-60 frame rate, currently looking into options for correcting jitter in a later update.

## Beta 1
* Bumped module and MCM version number to 9
* Promoted to beta.
* Added shoulder swapping via hotkey.
* Added separable option for swapping the distance clamping X axis when swapping shoulders.
* Fixed issue with `preproc_constexpr_struct.lua` incorrectly assuming all `a.b` syntax was a struct macro to be replaced.

## Alpha 1.7
* Bumped module and MCM version number to 8
* Added a "group edit" tab in the MCM, allowing users to edit all offset groups at once.
* Increased the maximum slider range for "MinFollowDistance" from 64 to 256.
* Fixed the crosshair position being wrong with ultrawide aspect ratios.
* Config now reads Skyrim.ini and will use user supplied `f3PArrowTiltUpAngle` and `f3PBoltTiltUpAngle` if found.
* Opt-in option for crosshair size manipulation.
* Added "ZoomOffset" settings to offset groups, allowing the camera to offset the zoom level in/out on different state transitions.
* MCM layout changes - **Users of prior versions may need to disable the esp and make a save after the menu disappears then quit and enable it again for changes to update correctly**
* Simplified the papyrus config binding code.

## Alpha 1.6
* Bumped module and MCM version number to 7
* Removed the need for a custom hudmenu.swf for the 3D crosshair.
* Removed the UI folder and skyrimui dependency.
* Enabled GLM's use of hardware intrinsics rather than let the auto vectorizer deal with it.

## Alpha 1.5
* Bumped module and MCM version number to 6
* Fixed smoothcam not starting during a new game until first saving and reloading.
* Fixed the game's crosshair reference raycasting being off - text prompting you to pick up/activate/talk now correctly lines up with the vanilla crosshair.
* Fixed an oversight in the threaded version of Raycast::IntersectRayAABBAllActorsIn which could cause a crash in extreme cases.
* Changed options for the 3D crosshair, now allowing it to be enabled for magic combat and aiming with the bow separately, removed other options as they aren't really helpful anymore.
* Reverse engineered arrow flight code and patched the game's attempts to try and skew arrows towards the crosshair - this was super broken with custom camera positions and made the 3D crosshair impossible to line up correctly. The crosshair works much better with bows now, though you still need to lead targets accounting for gravity. Arrow flight patch also helps with magic projectiles.
* Changed the bone used for camera positioning on horseback.

## Alpha 1.4
* Bumped module and MCM version number to 5
* Fixed the camera position not continuing to update while in other camera states like firstperson, causing a sudden jump when entering thirdperson.
* Added preset support with 6 save slots.
* Added a new smoothing option for transitioning between different side & up offsets.
* Added a new smoothing option for transitioning between different zoom levels.
* Added a new option to always use the 3D crosshair.
* Added a new option to grow the 3D crosshair by a set amount when intersecting with an NPC.
* Added new options to scale the crosshair based on ray-hit distance.
* Fixed choppy yaw rotation updates, most noticable when interpolation is disabled.
* Removed pitch singularity guards.
* Massive 3D crosshair improvements - much more accurate, basic ray-actor intersection (ray-AABB). Currently there are issues with AABB position and rotation with non-human NPCs and human NPCs which are not standing upright, but works for the most part. This is more of a hold-over for the moment until I can find a better raycasting method in the engine.
* Changed how the 3D crosshair works in the hudmenu, now using a separate crosshair object from the hud base instance, added new scaling methods. I'm now including the flash project (fla) due to these changes.
* Added a line and box drawing system for debug builds to help visualize ray casts and actor AABBs, toggled using the insert key.
* Switched to a different JSON library, new one should tolerate version upgrades much better.
* Tweaked some of the default config values.

## Alpha 1.3
* Bumped module and MCM version number to 4
* Fixed issue #11 causing the player to teleport when using Skyrim Souls RE and the HUD patch.
* Added SKSE Address Library support per issue #9, allowing the mod to be used across many versions of SKSE (In theory). The mod ships with the version database for 1.5.97, refer to https://www.nexusmods.com/skyrimspecialedition/mods/32444 for using this mod with older versions. **Note:** The mod is not guaranteed to work on all versions, things other than simple addresses can change from version to version which may cause crashes or other instabilities. Tested and working on 1.5.97 and 1.5.80.
* Trying out changing the primary camera detour for a collection of virtual function detours. These should be more compatible with other mods and are more maintainable.
* Force include the PCH via the build scripts
* Add some small code rewriting in the build scripts for SKSE, to enable a more transparent implementation of Address library.
* Created a code generator for building a map of SKSE 1.5.97 addresses, mapping them to address library ID numbers. This tool requires LuaJIT to run. Thanks to LuaPower for the support libraries used.
* Revert the original HUD patch, it caused massive issues with geometry culling and shadow maps due to being applied to every NiCamera in the game.
* Added an improved HUD patch, invoking the internal game method used to calculate the world-to-screen matrix. This patch is much safer and thus can always be active, the setting to enable the patch has been removed. This patch also appears to have fixed the geometry culling issue.
* Worked on the crosshair a bit more.

## Alpha 1.2
* Bumped module and MCM version number to 3
* Added distance clamping options which limit how far away the camera may drift from the player along each axis
* Separated player-camera interpolation from camera rotation interpolation (Referred to as local-space interpolation in MCM). This allows different smoothing values for positional and rotational following.
* Added a new patch correcting world-projected HUD elements (Floating quest markers and the like). **Note:** This patch modifies a commonly used function and since I can't be sure anyone else does the same thing, This is an opt-in patch. See the new option in the MCM - Enabling it will require you to restart your game.
* Adjusted crosshair math for bows, now casting the ray from the bow mid-bone position
* MCM layout changes - **Users of prior versions may need to disable the esp and make a save after the menu disappears then quit and enable it again for changes to update correctly**
* Split some game state code out into a separate file
* Fixed a filename typo in MCM/preproc

## Alpha 1.1
* Bumped module and MCM version number to 2
* Added Ranged, Magic and Melee offset settings for standing, walking, running, sprinting, sneaking and horseback
* Added per-state interpolation toggles to standing, walking, running, sprinting, sneaking, swimming, bow aiming, sitting and horseback
* Added an option to hide the crosshair when out of combat
* Added an option to hide the crosshair during melee combat
* Improved the 3D crosshair, still not super happy with it but for sure an improvement
* Added an option to disable delta time factoring in interpolation math - For testing reasons
* Added an option to disable the camera during dialog for conflicting mods
* Code refactoring in the camera classes and the config system
* Git is not required for building, removed the check and updated the readme
* Updated build scripts and added a generator for a non-AVX version of the project