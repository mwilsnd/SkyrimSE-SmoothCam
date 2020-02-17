#include "pch.h"

namespace {
	typedef bool(__fastcall* RayCastFunType)(
		UnkPhysicsHolder* physics, bhkWorld* world, glm::vec4& rayStart,
		glm::vec4& rayEnd, uint32_t* rayResultInfo, Character** hitCharacter, float traceHullSize
	);

	RelocAddr<RayCastFunType> playerCameraRayCastingFunc(0x4f45f0); // 1404f45f0
}

Raycast::RayResult Raycast::CastRay(glm::vec4 start, glm::vec4 end, float traceHullSize) {
	RayResult res;

#ifdef _DEBUG
	// A saftey net for my own bad code - if the engine EVER gets a nan or inf float
	// shit WILL hit the fan in the audio & rendering systems (but mostly the audio system)
	if (!mmath::IsValid(start) || !mmath::IsValid(end) || !mmath::IsValid(traceHullSize)) {
		__debugbreak();
		return res;
	}
#endif

	auto playerCamera = reinterpret_cast<CorrectedPlayerCamera*>(PlayerCamera::GetSingleton());
	auto ply = (*g_thePlayer);
	if (!ply || !ply->parentCell || !playerCamera || !playerCamera->physics) return res;

	// Touching the player refcount likely isn't REQUIRED - i'm just following the disassembly here
	ply->handleRefObject.IncRef();
	{
		auto physicsWorld = Physics::GetWorld(ply->parentCell);
		if (physicsWorld) {
			res.hit = (*playerCameraRayCastingFunc)(
				playerCamera->physics, physicsWorld,
				start, end, static_cast<uint32_t*>(res.data), &res.hitCharacter,
				traceHullSize
			);
		}
	}
	ply->handleRefObject.DecRef();

	if (res.hit) {
		res.hitPos = end;
		res.rayLength = glm::length(static_cast<glm::vec3>(res.hitPos) - static_cast<glm::vec3>(start));
	}

	return res;
}