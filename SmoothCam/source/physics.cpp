#include "pch.h"

namespace {
	typedef bhkWorld*(__fastcall* bhkWorldGetter)(const TESObjectCELL* cell);
	RelocAddr<bhkWorldGetter> GetBhkWorld(0x2654c0);
	
	constexpr auto hkpBroadphaseOffset = 0x88;
}

hkp3AxisSweep* Physics::GetBroadphase(const bhkWorld* physicsWorld) {
	auto ptr = reinterpret_cast<hkp3AxisSweep**>(physicsWorld->unk40() + hkpBroadphaseOffset);
	return *ptr;
}

bhkWorld* Physics::GetWorld(const TESObjectCELL* parentCell) {
	return (*GetBhkWorld)(parentCell);
}