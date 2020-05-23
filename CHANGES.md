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