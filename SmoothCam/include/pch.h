#pragma once
#include <string>
#include <shlobj.h>	
#include <stdlib.h>
#include <stdexcept>
#include <algorithm>
#include <functional> 
#include <regex>
#include <bitset>
#include <unordered_map>
#include <fstream>
#include <array>

#include <codeanalysis\warnings.h>
#pragma warning( push )
// Ignore all warnings from external code
#   pragma warning( disable : ALL_CODE_ANALYSIS_WARNINGS )
#   pragma warning( disable : ALL_CPPCORECHECK_WARNINGS )
#   pragma warning( disable : 26812 4244 4267 )
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <Windows.h>

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
#   include <skse64_common/skse_version.h>
    
#   include <skse64/NiTypes.h>
#   include <skse64/NiNodes.h>
#   include <skse64/NiObjects.h>
#   include <skse64/GameReferences.h>
#   include <skse64/GameEvents.h>
#   include <skse64/GameForms.h>
#   include <skse64/GameThreads.h>
#   include <skse64/GameMenus.h>
#   include <skse64/GameInput.h>
#   include <skse64/GameCamera.h>
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
#   endif
//#   include <skse64_common/RelocationEx.h>
    
// @TODO: Use Polyhook2's function detouring instead, drop microsoft detours
#   include <../Detours/include/detours.h>
#   include <polyhook2/Virtuals/VFuncSwapHook.hpp>
#   include <mapbox/eternal.hpp>
#   include <cereal/cereal.hpp>
#   include <cereal/archives/json.hpp>

#   define GLM_FORCE_LEFT_HANDED
#   define GLM_FORCE_DEPTH_ZERO_TO_ONE
#   define GLM_FORCE_QUAT_DATA_WXYZ
#   define GLM_ENABLE_EXPERIMENTAL
#   include <glm/glm.hpp>
#   include <glm/gtc/matrix_transform.hpp>
#   include <glm/gtc/quaternion.hpp>
#   include <glm/gtc/constants.hpp>
#   include <glm/gtx/easing.hpp>
#   include <glm/gtx/spline.hpp>
#pragma warning( pop )

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

#include "basicdetour.h"
#include "havok/hkp3AxisSweep.h"

#include "skyrimSE/bhkRigidBody.h"
#include "skyrimSE/bhkSimpleShapePhantom.h"
#include "skyrimSE/bhkWorld.h"
#include "skyrimSE/PlayerCamera.h"
#include "skyrimSE/ThirdPersonState.h"

#include "mmath.h"
#include "physics.h"
#include "raycast.h"
#include "config.h"

static inline void FatalError(const wchar_t* message) noexcept {
	MessageBox(nullptr, message, L"SmoothCamera", MB_ICONERROR);
	exit(-1);
}