#pragma once
#include "skyrimSE/bhkSimpleShapePhantom.h"
#include "skyrimSE/bhkRigidBody.h"

#pragma warning( disable : 26440 ) // skse macro is not noexcept

// I'm not exactly sure, but these two phantoms are used in the raycasting call.
// Clearly a hull trace - start and end shape? doesn't make much sense (yet).
struct UnkPhysicsHolder {
	bhkSimpleShapePhantom* unk1;
	bhkSimpleShapePhantom* unk2;
};

class CorrectedPlayerCamera : public TESCamera {
	public:
		CorrectedPlayerCamera();
		virtual ~CorrectedPlayerCamera();

		static CorrectedPlayerCamera* GetSingleton(void) {
			// 0FAF5D3C755F11266ECC496FD392A0A2EA23403B+37
			static RelocPtr<CorrectedPlayerCamera*> g_playerCamera(0x02EC59B8);
			return *g_playerCamera;
		}

		enum {
			kCameraState_FirstPerson = 0,
			kCameraState_AutoVanity,
			kCameraState_VATS,
			kCameraState_Free,
			kCameraState_IronSights,
			kCameraState_Furniture,
			kCameraState_Transition,
			kCameraState_TweenMenu,
			kCameraState_ThirdPerson1,
			kCameraState_ThirdPerson2,
			kCameraState_Horse,
			kCameraState_Bleedout,
			kCameraState_Dragon,
			kNumCameraStates
		};

		uint32_t unk;
		uint32_t magicNumber;
		UInt8 unk38[120];								    // 028

		TESCameraState* cameraStates[kNumCameraStates];	    // 0B8
		UnkPhysicsHolder* physics;							// 120
		bhkRigidBody* rigidBody;							// 128
		UInt64	unk130;										// 130
		UInt32	unk138;										// 138
		float	worldFOV;									// 13C
		float	firstPersonFOV;								// 140
		float   funk1;
		float   funk2;
		float   funk3;
		float   funk4;
		float   lookYaw;
		float   funk6;
		float   funk7;
		//UInt8	unk144[0x160 - 0x144];						// 144
		UInt8	unk160;										// 160
		UInt8	unk161;										// 161
		UInt8	unk162;										// 162 - init'd to 1
		UInt8	unk163;										// 163
		UInt8	unk164;										// 164
		UInt8	unk165;										// 165
		UInt8	pad166[2];									// 166

		MEMBER_FN_PREFIX(CorrectedPlayerCamera);
		DEFINE_MEMBER_FN(UpdateThirdPerson, void, 0x0084D630, bool weaponDrawn);
};

static_assert(offsetof(CorrectedPlayerCamera, cameraStates) == 0xB8);
static_assert(offsetof(CorrectedPlayerCamera, pad166) == 0x166);
static_assert(sizeof(CorrectedPlayerCamera) == sizeof(PlayerCamera));