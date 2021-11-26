cmake_minimum_required(VERSION 3.21.0)

option(BUILD_SKYRIM_AE "Build for Skyrim AE instead of SSE" OFF)

macro(set_from_environment VARIABLE)
	if (NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif ()
endmacro()

set_from_environment(VCPKG_ROOT)

if (DEFINED VCPKG_ROOT)
	set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
else ()
	message(
		WARNING
		"Variable VCPKG_ROOT is not set. Continuing without vcpkg."
	)
endif ()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(POLYHOOK_BUILD_STATIC_RUNTIME ON)
set(POLYHOOK_BUILD_DLL ON)
set(POLYHOOK_DISASM_CAPSTONE ON)
set(POLYHOOK_DISASM_ZYDIS ON)
set(POLYHOOK_FEATURE_EXCEPTION OFF)
set(POLYHOOK_FEATURE_DETOURS ON)
set(POLYHOOK_FEATURE_INLINENTD OFF)
set(POLYHOOK_FEATURE_PE ON)
set(POLYHOOK_FEATURE_VIRTUALS ON)
set(WITH_TESTS OFF)

project(
	SmoothCam
	LANGUAGES CXX
)

if (BUILD_SKYRIM_AE)
	add_subdirectory("Deps/CommonLibAE" CommonLibSSE)
else ()
	add_subdirectory("Deps/CommonLibSSE" CommonLibSSE)
endif (BUILD_SKYRIM_AE)

add_subdirectory("Deps/EASTL" EASTL)
add_subdirectory("Deps/PolyHook_2_0" PolyHook_2)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(binary_io CONFIG REQUIRED)

include(cmake/headers.cmake)
include(cmake/source.cmake)

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${HEADERS}
		${SOURCES}
)

add_library(
	${PROJECT_NAME}
	SHARED
	${HEADERS}
	${SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE "SmoothCam/include")
target_include_directories(${PROJECT_NAME} PRIVATE "Deps/glm")
target_include_directories(${PROJECT_NAME} PRIVATE "Deps/PolyHook_2_0")
target_include_directories(${PROJECT_NAME} PRIVATE "Deps/PolyHook_2_0/capstone/include")
target_include_directories(${PROJECT_NAME} PRIVATE "Deps/PolyHook_2_0/zydis/include")
target_include_directories(${PROJECT_NAME} PRIVATE "Deps/PolyHook_2_0/zydis/dependencies/zycore/include")
target_include_directories(${PROJECT_NAME} PRIVATE "Deps/eternal/include")
target_include_directories(${PROJECT_NAME} PRIVATE "Deps/json/include")
target_include_directories(${PROJECT_NAME} PRIVATE "Deps/detours")

if (MSVC)
	target_include_directories(${PROJECT_NAME} PRIVATE "Deps/PolyHook_2_0/zydis/msvc")
endif ()

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		CommonLibSSE::CommonLibSSE
		spdlog::spdlog
		EASTL
		PolyHook_2
		"${PROJECT_SOURCE_DIR}/Deps/Detours/lib.X64/detours.lib"
)

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		SmoothCam/include/pch.h
)

target_compile_features(
	${PROJECT_NAME}
	PRIVATE
		cxx_std_20
)

if (MSVC)
	add_compile_definitions(_UNICODE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

	if (BUILD_SKYRIM_AE)
		add_compile_definitions(IS_SKYRIM_AE)
	endif (BUILD_SKYRIM_AE)

	add_definitions(/FI"pch.h")

	set(SC_RELEASE_OPTS "/Zi;/fp:fast;/GL;/Gy-;/Gm-;/Gw;/sdl-;/GS-;/guard:cf-;/O2;/Ob2;/Oi;/Ot;/Oy;/fp:except-")

	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			/MP
			/await
			/W4
			/WX
			/permissive-
			/Zc:alignedNew
			/Zc:auto
			/Zc:__cplusplus
			/Zc:externC
			/Zc:externConstexpr
			/Zc:forScope
			/Zc:hiddenFriend
			/Zc:implicitNoexcept
			/Zc:lambda
			/Zc:noexceptTypes
			/Zc:preprocessor
			/Zc:referenceBinding
			/Zc:rvalueCast
			/Zc:sizedDealloc
			/Zc:strictStrings
			/Zc:ternary
			/Zc:threadSafeInit
			/Zc:trigraphs
			/Zc:wchar_t
	)
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/fp:strict>")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/ZI>")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/Od>")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/Gy>")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${SC_RELEASE_OPTS}>")

	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			/WX
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/LTCG;/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif ()