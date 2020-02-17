#pragma once
#include "pch.h"

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
	float					unk40[4];					// 40
	UInt32					unk50[3];					// 50
	float					fOverShoulderPosX;			// 5C
	float					fOverShoulderCombatAddY;	// 60
	float					fOverShoulderPosZ;			// 64
	float					unk68[3];					// 68
	float                   cameraZoom;
	float                   cameraLastZoom;
	float					unk74[4];					// 74
	float					unk8C;						// 8C - init'd 7F7FFFFF
	UInt32					unk90[3];					// 90
	float					unk9C;						// 9C - init'd 7F7FFFFF
	UInt64					unkA0;						// A0
	UInt64					unkA8;						// A8
	float					unkB0;						// B0
	UInt32					unkB4[3];					// B4
	float					unkC0[3];					// C0
	float                   lerpLeftScalar;
	float                   alwaysOne;
	float                   cameraYawDotPlayerYaw;
	float					someOtherRotation;			// D8
	UInt8					unkDC[7];					// DC 
	UInt8					padE3[5];					// E3
};
static_assert(sizeof(CorrectedThirdPersonState) == sizeof(ThirdPersonState));