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