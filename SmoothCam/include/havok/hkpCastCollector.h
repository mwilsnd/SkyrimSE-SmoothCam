#pragma once
using hkpShape = void;
using hkpCdBody = void;
using hkpCollidable = void;
using hkpShapeKey = uint32_t;

typedef struct bhkShapeList {
	hkpShape* shape;
	uint64_t unk0;
	void* shapeInfo;
	bhkShapeList* next;
	glm::vec3 unk1;
	uint32_t flags;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;
} bhkShapeList;

struct hkpRayHitResult {
	glm::vec3 normal;
	float hitFraction;
	bhkShapeList* hit;
};

struct hkpAllCdPointTempResult {
	glm::vec4 normal;
	float hitFraction;
};

class hkpCastCollector {
	public:
		hkpCastCollector() {
			results.reserve(64);
		}

		inline void reset() {
			results.clear();
			m_hitFraction = 0.0f;
			m_earlyOutHitFraction = 1.0f;
		}

		virtual void addRayHit(bhkShapeList* list, const hkpAllCdPointTempResult* hitInfo) {
			hkpRayHitResult hitResult;
			hitResult.hitFraction = hitInfo->hitFraction;
			hitResult.normal = static_cast<glm::vec3>(hitInfo->normal);

			while (list) {
				if (!list->next) break;
				list = list->next;
			}

			hitResult.hit = list;
			if (hitResult.hit) {
				const uint64_t m = 1ULL << static_cast<uint64_t>(hitResult.hit->flags & 0x7F);
				constexpr uint64_t filter = 0x40122716;
				if ((m & filter) != 0) {
					results.push_back(hitResult);
					// We only want further hits to be closer than this
					m_earlyOutHitFraction = hitResult.hitFraction;
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
		hkpShapeKey shapeKey;							//0x28
		uint32_t pad2;									//0x2C
		hkpShapeKey m_shapeKeys[MAX_HIERARCHY_DEPTH];
		uint32_t m_shapeKeyIndex = 0;
		uint32_t pad3;
		uint64_t unk1 = 0;
		hkpCollidable* m_rootCollidable = nullptr;
		uint64_t unk2 = 0;

		std::vector<hkpRayHitResult> results;
};

typedef __declspec(align(16)) struct hkpRayCastInfo {
	glm::vec4 start;					// 0x0
	glm::vec4 unkVec;					// 0x10
	bool unk0 = false;					// 0x20

	// auVar11 = subps(*(undefined *)(param_2 + 0x24),(undefined  [16])0x0);
	// auVar10 = ZEXT812(SUB168(auVar11,0) & 0x7ff ...
	// uVar3 = movmskps(uVar3,CONCAT412(0xffffffff, ...
	// if (((byte)uVar3 & 7) != 7)
	//		write unkVec.x SUB124(*hkpRayCastInfo >> 0x20, 0) + (float)hkpRayCastInfo[0x24]
	//		...
	//		write unkVec.w  SUB164(*hkpRayCastInfo >> 0x60, 0) + (float)hkpRayCastInfo[0x27]
	// unk0 = 0
	uint32_t flags = 0;					// 0x24

	uint64_t unk1_0 = 0;
	uint64_t unk1_1 = 0;
	uint64_t unk1_2 = 0;
	float unk2 = 1.0f;					// 0x40
	glm::ivec3 unk3 = { -1, -1, -1 };	// 0x44
	uint64_t unk4_0 = 0;
	uint64_t unk4_1 = 0;
	uint64_t unk4_2 = 0;
	uint64_t unk4_3 = 0;
	uint32_t unk5 = 0;					// 0x70
	uintptr_t unk6 = 0;
	uint64_t unk7 = 0;					// 0x80
	uintptr_t unk8 = 0;

	glm::vec4 end;						// 0x90

	// collector = *(longlong *)(param_2 + 0x2c);
	// lVar2 = *(longlong *)(param_2 + 0x2e);
	// if ((collector == 0) && (lVar2 == 0))
	//		collector = *(longlong *)(param_2 + 0x2a);
	//		if (collector == 0)
	//			FUN_140a7b000_UnkTraceRelated(lVar5, hkpRayCastInfo, hkpRayCastInfo + 0xc);
	//			|
	//			v
	//			local_58[0] = &vtable.hkpSimpleWorldRayCaster;
	//			broadphase = broadphase = *(longlong **)(lVar5 + 0x88);
	//			filter = *(longlong *)(this + 0xd0);
	//			hkpSimpleWorldCastRayBroadphaseCache(
	//				local_58,
	//				broadphase,
	//				hkpRayCastInfo,
	//				filter,
	//				0, // cache
	//				hkpRayCastInfo + 0xc
	//			);

	//			hkpSimpleWorldCastRayBroadphaseCache:
	//			(**(code **)(*broadphase + 200))(broadphase, hkpRayCastInfo, this, 0);

	// local_88[0] = &vtable.hkpWorldRayCaster;
	// broadphase = *(longlong **)(lVar5 + 0x88);
	// filter = *(longlong *)(lVar5 + 0xd0);
	// cache = *(longlong *)(param_2 + 0x28);
	// 140b28060:castRay(local_88, broadphase, hkpRayCastInfo, filter, cache, collector_00);
	// broadphase<vtable.hkp3AxisSweep>->FUN_140b306f0
	//	[ (**(code **)(*broadphase + 200))(broadphase, hkpRayCastInfo, this, 0) ] vtable.hkpWorldRayCaster->FUN_140b284f0
	//   if ((*(byte *)(*(longlong *)(broadphase + 0xb0) + 0x10 + uVar9 * 0x18) & 1) == 0) {
	//		(**(code **)(*this + 8))(
	//			SUB168(auVar28,0), this,
	//			*(undefined8 *)(*(longlong *)(broadphase + 0xb0) + 0x10 + uVar9 * 0x18),
	//			(ulonglong)uVar34); [ hkpShape->FUN_140a59340:invokeCollector ]
	// FUN_140a59340:invokeCollector(auVar28, this, broadphaseOffset, uVar34)
	//		(**(code **)*param_4)(param_4,param_3,&local_58); [ virtual addRayHit ]

	uint64_t unk9 = 0;					// 0xA0
	hkpCastCollector* collector;		// 0xA8
	uint64_t unk10 = 0;
	uint64_t unk11 = 0;
	bool unk12 = false;					// 0xC0

	// lVar22 = 3;
	// plVar16 = 0xC0
	// plVar16 = plVar16 + 2;
	// lVar22 += -1;
	// while (lVar22 != 0)
	uint64_t unk13[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
} hkpRayCastInfo;
static_assert(offsetof(hkpRayCastInfo, start.w) == 0xC);
static_assert(offsetof(hkpRayCastInfo, unk0) == 0x20);
static_assert(offsetof(hkpRayCastInfo, flags) == 0x24);
static_assert(offsetof(hkpRayCastInfo, unk2) == 0x40);
static_assert(offsetof(hkpRayCastInfo, unk5) == 0x70);
static_assert(offsetof(hkpRayCastInfo, unk7) == 0x80);
static_assert(offsetof(hkpRayCastInfo, end) == 0x90);
static_assert(offsetof(hkpRayCastInfo, end.w) == 0x9C);
static_assert(offsetof(hkpRayCastInfo, unk9) == 0xA0);
static_assert(offsetof(hkpRayCastInfo, collector) == 0xA8);
static_assert(offsetof(hkpRayCastInfo, unk12) == 0xC0);
static_assert(sizeof(hkpRayCastInfo) == 0x120);