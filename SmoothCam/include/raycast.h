#pragma once

namespace Raycast {
	constexpr auto MaxActorIntersections = 16;

	// Thanks to CBPC SSE for revealing this handy object
	// While not perfect, it works well enough for now
	class AIProcessManager {
		public:
		static AIProcessManager* GetSingleton();

		UInt8  unk000;                   // 008
		bool   enableDetection;          // 001 
		bool   unk002;                   // 002 
		UInt8  unk003;                   // 003
		UInt32 unk004;                   // 004
		bool   enableHighProcess;        // 008 
		bool   enableLowProcess;         // 009 
		bool   enableMiddleHighProcess;  // 00A 
		bool   enableMiddleLowProcess;   // 00B 
		bool   enableAISchedules;        // 00C 
		UInt8  unk00D;                   // 00D
		UInt8  unk00E;                   // 00E
		UInt8  unk00F;                   // 00F
		SInt32 numActorsInHighProcess;   // 010
		UInt32 unk014[(0x30 - 0x014) / sizeof(UInt32)];
		tArray<UInt32>  actorsHigh; // 030 
		tArray<UInt32>  actorsLow;  // 048 
		tArray<UInt32>  actorsMiddleLow; // 060
		tArray<UInt32>  actorsMiddleHigh; // 078
		UInt32  unk90[(0xF0 - 0x7C) / sizeof(UInt32)];
		tArray<void*> activeEffectShaders; // 108
		//mutable BSUniqueLock			 activeEffectShadersLock; // 120
	};

	static_assert(offsetof(AIProcessManager, numActorsInHighProcess) >= 0x10, "Unk141F831B0::actorsHigh is too early!");
	static_assert(offsetof(AIProcessManager, numActorsInHighProcess) <= 0x10, "Unk141F831B0::actorsHigh is too late!");

	static_assert(offsetof(AIProcessManager, actorsHigh) >= 0x030, "Unk141F831B0::actorsHigh is too early!");
	static_assert(offsetof(AIProcessManager, actorsHigh) <= 0x039, "Unk141F831B0::actorsHigh is too late!");

	static_assert(offsetof(AIProcessManager, actorsLow) >= 0x048, "Unk141F831B0::actorsLow is too early!");
	static_assert(offsetof(AIProcessManager, actorsLow) <= 0x048, "Unk141F831B0::actorsLow is too late!");

	static_assert(offsetof(AIProcessManager, actorsMiddleLow) >= 0x060, "Unk141F831B0::actorsMiddleLow is too early!");
	static_assert(offsetof(AIProcessManager, actorsMiddleLow) <= 0x060, "Unk141F831B0::actorsMiddleLow is too late!");

	static_assert(offsetof(AIProcessManager, actorsMiddleHigh) >= 0x078, "Unk141F831B0::actorsMiddleHigh is too early!");
	static_assert(offsetof(AIProcessManager, actorsMiddleHigh) <= 0x078, "Unk141F831B0::actorsMiddleHigh is too late!");

	static_assert(offsetof(AIProcessManager, activeEffectShaders) >= 0x108, "Unk141F831B0::activeEffectShaders is too early!");
	static_assert(offsetof(AIProcessManager, activeEffectShaders) <= 0x108, "Unk141F831B0::activeEffectShaders is too late!");

	// These require more inspection - They all contain vtables of assorted physics shapes
	// It would be nice to figure these out to the point that a TESObjectREFR could be
	// extracted from them
	struct hkpGenericShapeData {
		intptr_t* unk;
		uint32_t shapeType;
	};

	struct rayHitShapeInfo {
		hkpGenericShapeData* hitShape;
		uint32_t unk;
	};

	typedef __declspec(align(16)) struct bhkRayResult {
		// The actual param given to the engine
		union {
			uint32_t data[16];
			struct {
				// Might be surface normal
				glm::vec4 rayUnkPos;
				// The normal vector of the ray
				glm::vec4 rayNormal;

				rayHitShapeInfo colA;
				rayHitShapeInfo colB;
			};
		};

		// Custom utility data, not part of the game
		
		// True if the trace hit something before reaching it's end position
		bool hit;
		// If the ray hit a character actor, this will point to it
		Character* hitCharacter;
		// The length of the ray from start to hitPos
		float rayLength;
		// The position the ray hit, in world space
		glm::vec4 hitPos;

		// pad to 128
		uint64_t _pad[3];

		bhkRayResult() noexcept : hit(false), hitCharacter(nullptr) {}
	} RayResult;
	static_assert(sizeof(RayResult) == 128);

	using ActorRayResults = std::array<Raycast::RayResult, MaxActorIntersections>;
	uint8_t IntersectRayAABBAllActorsIn(const tArray<UInt32>& list, ActorRayResults& results, const glm::vec3& start,
		const glm::vec3& dir, float distance, uint8_t currentCount = 0);
	RayResult InteresctRayAABBAllActors(const glm::vec3& start, const glm::vec3& end);

	// Cast a ray from 'start' to 'end', returning the first thing it hits
	//	Params:
	//		glm::vec4 start:     Starting position for the trace in world space
	//		glm::vec4 end:       End position for the trace in world space
	//		float traceHullSize: Size of the collision hull used for the trace
	//
	// Returns:
	//	RayResult:
	//		A structure holding the results of the ray cast.
	//		If the ray hit something, result.hit will be true.
	RayResult CastRay(glm::vec4 start, glm::vec4 end, float traceHullSize, bool intersectCharacters = false);
}