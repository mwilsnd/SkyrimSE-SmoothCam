#pragma once

namespace ArrowFixes {
	struct LaunchData {
		void* vtbl;
		
		RE::NiPoint3 normal;
		RE::NiPoint3 unkVec1;

		RE::BGSProjectile* projectile;
		RE::PlayerCharacter* player;
		RE::NiAVObject* avObject;
		RE::TESObjectWEAP* weapon;
		RE::TESAmmo* ammo;

		float spawnYaw;
		float spawnPitch;

		uintptr_t refCounted;
		uintptr_t unk4;
		uintptr_t unk5;
	};

	struct UnkData {
		// Not sure what is going on in any of this
		void* unk0;
		RE::BGSEncounterZone* zone;
		uintptr_t unk1;
		void* unk2;
		
		struct UnkData1 {
			class LocationCleared;
			class ShoutMastered;

			byte evCleared[sizeof(RE::BSTEventSink<LocationCleared>)];
			const char* name;
			int unk0;
			byte evShoutMastered[sizeof(RE::BSTEventSink<ShoutMastered>)];
		};

		UnkData1* unk3;
		int unk4;
	};

	bool Attach();

#ifdef DEBUG
	void Draw(Render::D3DContext& ctx);
#endif
}