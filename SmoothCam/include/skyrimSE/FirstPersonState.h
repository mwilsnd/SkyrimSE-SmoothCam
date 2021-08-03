#pragma once

class CorrectedFirstPersonState : public TESCameraState {
	public:
		CorrectedFirstPersonState();
		virtual ~CorrectedFirstPersonState();

		PlayerInputHandler inputHandler;
		NiPoint3 lastPosition;
		NiPoint3 vel;
		NiPoint3 damping;
		uint32_t pad54;
		NiAVObject* cameraObj;
		NiNode* fovControl;
		float sittingRotation;
		float unk6C;
		float unk70;
		float currentPitchOfs;
		float targetPitchOfs;
		float unk7C;
		uint32_t unk80;
		bool cameraOverride;
		bool cameraPitchOverride;
		uint16_t unk86;
		uint64_t unk88;
};
static_assert(sizeof(CorrectedFirstPersonState) == 0x90);