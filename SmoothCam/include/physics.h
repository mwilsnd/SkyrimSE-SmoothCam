#pragma once

namespace Physics {
	hkp3AxisSweep* GetBroadphase(const bhkWorld* physicsWorld);
	bhkWorld* GetWorld(const TESObjectCELL* parentCell);
}