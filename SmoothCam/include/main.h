#pragma once
#include "camera.h"
#include "detours.h"
#include "papyrus.h"

#include "render/d3d_context.h"
#ifdef WITH_D2D
#   include "render/d2d.h"
#endif

constexpr eastl::array<uintptr_t, 36> idsToCache({
	// arrow fixes
	42537, 42536, 12204, 514905, 514725,
	49866, 42998, 43008, 42928,

	// camera
	69271,

	// crosshair
	80230, 80233, 505066, 515530, 505064, 505070, 505072,

	// firstperson
	39401, 100421, 100424, 100854,

	// main
	75446,

	// game state
	36457,

	// physics
	18536,

	// raycast
	32270, 76160,

	// thirdperson
	49816, 527997, 49858,

	// render/common
	513786,

	// render/d3d_context
	524728, 524730, 75713, 75694, 524768, 524998,
});