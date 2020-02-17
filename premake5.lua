newoption {
	trigger = "VS_PLATFORM",
	value = "vs2019",
	description = "Select the version of Visual Studio Community to use.",
	allowed = {
		{ "vs2017", "Visual Studio Community 2017" },
		{ "vs2019", "Visual Studio Community 2019" },
	}
}

if not _OPTIONS["VS_PLATFORM"] then
	return error( "No visual studio platform selected, please set --VS_PLATFORM to vs2017 or vs2019" )
end

newaction {
	trigger = "dd",
	description = "Auto download all required dependencies and place them in the correct location",
	execute = function()
		term.pushColor( term.lightGreen )
			print( "Checking for command line tools..." )
		term.popColor()

		local found_git = false
		local found_7z = false
		for str in os.getenv( "PATH" ):gmatch( "([^;]+)" ) do
			if str:find("git\\cmd") ~= nil then
				found_git = true
			end
			if str:find("7-Zip") ~= nil then
				found_7z = true
			end			
		end

		if not found_git then
			term.pushColor( term.errorColor )
				print( "git not found! Please install git for windows!" )
			term.popColor()
			return
		end

		if not found_7z then
			term.pushColor( term.errorColor )
				print( "7-Zip not found! Please install 7-Zip for windows and place the folder containing 7z.exe in the system PATH." )
			term.popColor()
			return
		end

		term.pushColor( term.lightGreen )
			print( "Downloading dependencies, please wait..." )
		term.popColor()

		os.mkdir( "Deps" )

		local deps = {
			{
				ok = false, name = "SKSE64", path = "BuildScripts/download_file.lua",
				args = {"Deps/temp_downloads", "Deps", "skse64_2_00_17.7z", "https://skse.silverlock.org/beta/skse64_2_00_17.7z" }
			},
		}
		for k, v in ipairs( deps ) do
			term.pushColor( term.lightGreen )
				printf( "Downloading %s...", v.name )
			term.popColor()
			v.ok = dofile( v.path )( table.unpack(v.args) )
		end

		for k, v in ipairs( deps ) do
			if not v.ok then
				term.pushColor( term.errorColor )
					printf( "Error downloading %s! Please aquire this dependency manually.", v.name )
				term.popColor()
			else
				term.pushColor( term.lightGreen )
					printf( "Finished downloading %s!", v.name )
				term.popColor()
			end
		end
	end,
}

newaction {
	trigger = "dmake",
	description = "Build dependencies",
	execute = function()
		term.pushColor( term.lightGreen )
			print( "Building Microsoft Detours..." )
		term.popColor()

		print( dofile( "BuildScripts/project_ms_detours.lua" )( os.getcwd().. "/Deps/Detours", _OPTIONS["VS_PLATFORM"] ) )

		term.pushColor( term.lightGreen )
			print( "Done." )
		term.popColor()
	end,
}

workspace "SmoothCam"
	architecture "x64"
	configurations { "Debug", "Release" }
	startproject "SmoothCam"

dofile( "BuildScripts/project_common_skse64.lua" )
dofile( "BuildScripts/project_skse64_common.lua" )
dofile( "BuildScripts/project_skse64.lua" )
dofile( "BuildScripts/project_polyhook.lua" )

local outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
project "SmoothCam"
	location "SmoothCam"
	language "C++"
	compileas "C++"
	cppdialect "C++17"
	kind "SharedLib"
	staticruntime "On"

	targetdir( "SmoothCam/bin/".. outputDir.. "/%{prj.name}" )
	objdir( "SmoothCam/bin-obj/".. outputDir.. "/%{prj.name}" )
	dependson { "common_skse64", "skse64_common", "skse64", "PolyHook2" }

	files { "SmoothCam/**.h", "SmoothCam/**.cpp", "SmoothCam/**.rc", "SmoothCam/**.def", "SmoothCam/**.ini" }
	pchheader "pch.h"
	pchsource "SmoothCam/source/pch.cpp"
	forceincludes { "../Deps/skse64_2_00_17/src/common/IPrefix.h" }
	includedirs {
		"./SmoothCam/include",
		"Deps/skse64_2_00_17/src/skse64",
		"Deps/skse64_2_00_17/src",
		"Deps/glm",
		"Deps/PolyHook_2_0",
		"Deps/eternal/include",
		"Deps/cereal/include",
	}

	filter "system:windows"
		systemversion "latest"
		debugdir( "../bin/".. outputDir.. "/%{prj.name}" )
		vectorextensions "AVX"
		characterset "Unicode"
		intrinsics "On"
		fpu "Hardware"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		editandcontinue "On"
		floatingpoint "Strict"
		functionlevellinking "On"

		libdirs {
			"Deps/skse64_2_00_17/src/common/bin/Debug-windows-x86_64/common_skse64",
			"Deps/skse64_2_00_17/src/skse64/skse64_common/bin/Debug-windows-x86_64/skse64_common",
			"Deps/skse64_2_00_17/src/skse64/skse64/bin/Debug-windows-x86_64/skse64",

			"Deps/Detours/lib.X64",
			"Deps/PolyHook_2_0/bin/Debug-windows-x86_64/PolyHook2",
		}

		links {
			"common_skse64.lib",
			"skse64_common.lib",
			"skse64.lib",

			"detours.lib",
			"PolyHook2.lib",
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
			"LinkTimeOptimization", "FatalWarnings", "NoBufferSecurityCheck",
			"NoMinimalRebuild", "NoRuntimeChecks",
		}

		libdirs {
			"Deps/skse64_2_00_17/src/common/bin/Release-windows-x86_64/common_skse64",
			"Deps/skse64_2_00_17/src/skse64/skse64_common/bin/Release-windows-x86_64/skse64_common",
			"Deps/skse64_2_00_17/src/skse64/skse64/bin/Release-windows-x86_64/skse64",

			"Deps/Detours/lib.X64",
			"Deps/PolyHook_2_0/bin/Release-windows-x86_64/PolyHook2",
		}

		links {
			"common_skse64.lib",
			"skse64_common.lib",
			"skse64.lib",
			
			"detours.lib",
			"PolyHook2.lib",
		}