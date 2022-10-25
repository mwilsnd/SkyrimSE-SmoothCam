#include "raycast.h"

extern Offsets* g_Offsets;

Raycast::RayCollector::RayCollector() {}

void Raycast::RayCollector::AddRayHit(const RE::hkpCdBody& body, const RE::hkpShapeRayCastCollectorOutput& hitInfo) {
	HitResult hit;
	hit.hitFraction = hitInfo.hitFraction;
	hit.normal = {
		hitInfo.normal.quad.m128_f32[0],
		hitInfo.normal.quad.m128_f32[1],
		hitInfo.normal.quad.m128_f32[2] 
	};
	
	const RE::hkpCdBody* obj = &body;
	while (obj) {
		if (!obj->parent) break;
		obj = obj->parent;
	}

	hit.body = obj;
	if (!hit.body) return;

	const auto collisionObj = static_cast<const RE::hkpCollidable*>(hit.body);
	const auto flags = collisionObj->broadPhaseHandle.collisionFilterInfo;

	const uint64_t m = 1ULL << static_cast<uint64_t>(flags);
	constexpr uint64_t filter = 0x40122716; //@TODO
	if ((m & filter) != 0) {
		if (objectFilter.size() > 0) {
			for (const auto filteredObj : objectFilter) {
				if (hit.getAVObject() == filteredObj) return;
			}
		}

		earlyOutHitFraction = hit.hitFraction;
		hits.push_back(eastl::move(hit));
	}
}

const eastl::vector<Raycast::RayCollector::HitResult>& Raycast::RayCollector::GetHits() {
	return hits;
}

void Raycast::RayCollector::Reset() {
	earlyOutHitFraction = 1.0f;
	hits.clear();
	objectFilter.clear();
}

RE::NiAVObject* Raycast::RayCollector::HitResult::getAVObject() {
	typedef RE::NiAVObject* (*_GetUserData)(const RE::hkpCdBody*);
	static auto getAVObject = REL::Relocation<_GetUserData>(g_Offsets->GetNiAVObject);
	return body ? getAVObject(body) : nullptr;
}

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

Raycast::RayCollector* getCastCollector() noexcept {
	static Raycast::RayCollector collector = Raycast::RayCollector();
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

	constexpr auto one = 1.0f;
	const auto from = start * hkpScale;
	const auto to = dif * hkpScale;

	RE::hkpWorldRayCastInput pickRayInput{};
	pickRayInput.from = RE::hkVector4(from.x, from.y, from.z, one);
	pickRayInput.to = RE::hkVector4(0.0, 0.0, 0.0, 0.0);

	auto collector = getCastCollector();
	collector->Reset();

	RE::bhkPickData pickData{};
	pickData.rayInput = pickRayInput;
	pickData.ray = RE::hkVector4(to.x, to.y, to.z, one);
	pickData.rayHitCollectorA8 = reinterpret_cast<RE::hkpClosestRayHitCollector*>(collector);

	const auto ply = RE::PlayerCharacter::GetSingleton();
	if (!ply->parentCell) return {};

	if (ply->loadedData && ply->loadedData->data3D)
		collector->AddFilter(ply->loadedData->data3D.get());

	auto physicsWorld = ply->parentCell->GetbhkWorld();
	if (physicsWorld) {
		physicsWorld->PickObject(pickData);
	}
	
	Raycast::RayCollector::HitResult best{};
	best.hitFraction = 1.0f;
	glm::vec4 bestPos = {};

	for (auto& hit : collector->GetHits()) {
		const auto pos = (dif * hit.hitFraction) + start;
		if (best.body == nullptr) {
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

	if (!best.body) return result;
	auto av = best.getAVObject();
	result.hit = av != nullptr;

	if (result.hit) {
		auto ref = av->userData;
		if (ref && ref->formType == RE::FormType::ActorCharacter) {
			result.hitCharacter = reinterpret_cast<RE::Character*>(ref);
		}
	}

	return result; 
}