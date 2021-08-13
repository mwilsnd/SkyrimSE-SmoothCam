#pragma once
#include <shlobj.h>	
#include <stdlib.h>
#include <cstdlib>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <thread>
#include <xmmintrin.h>

#include <new>
void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags,
    unsigned debugFlags, const char* file, int line);

extern "C" {
    int __cdecl Vsnprintf8(char* p, size_t n, const char* pFormat, va_list arguments);
}

#include "code_analysis.h"

// Ignore all warnings from external code
SILENCE_CODE_ANALYSIS;
#   define EASTL_EASTDC_VSNPRINTF 0
#   include <EASTL/shared_ptr.h>
#   include <EASTL/unique_ptr.h>
#   include <EASTL/string.h>
#   include <EASTL/numeric_limits.h>
#   include <EASTL/algorithm.h>
#   include <EASTL/functional.h>
#   include <EASTL/bitset.h>
#   include <EASTL/unordered_map.h>
#   include <EASTL/array.h>
#   include <EASTL/tuple.h>
#   include <EASTL/vector.h>
#   include <EASTL/fixed_vector.h>
#   include <EASTL/fixed_slist.h>
#   include <EASTL/fixed_list.h>

#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <Windows.h>
#   include <winnt.h>
#   undef CreateFont
#   undef NO_DATA

// Compat when building in unicode mode with SKSE
// (ModInfo.fileData is of type WIN32_FIND_DATA which changes based on mbcs or unicode)
#   ifdef UNICODE
		// Force the use of WIN32_FIND_DATAA while including SKSE, silence macro warnings
#       pragma warning( disable : 4005 )
#       define WIN32_FIND_DATA WIN32_FIND_DATAA
#   endif
    
#   include <common/ITypes.h>
#   include <common/IDebugLog.h>
    
#   include <skse64/PluginAPI.h>
#   include <skse64/GameAPI.h>
#   include <skse64/GameRTTI.h>
#   include <skse64_common/skse_version.h>

#   include <skse64/NiTypes.h>
#   include <skse64/NiNodes.h>
#   include <skse64/NiObjects.h>
#   include <skse64/NiGeometry.h>
#   include <skse64/NiRenderer.h>
#   include <skse64/NiProperties.h>
#   include <skse64/GameReferences.h>
#   include <skse64/GameEvents.h>
#   include <skse64/GameForms.h>
#   include <skse64/GameThreads.h>
#   include <skse64/GameMenus.h>
#   include <skse64/GameInput.h>
#   include <skse64/GameCamera.h>
#   include <skse64/GameSettings.h>
#   include <skse64/GameData.h>
    
#   include <skse64/PapyrusArgs.h>
#   include <skse64/PapyrusEvents.h>
#   include <skse64/PapyrusNativeFunctions.h>
#   include <skse64/ScaleformCallbacks.h>
#   include <skse64/ScaleformMovie.h>
    
#   include <skse64/PapyrusVM.h>
#   include <skse64/PapyrusNativeFunctions.h>

#   ifdef UNICODE
		// Revert our compat hack
#       undef WIN32_FIND_DATA
#       define WIN32_FIND_DATA WIN32_FIND_DATAW
#   endif

// @TODO: Use Polyhook2's function detouring instead, drop microsoft detours
#   include <../Detours/include/detours.h>
#   include <polyhook2/Virtuals/VFuncSwapHook.hpp>
#   include <mapbox/eternal.hpp>

#   include "addrlib/versiondb.h"

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

using vec2u = glm::vec<2, float, glm::highp>;
using vec3u = glm::vec<3, float, glm::highp>;
using vec4u = glm::vec<4, float, glm::highp>;

using vec2ui = glm::vec<2, int, glm::highp>;
using vec3ui = glm::vec<3, int, glm::highp>;
using vec4ui = glm::vec<4, int, glm::highp>;
RESTORE_CODE_ANALYSIS;

// Trying to use code analysis on a project like this is like thinking a high-visibility vest
// will make you safer when jumping out of a burning building.

// These warnings are mostly pedantic and due to the nature of the code, we choose to disable them.
#pragma warning( disable : 26812 ) // Unscoped enum types - dependencies must update
#pragma warning( disable : 26490 ) // reinterpret_cast - unavoidable in this code base
#pragma warning( disable : 26429 ) // gsl::not_null - until gsl is part of msvc, ignored
#pragma warning( disable : 26446 ) // gsl::at - same as above
#pragma warning( disable : 26482 ) // array index must be constexpr - no.
#pragma warning( disable : 26481 ) // pointer math - required.
#pragma warning( disable : 26489 ) // invalid pointer lifetime (disabled until annotations are supported)
#pragma warning( disable : 26438 ) // avoid goto - no.
#pragma warning( disable : 26426 ) // no global init calls - globals are initialized before use, turned off
#pragma warning( disable : 26409 ) // avoid naked new and delete - required for papyrus registration
#pragma warning( disable : 26486 ) // invalid pointer passing - also busted?

#include "modapi.h"
#include "util.h"
#include "debug/console.h"
#include "addrlib/offsets.h"
#include "timer.h"

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

#include "basicdetour.h"
#include "havok/hkp3AxisSweep.h"

#include "skyrimSE/bhkRigidBody.h"
#include "skyrimSE/bhkSimpleShapePhantom.h"
#include "skyrimSE/bhkWorld.h"
#include "skyrimSE/PlayerCamera.h"
#include "skyrimSE/FirstPersonState.h"
#include "skyrimSE/ThirdPersonState.h"
#include "skyrimSE/PlayerCameraTransitionState.h"
#include "skyrimSE/HorseCameraState.h"
#include "skyrimSE/ArrowProjectile.h"

#include "arrow_fixes.h"
#include "mmath.h"
#include "physics.h"
#include "raycast.h"
#include "config.h"
#include "game_state.h"

#include "render/common.h"
#include "render/d3d_context.h"
#include "render/cbuffer.h"
#include "render/shader.h"
#include "render/vertex_buffer.h"

static inline void FatalError(const wchar_t* message) noexcept {
    ShowCursor(true);
    FatalAppExit(0, message);
}

static inline void WarningPopup(const wchar_t* message) noexcept {
    ShowCursor(true);
    MessageBox(nullptr, message, L"SmoothCamera", MB_ICONERROR);
}