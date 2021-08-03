#include "physics.h"

namespace {
	typedef bhkWorld*(__fastcall* bhkWorldGetter)(const TESObjectCELL* cell);
	constexpr auto hkpBroadphaseOffset = 0x88;
}

hkp3AxisSweep* Physics::GetBroadphase(bhkWorld* physicsWorld) {
	auto ptr = reinterpret_cast<hkp3AxisSweep**>(physicsWorld->GetHavokWorld() + hkpBroadphaseOffset);
	return *ptr;
}

bhkWorld* Physics::GetWorld(const TESObjectCELL* parentCell) {
	static auto getWorld = Offsets::Get<bhkWorldGetter>(18536);
	return getWorld(parentCell); // 0x2654c0
}

glm::vec3 Physics::GetGravityVector(const TESObjectREFR* ref) {
	glm::vec3 gravity = { 0.0f, 0.0f, -9.8f };
	if (ref && ref->parentCell) {
		auto bhkWorld = Physics::GetWorld(ref->parentCell);
		if (bhkWorld) {
			auto hkpWorld = bhkWorld->GetHavokWorld();
			if (hkpWorld)
				gravity = static_cast<glm::vec3>(hkpWorld->gravity);
		}
	}
	return gravity;
}