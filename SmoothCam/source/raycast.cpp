#include "raycast.h"

extern Offsets* g_Offsets;

Raycast::RayResult Raycast::CastRay(glm::vec4 start, glm::vec4 end, float traceHullSize)
	noexcept
{
	RayResult res;
#ifdef _DEBUG
	// A saftey net for my own bad code - if the engine EVER gets a nan or inf float
	// shit WILL hit the fan in the audio & rendering systems (but mostly the audio system)
	if (!mmath::IsValid(start) || !mmath::IsValid(end) || !mmath::IsValid(traceHullSize)) {
		__debugbreak();
		return res;
	}
#endif

	const auto ply = RE::PlayerCharacter::GetSingleton();
	const auto cam = RE::PlayerCamera::GetSingleton();
	if (!ply->parentCell || !cam->unk120) return res;

	auto physicsWorld = ply->parentCell->GetbhkWorld();
	if (physicsWorld) {
		typedef bool(__fastcall* RayCastFunType)(
			decltype(RE::PlayerCamera::unk120) physics, RE::bhkWorld* world, glm::vec4& rayStart,
			glm::vec4& rayEnd, uint32_t* rayResultInfo, RE::Character** hitCharacter, float traceHullSize
		);
		static auto cameraCaster = REL::Relocation<RayCastFunType>(g_Offsets->CameraCaster);
		res.hit = cameraCaster(
			cam->unk120, physicsWorld,
			start, end, static_cast<uint32_t*>(res.data), &res.hitCharacter,
			traceHullSize
		);
	}

	if (res.hit) {
		res.hitPos = end;
		res.rayLength = glm::length(static_cast<glm::vec3>(res.hitPos) - static_cast<glm::vec3>(start));
	}

	return res;
}

SkyrimSE::bhkLinearCastCollector* getCastCollector() noexcept {
	static SkyrimSE::bhkLinearCastCollector collector = SkyrimSE::bhkLinearCastCollector();
	return &collector;
}

Raycast::RayResult Raycast::hkpCastRay(const glm::vec4& start, const glm::vec4& end) noexcept {
#ifdef _DEBUG
	if (!mmath::IsValid(start) || !mmath::IsValid(end)) {
		__debugbreak();
		return {};
	}
#endif

	constexpr auto hkpScale = 0.0142875f;
	const auto dif = end - start;

	SkyrimSE::bhkRayCastInfo info;
	info.start = start * hkpScale;
	info.end = dif * hkpScale;
	info.collector = getCastCollector();
	info.collector->reset();

	const auto ply = RE::PlayerCharacter::GetSingleton();
	if (!ply->parentCell) return {};

	if (ply->loadedData && ply->loadedData->data3D)
		info.collector->addFilter(ply->loadedData->data3D.get());

	auto physicsWorld = ply->parentCell->GetbhkWorld();
	if (physicsWorld) {
		typedef void(__thiscall RE::bhkWorld::*CastRay)(SkyrimSE::hkpRayCastInfo*) const;
		(physicsWorld->*reinterpret_cast<CastRay>(&RE::bhkWorld::Unk_33))(&info);
	}
	
	SkyrimSE::bhkRayHitResult best = {};
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

	if (!best.hit) return result;
	typedef RE::NiAVObject*(*_GetUserData)(SkyrimSE::bhkShapeList*);
	auto getAVObject = REL::Relocation<_GetUserData>(g_Offsets->GetNiAVObject);
	auto av = getAVObject(best.hit);
	result.hit = av != nullptr;

	if (result.hit) {
		auto ref = av->userData;
		if (ref && ref->formType == RE::FormType::ActorCharacter) {
			result.hitCharacter = reinterpret_cast<RE::Character*>(ref);
		}
	}

	return result; 
}