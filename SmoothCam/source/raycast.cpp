#include <execution>

namespace {
	typedef bool(__fastcall* RayCastFunType)(
		UnkPhysicsHolder* physics, bhkWorld* world, glm::vec4& rayStart,
		glm::vec4& rayEnd, uint32_t* rayResultInfo, Character** hitCharacter, float traceHullSize
	);

	constexpr auto ThreadActorIntersectionAtCount = 64;
}

#ifdef _DEBUG
void DrawAABB(const mmath::AABB& aabb, bool hit) {
	const auto hitColor = glm::vec3{ 0.0f, 1.0f, 0.0f };
	const auto missColor = glm::vec3{ 1.0f, 0.0f, 0.0f };
	const auto color = hit ? hitColor : missColor;
	const auto blf = mmath::PointToScreen(aabb.mins);
	const auto brf = mmath::PointToScreen({ aabb.maxs.x, aabb.mins.y, aabb.mins.z });
	const auto blb = mmath::PointToScreen({ aabb.mins.x, aabb.maxs.y, aabb.mins.z });
	const auto brb = mmath::PointToScreen({ aabb.maxs.x, aabb.maxs.y, aabb.mins.z });
	const auto tlf = mmath::PointToScreen({ aabb.mins.x, aabb.mins.y, aabb.maxs.z });
	const auto trf = mmath::PointToScreen({ aabb.maxs.x, aabb.mins.y, aabb.maxs.z });
	const auto tlb = mmath::PointToScreen({ aabb.mins.x, aabb.maxs.y, aabb.maxs.z });
	const auto trb = mmath::PointToScreen(aabb.maxs);
	DebugDrawing::Submit(DebugDrawing::DrawBox(DebugDrawing::DrawBox::BoxPoints({
		blf, brf, blb, brb,
		tlf, trf, tlb, trb
	}), color));
}
#endif

Raycast::AIProcessManager* Raycast::AIProcessManager::GetSingleton() {
	static auto ofs = Offsets::Get<AIProcessManager**>(514167);
	return *ofs;
}

// Perform a ray-AABB intersection test with all actors in the given list, return the hits
uint8_t Raycast::IntersectRayAABBAllActorsIn(const tArray<UInt32>& list, ActorRayResults& results,
	const glm::vec3& start, const glm::vec3& dir, float distance, uint8_t currentCount)
{
	uint8_t hitCount = 0;
#ifdef _DEBUG
	Profiler prof;
#endif

	// Lazy effort to multithread the tests when dealing with a large amount of actors
	if (list.count >= ThreadActorIntersectionAtCount) {
		std::atomic_uint counter = currentCount;

		std::for_each(
			std::execution::par_unseq,
			list.entries,
			list.entries + list.count,
			[&counter, &results, &start, &dir, &distance](UInt32 refID) {
				if (counter >= Raycast::MaxActorIntersections) return;
				
				NiPointer<TESObjectREFR> ref;
				(*LookupREFRByHandle)(refID, ref);
				if (!ref) return;
				auto actor = DYNAMIC_CAST(ref, TESObjectREFR, Actor);
				if (!actor) return;

				const auto bits = std::bitset<32>(actor->flags1);
				constexpr const auto setOnDeath = 23;
				if (bits[setOnDeath]) return;

				actor->IncRef();
				{
					const auto pos = glm::vec3(actor->pos.x, actor->pos.y, actor->pos.z);
					if (glm::length2(static_cast<glm::vec3>(start) - pos) <= distance) {
						const auto aabb = mmath::GetActorAABB(actor);
						glm::vec3 hit;
						if (mmath::IntersectRayAABB(start, dir, aabb, hit)) {
							Raycast::RayResult res;
							res.hit = true;
							res.hitPos = glm::vec4(hit, 0.0f);
							res.rayLength = glm::length(hit - start);
							res.hitCharacter = reinterpret_cast<Character*>(actor);
							const auto old = counter.fetch_add(1, std::memory_order::memory_order_relaxed);
							if (old < ThreadActorIntersectionAtCount)
								results[old] = res;
#ifdef _DEBUG
							DrawAABB(aabb, true);
#endif
						}
#ifdef _DEBUG
						else
							DrawAABB(aabb, false);
#endif
					}
				}
				actor->DecRef();
			}
		);

		hitCount = counter;
	} else {
		uint32_t counter = currentCount;

		std::for_each(
			std::execution::seq,
			list.entries,
			list.entries + list.count,
			[&counter, &results, &start, &dir, &distance](UInt32 refID) {
				if (counter >= Raycast::MaxActorIntersections) return;

				NiPointer<TESObjectREFR> ref;
				(*LookupREFRByHandle)(refID, ref);
				if (!ref) return;
				auto actor = DYNAMIC_CAST(ref, TESObjectREFR, Actor);
				if (!actor) return;

				const auto bits = std::bitset<32>(actor->flags1);
				constexpr const auto setOnDeath = 23;
				if (bits[setOnDeath]) return;

				actor->IncRef();
				{
					const auto pos = glm::vec3(actor->pos.x, actor->pos.y, actor->pos.z);
					if (glm::length2(static_cast<glm::vec3>(start) - pos) <= distance) {
						const auto aabb = mmath::GetActorAABB(actor);
						glm::vec3 hit;
						if (mmath::IntersectRayAABB(start, dir, aabb, hit)) {
							Raycast::RayResult res;
							res.hit = true;
							res.hitPos = glm::vec4(hit, 0.0f);
							res.rayLength = glm::length(hit - start);
							res.hitCharacter = reinterpret_cast<Character*>(actor);
							results[counter] = res;
							counter++;
#ifdef _DEBUG
							DrawAABB(aabb, true);
#endif
						}
#ifdef _DEBUG
						else
							DrawAABB(aabb, false);
#endif
					}
				}
				actor->DecRef();
			}
		);

		hitCount = counter;
	}

#ifdef _DEBUG
	const auto snap = prof.Snap();
	int bp = 0;
#endif

	return hitCount;
}

Raycast::RayResult Raycast::InteresctRayAABBAllActors(const glm::vec3& start, const glm::vec3& end) {
	const auto aiMgr = AIProcessManager::GetSingleton();
	const auto rayDistance = glm::length2(start - static_cast<glm::vec3>(end));
	const auto dir = glm::normalize(static_cast<glm::vec3>(end) - start);
	std::array<Raycast::RayResult, Raycast::MaxActorIntersections> hitActors;

	// Intersect with all actors in the high process
	// @Note: With debug drawing on, I can see actors right next to the player that aren't in this list
	// until they start a new AI activity - for now this will have to do
	const auto hitCount = IntersectRayAABBAllActorsIn(aiMgr->actorsHigh, hitActors, start, dir, rayDistance);
	
	Raycast::RayResult closestHit;
	for (auto i = 0; i < hitCount; i++) {
		auto hit = hitActors[i];
		if (closestHit.hit) {
			if (closestHit.rayLength > hit.rayLength)
				closestHit = hit;
		} else {
			closestHit = hit;
		}
	}

	return closestHit;
}

Raycast::RayResult Raycast::CastRay(glm::vec4 start, glm::vec4 end, float traceHullSize, bool intersectCharacters) {
	RayResult res;
	RayResult actorRes;

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
		// Homebrew intersection with actors
		// This is pretty lame, need to find an engine method for proper ray-actor testing
		if (intersectCharacters)
			actorRes = InteresctRayAABBAllActors(start, end);

		auto physicsWorld = Physics::GetWorld(ply->parentCell);
		if (physicsWorld) {
			res.hit = Offsets::Get<RayCastFunType>(32270)( // 0x4f45f0
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

	// Check if we hit an actor and if the hit is closer to us
	if (intersectCharacters && actorRes.hit && res.hit) {
		if (actorRes.rayLength < res.rayLength)
			return actorRes; // Hit an actor
	}

	return res;
}