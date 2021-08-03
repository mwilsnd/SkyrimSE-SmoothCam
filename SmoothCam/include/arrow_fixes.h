#pragma once
#ifdef DEBUG
#include "render/d3d_context.h"
#include "render/line_drawer.h"
#endif

namespace ArrowFixes {
	struct LaunchData {
		void* vtbl;
		
		NiPoint3 normal;
		NiPoint3 unkVec1;

		BGSProjectile* projectile;
		PlayerCharacter* player;
		NiAVObject* avObject;
		TESObjectWEAP* weapon;
		TESAmmo* ammo;

		float spawnYaw;
		float spawnPitch;

		uintptr_t refCounted;
		uintptr_t unk4;
		uintptr_t unk5;
	};

	struct UnkData {
		// Not sure what is going on in any of this
		void* unk0;
		BGSEncounterZone* zone;
		uintptr_t unk1;
		void* unk2;
		
		struct UnkData1 {
			BSTEventSink<LocationCleared> evCleared;
			const char* name;
			int unk0;
			BSTEventSink<ShoutMastered> evShoutMastered;
		};

		UnkData1* unk3;
		int unk4;
	};

	bool Attach();

#ifdef DEBUG
	void Draw(Render::D3DContext& ctx);
	void Shutdown();
#endif
}