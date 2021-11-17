; Oblivion style
DialogueSliderSetting oblivionDialogueMaxFOV -> [
	settingName: "OblivionDialogueMaxFOV"
	displayName: "Max FOV Zoom"
	desc: "The max FOV zoom amount to apply during dialogue."
	defaultValue: 30
	interval: 0.1
	min: 0
	max: 30
	displayFormat: "{1}"
	page: "Oblivion"
]
DialogueSliderSetting oblivionDialogueFOVDurationIn -> [
	settingName: "OblivionDialogueFOVDurationIn"
	displayName: "Zoom In Duration"
	desc: "The amount of time (in seconds) FOV zoom should take."
	defaultValue: 1
	interval: 0.1
	min: 0
	max: 5
	displayFormat: "{1}"
	page: "Oblivion"
]
DialogueSliderSetting oblivionDialogueFOVDurationOut -> [
	settingName: "OblivionDialogueFOVDurationOut"
	displayName: "Zoom Out Duration"
	desc: "The amount of time (in seconds) FOV zoom should take."
	defaultValue: 1
	interval: 0.1
	min: 0
	max: 5
	displayFormat: "{1}"
	page: "Oblivion"
]
DialogueToggleSetting oblivionDialogueRunFPV -> [
	settingName: "OblivionDialogueRunFPV"
	displayName: "Run In Firstperson"
	desc: "Apply this dialogue camera when in firstperson."
	page: "Oblivion"
]

; Face to face
DialogueSliderSetting faceToFaceSideOffset -> [
	settingName: "FaceToFaceDialogueSideOffset"
	displayName: "Side Offset"
	desc: "The amount to move the camera to the right."
	defaultValue: 25.0
	displayFormat: "{0}"
	page: "Face To Face"
]
DialogueSliderSetting faceToFaceUpOffset -> [
	settingName: "FaceToFaceDialogueUpOffset"
	displayName: "Up Offset"
	desc: "The amount to move the camera up."
	displayFormat: "{0}"
	page: "Face To Face"
]
DialogueSliderSetting faceToFaceZoomOffset -> [
	settingName: "FaceToFaceDialogueZoomOffset"
	displayName: "Zoom Offset"
	desc: "The amount to offset the camera zoom by."
	min: -100.0
	max: 100.0
	displayFormat: "{0}"
	page: "Face To Face"
]
DialogueSliderSetting faceToFaceRotationDuration -> [
	settingName: "FaceToFaceDialogueRotationDuration"
	displayName: "Rotation Duration"
	desc: "The amount of time (in seconds) for the camera to rotate towards the speaking actor."
	defaultValue: 1
	interval: 0.1
	min: 0
	max: 5
	displayFormat: "{1}"
	page: "Face To Face"
]
DialogueSliderSetting faceToFaceDurationIn -> [
	settingName: "FaceToFaceDialogueDurationIn"
	displayName: "Zoom In Duration"
	desc: "The amount of time (in seconds) dialogue zoom should take."
	defaultValue: 1
	interval: 0.1
	min: 0
	max: 5
	displayFormat: "{1}"
	page: "Face To Face"
]
DialogueSliderSetting faceToFaceDurationOut -> [
	settingName: "FaceToFaceDialogueDurationOut"
	displayName: "Zoom Out Duration"
	desc: "The amount of time (in seconds) dialogue zoom should take."
	defaultValue: 1
	interval: 0.1
	min: 0
	max: 5
	displayFormat: "{1}"
	page: "Face To Face"
]
DialogueToggleSetting faceToFaceNoSwitch -> [
	settingName: "FaceToFaceNoSwitch"
	displayName: "Disable Rotation"
	desc: "If enabled the camera will simply look at the NPC, never rotating around to face the player. "
	page: "Face To Face"
]
DialogueToggleSetting faceToFaceForceThirdperson -> [
	settingName: "FaceToFaceDialogueForceThirdperson"
	displayName: "Force Thirdperson"
	desc: "If entering dialogue while in firstperson, force the camera into thirdperson for the duration of the conversation. If diabled, this camera will not run when talking in firstperson."
	page: "Face To Face"
]

; Global (not paginated) values
ListSettingRealStorage dialogueMode -> [
	settingName: "DialogueMode"
	displayName: "Dialogue Mode"
	desc: "The type of camera to use for player-NPC conversation."
	arrayType: dialogueModes
	refreshPageOnUpdate: true
	page: " Dialogue"
]