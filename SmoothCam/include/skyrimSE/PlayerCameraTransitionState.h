#pragma once
class CorrectedPlayerCameraTransitionState : public TESCameraState {
	public:
		CorrectedPlayerCameraTransitionState();
		virtual ~CorrectedPlayerCameraTransitionState();

		uint32_t something;			// 20 (g_gameTime - something) / somethingElse
		uint32_t somethingElse;		// 24 (if (somethingElse != 0.0f))
		TESCameraState* fromState;	// 28
		TESCameraState* toState;	// 30
		float scalar;				// 38	(if (scalar == 0.0f))
		bool unk3C;					// 3C
		bool unk3D;					// 3D
		uint16_t pad3E;
};