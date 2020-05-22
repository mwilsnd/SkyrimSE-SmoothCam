#pragma once

class CorrectedThirdPersonState : public TESCameraState
{
public:
	CorrectedThirdPersonState();
	virtual ~CorrectedThirdPersonState();
	virtual void Unk_09(void);						    // 0x48
	virtual void Unk_0A(void);						    // 0x50
	virtual void UpdateMode(bool weaponDrawn);		    // 0x58

	PlayerInputHandler		inputHandler;				// 20
	NiNode*                 cameraNode;				    // 30
	NiNode*                 controllerNode;			    // 38
	NiPoint3				translation;				// 40
	NiQuaternion			rotation;					// 50
	float					fOverShoulderPosX;			// 5C
	float					fOverShoulderCombatAddY;	// 60
	float					fOverShoulderPosZ;			// 64
	NiPoint3				offsetVector;				// 68
	float                   cameraZoom;
	float                   cameraLastZoom;
	float					yaw1;
	float					yaw2;
	float					zoom;
	float					pitchRelated;
	float					unk8C;						// 8C - init'd 7F7FFFFF
	UInt32					unk90[3];					// 90
	float					unk9C;						// 9C - init'd 7F7FFFFF
	UInt64					unkA0;						// A0
	UInt64					unkA8;						// A8
	float					somethingSprintRelated;		// B0
	UInt32					unkB4[3];					// B4
	float					cameraShake[3];				// C0
	float                   lerpLeftScalar;
	float                   somethingElseSprintRelated;
	float                   yaw;
	float					pitch;						// D8
	bool					freeRotationEnabled;		// DC
	bool					stateNotActive;				// DD
	UInt16					unkDC;						// DE
	bool					toggleAnimCam;				// E0
	bool					applyOffsets;				// E1
	UInt16					unkE2;						// E2
	UInt32					unkE4;						// E4
};
static_assert(sizeof(CorrectedThirdPersonState) == sizeof(ThirdPersonState));