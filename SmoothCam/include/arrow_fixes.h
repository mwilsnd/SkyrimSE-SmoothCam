#pragma once

namespace ArrowFixes {
	struct LaunchData {
		void* vtbl;
		NiPoint3 unkVec1;
		float pad1;

		uintptr_t unk1;
		BGSProjectile* projectile;
		PlayerCharacter* player;
		uintptr_t unk2;
		TESObjectWEAP* weapon;
		TESAmmo* ammo;

		float spawnYaw;
		float spawnPitch;

		uintptr_t unk3;
		uintptr_t unk4;
		uintptr_t unk5;
	};

	bool Attach();
}