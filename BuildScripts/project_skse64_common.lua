local outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
local parent = "../Deps/skse64_2_00_17/src/skse64"
local loc = parent.. "/skse64_common"

project "skse64_common"
	location( loc )
	language "C++"
	compileas "C++"
	cppdialect "C++17"
	kind "StaticLib"
	staticruntime "On"

	targetdir( loc.. "/bin/".. outputDir.. "/%{prj.name}" )
	objdir( loc.. "/bin-obj/".. outputDir.. "/%{prj.name}" )

	dependson { "common_skse64" }

	files { loc.. "/**.h", loc.. "/**.cpp" }
	forceincludes  { parent.. "/../common/IPrefix.h" }
	includedirs {
		parent,
		parent.. "/../"
	}

	filter "system:windows"
		systemversion "latest"
		debugdir( "../bin/".. outputDir.. "/%{prj.name}" )
		vectorextensions( _SIMD_MODE )
		characterset "MBCS"
		intrinsics "On"
		fpu "Hardware"

		defines {
			"_USRDLL",
			"SKSE64_EXPORTS",
			"RUNTIME",
			"RUNTIME_VERSION=0x01050610",
		}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		editandcontinue "On"
		floatingpoint "Strict"
		functionlevellinking "On"

		libdirs {
			parent.. "/../common/bin/Debug-windows-x86_64/common_skse64",
		}
		links {
			"common_skse64.lib",
		}

	filter "configurations:Release"
		defines { "NODEBUG", "NDEBUG" }
		linkoptions { "/LTCG" }
		buildoptions { "/Ob2", "/Ot" }
		optimize "Speed"
		editandcontinue "Off"
		floatingpoint "Fast"
		functionlevellinking "Off"
		runtime "Release"
		omitframepointer "On"
		flags {
			"LinkTimeOptimization", "NoBufferSecurityCheck",
			"NoMinimalRebuild", "NoRuntimeChecks",
		}

		libdirs {
			parent.. "/../common/bin/Release-windows-x86_64/common_skse64",
		}
		links {
			"common_skse64.lib",
		}