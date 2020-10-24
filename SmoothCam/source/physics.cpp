namespace {
	typedef bhkWorld*(__fastcall* bhkWorldGetter)(const TESObjectCELL* cell);
	constexpr auto hkpBroadphaseOffset = 0x88;
}

hkp3AxisSweep* Physics::GetBroadphase(const bhkWorld* physicsWorld) {
	auto ptr = reinterpret_cast<hkp3AxisSweep**>(physicsWorld->unk40() + hkpBroadphaseOffset);
	return *ptr;
}

bhkWorld* Physics::GetWorld(const TESObjectCELL* parentCell) {
	static auto getWorld = Offsets::Get<bhkWorldGetter>(18536);
	return getWorld(parentCell); // 0x2654c0
}