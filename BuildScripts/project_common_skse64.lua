local outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
local loc = "../Deps/skse64_2_00_19/src/common"

project "common_skse64"
	location( loc )
	language "C++"
	compileas "C++"
	cppdialect "C++17"
	kind "StaticLib"
	staticruntime "On"

	targetdir( loc.. "/bin/".. outputDir.. "/%{prj.name}" )
	objdir( loc.. "/bin-obj/".. outputDir.. "/%{prj.name}" )

	files { loc.. "/**.h", loc.. "/**.cpp" }
	forceincludes { "IPrefix.h" }
	includedirs {
		loc.. "/../",
	}

	links {
		"winmm.lib",
	}

	filter "system:windows"
		systemversion "latest"
		debugdir( "../bin/".. outputDir.. "/%{prj.name}" )
		vectorextensions "AVX"
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
		flags { "MultiProcessorCompile" }

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
			"NoMinimalRebuild", "NoRuntimeChecks", "MultiProcessorCompile"
		}