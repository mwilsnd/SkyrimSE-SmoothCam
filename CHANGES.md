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