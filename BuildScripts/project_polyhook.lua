local outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
local loc = "../Deps/PolyHook_2_0"

project "PolyHook2"
	location( loc )
	language "C++"
	compileas "C++"
	cppdialect "C++17"
	kind "StaticLib"
	staticruntime "On"

	targetdir( loc.. "/bin/".. outputDir.. "/%{prj.name}" )
	objdir( loc.. "/bin-obj/".. outputDir.. "/%{prj.name}" )

	files {
		loc.. "/headers/Virtuals/VTableSwapHook.hpp",
		loc.. "/headers/Virtuals/VFuncSwapHook.hpp",
		loc.. "/headers/Enums.hpp",
		loc.. "/header/IHook.hpp",
		loc.. "/header/MemProtector.hpp",
		loc.. "/header/Misc.hpp",
		loc.. "/header/Instruction.hpp",
		loc.. "/header/ADisassembler.hpp",

		loc.. "/sources/VTableSwapHook.cpp",
		loc.. "/sources/VFuncSwapHook.cpp",
		loc.. "/sources/MemProtector.cpp",
	}

	includedirs {
		loc
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

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		editandcontinue "On"
		floatingpoint "Strict"
		functionlevellinking "On"

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