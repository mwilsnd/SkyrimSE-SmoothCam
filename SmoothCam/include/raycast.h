#pragma once

namespace Raycast {
	struct hkpGenericShapeData {
		intptr_t* unk;
		uint32_t shapeType;
	};

	struct rayHitShapeInfo {
		hkpGenericShapeData* hitShape;
		uint32_t unk;
	};

	class RayCollector {
	public:
		struct HitResult {
			glm::vec3 normal;
			float hitFraction;
			const RE::hkpCdBody* body;

			RE::NiAVObject* getAVObject();
		};

	public:
		RayCollector();
		~RayCollector() = default;

		virtual void AddRayHit(const RE::hkpCdBody& body, const RE::hkpShapeRayCastCollectorOutput& hitInfo);

		inline void AddFilter(const RE::NiAVObject* obj) noexcept {
			objectFilter.push_back(obj);
		}

		const eastl::vector<HitResult>& GetHits();

		void Reset();

	private:
		float earlyOutHitFraction{ 1.0f }; // 08
		std::uint32_t pad0C{};
		RE::hkpWorldRayCastOutput rayHit; // 10

		eastl::vector<HitResult> hits{};
		eastl::vector<const RE::NiAVObject*> objectFilter;
	};

#pragma warning(push)
#pragma warning(disable : 26495)
	typedef __declspec(align(16)) struct bhkRayResult {
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
		bool hit = false;
		// If the ray hit a character actor, this will point to it
		RE::Character* hitCharacter = nullptr;
		// The length of the ray from start to hitPos
		float rayLength = 0.0f;
		// The position the ray hit, in world space
		glm::vec4 hitPos{};

		// pad to 128
		uint64_t _pad{};
	} RayResult;
	static_assert(sizeof(RayResult) == 128);
#pragma warning(pop)

	// Cast a ray from 'start' to 'end', returning the first thing it hits
	// This variant is used by the camera to test with the world for clipping
	//	Params:
	//		glm::vec4 start:     Starting position for the trace in world space
	//		glm::vec4 end:       End position for the trace in world space
	//		float traceHullSize: Size of the collision hull used for the trace
	//
	// Returns:
	//	RayResult:
	//		A structure holding the results of the ray cast.
	//		If the ray hit something, result.hit will be true.
	RayResult CastRay(glm::vec4 start, glm::vec4 end, float traceHullSize) noexcept;
	
	// Cast a ray from 'start' to 'end', returning the first thing it hits
	// This variant collides with pretty much any solid geometry
	//	Params:
	//		glm::vec4 start:     Starting position for the trace in world space
	//		glm::vec4 end:       End position for the trace in world space
	//
	// Returns:
	//	RayResult:
	//		A structure holding the results of the ray cast.
	//		If the ray hit something, result.hit will be true.
	RayResult hkpCastRay(const glm::vec4& start, const glm::vec4& end) noexcept;
}