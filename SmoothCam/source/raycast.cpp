#include "raycast.h"

namespace {
	typedef bool(__fastcall* RayCastFunType)(
		UnkPhysicsHolder* physics, bhkWorld* world, glm::vec4& rayStart,
		glm::vec4& rayEnd, uint32_t* rayResultInfo, Character** hitCharacter, float traceHullSize
	);
}

Raycast::RayResult Raycast::CastRay(glm::vec4 start, glm::vec4 end, float traceHullSize, bool intersectCharacters) {
	RayResult res;
#ifdef _DEBUG
	// A saftey net for my own bad code - if the engine EVER gets a nan or inf float
	// shit WILL hit the fan in the audio & rendering systems (but mostly the audio system)
	if (!mmath::IsValid(start) || !mmath::IsValid(end) || !mmath::IsValid(traceHullSize)) {
		__debugbreak();
		return res;
	}
#endif

	auto playerCamera = CorrectedPlayerCamera::GetSingleton();
	auto ply = (*g_thePlayer);
	if (!ply || !ply->parentCell || !playerCamera || !playerCamera->physics) return res;

	ply->handleRefObject.IncRef();
	{
		auto physicsWorld = Physics::GetWorld(ply->parentCell);
		if (physicsWorld) {
			static auto cameraCaster = Offsets::Get<RayCastFunType>(32270);
			res.hit = cameraCaster( // 0x4f45f0
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

hkpCastCollector* getCastCollector() {
	static hkpCastCollector collector = hkpCastCollector();
	return &collector;
}

Raycast::RayResult Raycast::hkpCastRay(glm::vec4 start, glm::vec4 end) {
#ifdef _DEBUG
	if (!mmath::IsValid(start) || !mmath::IsValid(end)) {
		__debugbreak();
		return {};
	}
#endif

	constexpr auto hkpScale = 0.0142875f;
	const auto dif = end - start;

	hkpRayCastInfo info;
	info.start = start * hkpScale;
	info.end = dif * hkpScale;
	info.collector = getCastCollector();
	info.collector->reset();

	auto ply = *g_thePlayer;
	ply->handleRefObject.IncRef();
	{
		auto physicsWorld = Physics::GetWorld(ply->parentCell);
		if (physicsWorld) {
			physicsWorld->CastRay(&info);
		}
	}
	ply->handleRefObject.DecRef();

	hkpRayHitResult best = {};
	best.hitFraction = 1.0f;
	glm::vec4 bestPos = {};

	for (auto& hit : info.collector->results) {
		const auto pos = (dif * hit.hitFraction) + start;
		if (best.hit == nullptr) {
			best = hit;
			bestPos = pos;
			continue;
		}

		if (hit.hitFraction < best.hitFraction) {
			best = hit;
			bestPos = pos;
		}
	}

	RayResult result;
	result.hitPos = bestPos;
	result.rayLength = glm::length(bestPos - start);

	// FUN_1404f45f0 <32270>
	//		140dad060:GetAVObjectFromHavok <76160>
	//		1402945e0:ExtractCharacterFromTraceRes <19323>

	/*
	MOV        EDI,dword ptr [TheCamera->unk120 + 0x2c]
	AND        EDI,0x7f
	CALL       140dad060:GetAVObjectFromHavok <76160>

	lVar1 = (**(code **)(*(longlong *)param_1 + 0x28))(param_1);
		mov rax, rcx
		ret
	*/

	if (!best.hit) return result;
	typedef NiAVObject*(__fastcall* _GetUserData)(bhkShapeList*);
	static auto getAVObject = Offsets::Get<_GetUserData>(76160);
	auto av = getAVObject(best.hit);
	result.hit = av != nullptr;

	// What a useless function, only returning a valid character if it hits the actor origin?
	/*
	typedef Character*(__fastcall* ExtractCharacterFromTraceRes)(NiAVObject*);
	if (result.hit) {
		auto character = Offsets::Get<ExtractCharacterFromTraceRes>(19323)(av);
		if (character && *reinterpret_cast<char*>(reinterpret_cast<intptr_t>(character) + 0x1a) == '>') {
			result.hitCharacter = character;
		}
	}
	*/

	return result; 
}