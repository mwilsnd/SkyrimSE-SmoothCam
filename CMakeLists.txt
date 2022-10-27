cmake_minimum_required(VERSION 3.21.0)

set(VCPKG_OVERLAY_PORTS "" CACHE STRING "")

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

set(SKSE_SUPPORT_XBYAK ON CACHE BOOL "")
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "")
set(Boost_USE_STATIC_RUNTIME ON CACHE BOOL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")
set(POLYHOOK_BUILD_STATIC_RUNTIME ON CACHE BOOL "")
set(POLYHOOK_BUILD_DLL ON CACHE BOOL "")
set(POLYHOOK_DISASM_ZYDIS ON CACHE BOOL "")
set(POLYHOOK_FEATURE_EXCEPTION OFF CACHE BOOL "")
set(POLYHOOK_FEATURE_DETOURS ON CACHE BOOL "")
set(POLYHOOK_FEATURE_INLINENTD OFF CACHE BOOL "")
set(POLYHOOK_FEATURE_PE ON CACHE BOOL "")
set(POLYHOOK_FEATURE_VIRTUALS ON CACHE BOOL "")
set(WITH_TESTS OFF CACHE BOOL "")

project(
	SmoothCam
	LANGUAGES CXX
)

add_subdirectory("Deps/CommonLibSSE" CommonLibSSE)
add_subdirectory("Deps/EASTL" EASTL)
add_subdirectory("Deps/PolyHook_2_0" PolyHook_2)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(binary_io CONFIG REQUIRED)
find_package(xbyak CONFIG REQUIRED)

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
		xbyak::xbyak
		"$<$<CONFIG:RELEASE>:${PROJECT_SOURCE_DIR}/Deps/Detours/lib.X64/detours.lib>"
		"$<$<CONFIG:DEBUG>:${PROJECT_SOURCE_DIR}/Deps/Detours/lib.X64Debug/detours.lib>"
)

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		SmoothCam/include/pch.h
)

target_compile_features(
	${PROJECT_NAME}
	PRIVATE
		cxx_std_23
)

if (MSVC)
	add_compile_definitions(_UNICODE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

	add_compile_definitions(SKSE_SUPPORT_XBYAK)
	
	add_definitions(/FI"pch.h")

	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			/MP
			/await
			/W4
			/WX
			/permissive-
			/Zc:__cplusplus
			/Zc:externConstexpr
			/Zc:inline
			/Zc:lambda
			/Zc:referenceBinding
			/Zc:rvalueCast
			/Zc:strictStrings
			/Zc:ternary
			/Zc:wchar_t
	)
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/Gy;/Od;/ZI;/fp:strict>")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:/Zi;/fp:fast;/GL;/Gy-;/Gw;/sdl-;/GS-;/guard:cf-;/O2;/Ob2;/Oi;/Ot;/Oy;/fp:except->")

	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			/WX
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/LTCG;/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL;/IGNORE:4099>"
	)
endif ()