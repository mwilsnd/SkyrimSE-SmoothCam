#pragma once
#include "offset_ids.h"

namespace SkyrimSE {
	typedef struct bhkShapeList {
		RE::hkpShape* shape;
		uint64_t unk0;
		void* shapeInfo;
		bhkShapeList* next;
		vec3u unk1;
		uint32_t flags;
		uint32_t unk2;
		uint32_t unk3;
		uint32_t unk4;
	} bhkShapeList;

	struct bhkRayHitResult {
		glm::vec3 normal;
		float hitFraction;
		bhkShapeList* hit;
	};

	struct bhkAllCdPointTempResult {
		glm::vec4 normal;
		float hitFraction;
	};

	class bhkLinearCastCollector {
		public:
			bhkLinearCastCollector() noexcept {
				results.reserve(64);
			}

			inline void reset() noexcept {
				results.clear();
				objectFilter.clear();
				m_hitFraction = 0.0f;
				m_earlyOutHitFraction = 1.0f;
			}

			inline void addFilter(const RE::NiAVObject* obj) noexcept {
				objectFilter.push_back(obj);
			}

			virtual void addRayHit(bhkShapeList* list, const bhkAllCdPointTempResult* hitInfo) {
				extern Offsets* g_Offsets;

				bhkRayHitResult hitResult;
				hitResult.hitFraction = hitInfo->hitFraction;
				hitResult.normal = static_cast<glm::vec3>(hitInfo->normal);

				while (list) {
					if (!list->next) break;
					list = list->next;
				}

				hitResult.hit = list;
				if (hitResult.hit) {
					const uint64_t m = 1ULL << static_cast<uint64_t>(hitResult.hit->flags & 0x7F);
					constexpr uint64_t filter = 0x40122716; //@TODO
					if ((m & filter) != 0) {
						if (objectFilter.size() > 0)
							for (const auto obj : objectFilter) {
								typedef RE::NiAVObject*(*GetUserData)(SkyrimSE::bhkShapeList*);
								if (REL::Relocation<GetUserData>(g_Offsets->GetNiAVObject)(hitResult.hit) == obj)
									return;
							}
						
						// We only want further hits to be closer than this
						m_earlyOutHitFraction = hitResult.hitFraction;
						results.push_back(eastl::move(hitResult));
					}
				}
			}

		public:
			enum { MAX_HIERARCHY_DEPTH = 8 };

			float m_earlyOutHitFraction = 1.0f;				//0x08
			uint32_t pad0;									//0x0C
			uint64_t pad1[2];								//0x10, 0x18
			float m_hitFraction = 0.0f;						//0x20
			uint32_t unk0 = 0;								//0x24
			RE::hkpShapeKey shapeKey;						//0x28
			uint32_t pad2;									//0x2C
			RE::hkpShapeKey m_shapeKeys[MAX_HIERARCHY_DEPTH] = {};
			uint32_t m_shapeKeyIndex = 0;
			uint32_t pad3;
			uint64_t unk1 = 0;
			RE::hkpCollidable* m_rootCollidable = nullptr;
			uint64_t unk2 = 0;

			eastl::vector<bhkRayHitResult> results;
			eastl::vector<const RE::NiAVObject*> objectFilter;
	};

	typedef __declspec(align(16)) struct hkpRayCastInfo {
		glm::vec4 start = {};							// 0x0
		glm::vec4 unkVec = {};							// 0x10
		bool unk0 = false;								// 0x20
		uint32_t flags = 0;								// 0x24
		uint64_t unk1_0 = 0;
		uint64_t unk1_1 = 0;
		uint64_t unk1_2 = 0;
		float unk2 = 1.0f;								// 0x40
		vec3ui unk3 = { -1, -1, -1 };					// 0x44
		uint64_t unk4_0 = 0;
		uint64_t unk4_1 = 0;
		uint64_t unk4_2 = 0;
		uint64_t unk4_3 = 0;
		uint32_t unk5 = 0;								// 0x70
		uintptr_t unk6 = 0;
		uint64_t unk7 = 0;								// 0x80
		uintptr_t unk8 = 0;
		glm::vec4 end = {};								// 0x90
		uint64_t unk9 = 0;								// 0xA0
		bhkLinearCastCollector* collector = nullptr;	// 0xA8
		uint64_t unk10 = 0;
		uint64_t unk11 = 0;
		bool unk12 = false;								// 0xC0
		uint64_t unk13[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	} bhkRayCastInfo;
	static_assert(offsetof(bhkRayCastInfo, start.w) == 0xC);
	static_assert(offsetof(bhkRayCastInfo, unk0) == 0x20);
	static_assert(offsetof(bhkRayCastInfo, flags) == 0x24);
	static_assert(offsetof(bhkRayCastInfo, unk2) == 0x40);
	static_assert(offsetof(bhkRayCastInfo, unk5) == 0x70);
	static_assert(offsetof(bhkRayCastInfo, unk7) == 0x80);
	static_assert(offsetof(bhkRayCastInfo, end) == 0x90);
	static_assert(offsetof(bhkRayCastInfo, end.w) == 0x9C);
	static_assert(offsetof(bhkRayCastInfo, unk9) == 0xA0);
	static_assert(offsetof(bhkRayCastInfo, collector) == 0xA8);
	static_assert(offsetof(bhkRayCastInfo, unk12) == 0xC0);
	static_assert(sizeof(bhkRayCastInfo) == 0x120);
}