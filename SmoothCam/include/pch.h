#pragma once
#include <codeanalysis\warnings.h>
#include "code_analysis.h"
#include <new>
void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags,
    unsigned debugFlags, const char* file, int line);

extern "C" {
    int __cdecl Vsnprintf8(char* p, size_t n, const char* pFormat, va_list arguments);
}

// Ignore all warnings from external code
SILENCE_CODE_ANALYSIS;
SILENCE_EXTERNAL;
#pragma warning(push)
#   pragma warning(disable : 4201 28020 26439 6011 6294)
#   include "RE/Skyrim.h"
#   include "SKSE/SKSE.h"
#   include <xbyak/xbyak.h>
#pragma warning(pop)

#   include <shlobj.h>	
#   include <stdlib.h>
#   include <cstdlib>
#   include <stdexcept>
#   include <fstream>
#   include <filesystem>
#   include <thread>
#   include <xmmintrin.h>
#   include <stdint.h>
#   include <utility>
#   undef CreateFont
#   undef NO_DATA

#   pragma warning(push)
#   ifdef NDEBUG
#	    include <spdlog/sinks/basic_file_sink.h>
#   else
#	    include <spdlog/sinks/msvc_sink.h>
#   endif
#   pragma warning(pop)

#   define EASTL_EASTDC_VSNPRINTF 0
#   include <EASTL/shared_ptr.h>
#   include <EASTL/unique_ptr.h>
#   include <EASTL/string.h>
#   include <EASTL/numeric_limits.h>
#   include <EASTL/algorithm.h>
#   include <EASTL/functional.h>
#   include <EASTL/bitset.h>
#   include <EASTL/unordered_map.h>
#   include <EASTL/map.h>
#   include <EASTL/array.h>
#   include <EASTL/tuple.h>
#   include <EASTL/vector.h>
#   include <EASTL/fixed_vector.h>
#   include <EASTL/fixed_slist.h>
#   include <EASTL/fixed_list.h>
#   include <EASTL/bonus/fixed_ring_buffer.h>

#   ifdef BUCK_V2
#       include <detours/detours.h>
#   else
#       //include "include/detours.h"
#   endif
#   include <polyhook2/Virtuals/VFuncSwapHook.hpp>
#   include <polyhook2/ZydisDisassembler.hpp>
#   include <polyhook2/Detour/x64Detour.hpp>
#   include <mapbox/eternal.hpp>

#   define GLM_FORCE_INLINE
#   define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#   define GLM_FORCE_INTRINSICS
#   define GLM_FORCE_LEFT_HANDED
#   define GLM_FORCE_DEPTH_ZERO_TO_ONE
#   define GLM_FORCE_QUAT_DATA_WXYZ
#   define GLM_ENABLE_EXPERIMENTAL
#   include <glm/glm.hpp>
#   include <glm/gtc/matrix_transform.hpp>
#   include <glm/ext/matrix_clip_space.hpp>
#   include <glm/gtc/quaternion.hpp>
#   include <glm/gtc/constants.hpp>
#   include <glm/gtx/easing.hpp>
#   include <glm/gtx/spline.hpp>
#   include <glm/gtx/norm.hpp>
#   include <glm/gtx/hash.hpp>
#   include <glm/gtx/type_trait.hpp>

using vec2u = glm::vec<2, float, glm::highp>;
using vec3u = glm::vec<3, float, glm::highp>;
using vec4u = glm::vec<4, float, glm::highp>;

using vec2ui = glm::vec<2, int, glm::highp>;
using vec3ui = glm::vec<3, int, glm::highp>;
using vec4ui = glm::vec<4, int, glm::highp>;
RESTORE_EXTERNAL;
RESTORE_CODE_ANALYSIS;

// These warnings are mostly pedantic and due to the nature of the code, we choose to disable them.
#pragma warning( disable : 26812 ) // Unscoped enum types - dependencies must update
#pragma warning( disable : 26490 ) // reinterpret_cast - unavoidable in this code base
#pragma warning( disable : 26429 ) // gsl::not_null - until gsl is part of msvc, ignored
#pragma warning( disable : 26446 ) // gsl::at - same as above
#pragma warning( disable : 26482 ) // array index must be constexpr - no.
#pragma warning( disable : 26481 ) // pointer math - required.
#pragma warning( disable : 26489 ) // invalid pointer lifetime (disabled until annotations are supported)
#pragma warning( disable : 26426 ) // no global init calls - globals are initialized before use, turned off
#pragma warning( disable : 26486 ) // invalid pointer passing - also busted?
#pragma warning( disable : 4201 ) // i like doing this, fight me

// Enable in-progress features
//#define DEVELOPER

// Enable an exception handler that writes minidumps when smoothcam code crashes
#define EMIT_MINIDUMPS

// Enable Direct2D code
//#define WITH_D2D
#ifdef WITH_D2D
// Enable debug overlays
#   define WITH_CHARTS

#   ifdef WITH_CHARTS
#       include "profile.h"
#   endif
#endif

#include "skyrimSE/PlayerCameraTransitionState.h"
#include "skyrimSE/AutoVanityState.h"
#include "skyrimSE/HorseCameraState.h"
#include "skyrimSE/FreeCameraState.h"
#include "skyrimSE/DragonCameraState.h"
#include "skyrimSE/IMovementInterface.h"

#include "offset_ids.h"
#include "modapi.h"
#include "util.h"
#include "debug/console.h"
#include "timer.h"
#include "basicdetour.h"
#include "config.h"
#include "mmath.h"
#include "raycast.h"
#include "game_state.h"

#include "render/common.h"
#include "render/d3d_context.h"
#include "render/cbuffer.h"
#include "render/shader.h"
#include "render/vertex_buffer.h"

using namespace std::literals;
namespace logger = SKSE::log;

static inline void FatalError(const wchar_t* message) noexcept {
    ShowCursor(true);
    FatalAppExit(0, message);
}

static inline void WarningPopup(const wchar_t* message) noexcept {
    ShowCursor(true);
    MessageBox(nullptr, message, L"SmoothCamera", MB_ICONERROR);
}