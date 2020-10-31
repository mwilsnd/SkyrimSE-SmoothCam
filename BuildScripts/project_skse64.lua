local outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
local parent = "../Deps/skse64_2_00_19/src/skse64"
local loc = parent.. "/skse64"

project "skse64"
	location( loc )
	language "C++"
	compileas "C++"
	cppdialect "C++17"
	kind "StaticLib"
	staticruntime "On"

	targetdir( loc.. "/bin/".. outputDir.. "/%{prj.name}" )
	objdir( loc.. "/bin-obj/".. outputDir.. "/%{prj.name}" )

	dependson { "common_skse64", "skse64_common" }

	files {
		loc.. "/**.h",
		loc.. "/**.cpp",
	}
	forceincludes {
		"../Deps/skse64_2_00_19/src/common/IPrefix.h",
		"../SmoothCam/include/addrlib/skse_macros.h",
	}
	includedirs {
		parent,
		parent.. "/../",
		"../SmoothCam/include",
		"../Deps/eternal/include",
	}

	filter "system:windows"
		systemversion "latest"
		debugdir( "../bin/".. outputDir.. "/%{prj.name}" )
		vectorextensions( _SIMD_MODE )
		characterset "MBCS"
		intrinsics "On"
		fpu "Hardware"
		flags {
			"MultiProcessorCompile"
		}

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

		libdirs {
			parent.. "/../common/bin/Debug-windows-x86_64/common_skse64",
			--parent.. "/skse64_common/bin/Debug-windows-x86_64/skse64_common",
		}
		links {
			--"skse64_common.lib",
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
			"NoMinimalRebuild", "NoRuntimeChecks", "MultiProcessorCompile"
		}

		libdirs {
			parent.. "/../common/bin/Release-windows-x86_64/common_skse64",
			--parent.. "/skse64_common/bin/Release-windows-x86_64/skse64_common",
		}
		links {
			--"skse64_common.lib",
		}