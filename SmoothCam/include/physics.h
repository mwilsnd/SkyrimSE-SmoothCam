#pragma once
#include "pch.h"

namespace Physics {
	hkp3AxisSweep* GetBroadphase(const bhkWorld* physicsWorld);
	bhkWorld* GetWorld(const TESObjectCELL* parentCell);
}