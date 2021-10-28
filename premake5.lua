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

local function rewriteFile(strPath, strData)
	local f = io.open(strPath, "w")
	assert(f, "Failed to open file ".. strPath.. "!\n")
	f:write(strData)
	f:close()
end

newaction {
	trigger = "dd",
	description = "Auto download all required dependencies and place them in the correct location",
	execute = function()
		term.pushColor( term.lightGreen )
			print( "Checking for command line tools..." )
		term.popColor()

		local found_7z = false
		for str in os.getenv( "PATH" ):gmatch( "([^;]+)" ) do
			if str:find("7-Zip") ~= nil then
				found_7z = true
			end
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
				args = { "Deps/temp_downloads", "Deps", "skse64_2_00_19.7z", "http://skse.silverlock.org/beta/skse64_2_00_19.7z" },
				postExec = function(tbl)
					--Gross hack to deal with SKSE macros and our use of versionlib
					--We blank the macros from this file, then force include our own version with the edits we need
					local root = tbl.args[2].. "/skse64_2_00_19/src"
					rewriteFile(root.. "/skse64/skse64_common/Utilities.h", [[
#pragma once
// this is the solution to getting a pointer-to-member-function pointer
template <typename T>
uintptr_t GetFnAddr(T src)
{
	union
	{
		uintptr_t	u;
		T			t;
	} data;

	data.t = src;

	return data.u;
}

std::string GetRuntimePath();
std::string GetRuntimeName();
const std::string & GetRuntimeDirectory();

const std::string & GetConfigPath();
std::string GetConfigOption(const char * section, const char * key);
bool GetConfigOption_UInt32(const char * section, const char * key, UInt32 * dataOut);

const std::string & GetOSInfoStr();

void * GetIATAddr(void * module, const char * searchDllName, const char * searchImportName);

const char * GetObjectClassName(void * objBase);
void DumpClass(void * theClassPtr, UInt64 nIntsToDump);]])

					--And rewrite this one to use our own impl
					rewriteFile(root.. "/skse64/skse64_common/Relocation.h", [[
#pragma once
#include "addrlib/relocation.h"
					]])
				end,
			},
		}
		for k, v in ipairs( deps ) do
			term.pushColor( term.lightGreen )
				printf( "Downloading %s...", v.name )
			term.popColor()
			v.ok = dofile( v.path )( table.unpack(v.args) )
			if v.ok then
				if v.postExec then v.postExec(v) end
			end
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
dofile( "BuildScripts/project_eastl.lua" )(os.getcwd(), _OPTIONS["VS_PLATFORM"])

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
	dependson { "common_skse64", "skse64_common", "skse64", "PolyHook2", "eastl" }

	files { "SmoothCam/**.h", "SmoothCam/**.cpp", "SmoothCam/**.rc", "SmoothCam/**.def", "SmoothCam/**.ini" }
	pchheader "pch.h"
	pchsource "SmoothCam/source/pch.cpp"
	forceincludes {
		"../Deps/skse64_2_00_19/src/common/IPrefix.h",
		"../SmoothCam/include/addrlib/skse_macros.h",
		"pch.h"
	}
	includedirs {
		"./SmoothCam/include",
		"Deps/skse64_2_00_19/src/skse64",
		"Deps/skse64_2_00_19/src",
		"Deps/glm",
		"Deps/PolyHook_2_0",
		"Deps/eternal/include",
		"Deps/json/single_include",
		"Deps/EASTL/include",
		"Deps/EABase/include/Common",
	}

	filter "system:windows"
		systemversion "latest"
		debugdir( "../bin/".. outputDir.. "/%{prj.name}" )
		vectorextensions "AVX"
		characterset "Unicode"
		intrinsics "On"
		fpu "Hardware"

	filter "configurations:Debug"
		defines { "DEBUG", "SMOOTHCAM_IMPL" }
		buildoptions { "/bigobj" }
		symbols "On"
		editandcontinue "On"
		floatingpoint "Strict"
		functionlevellinking "On"
		flags {
			"MultiProcessorCompile"
		}

		libdirs {
			"Deps/skse64_2_00_19/src/common/bin/Debug-windows-x86_64/common_skse64",
			"Deps/skse64_2_00_19/src/skse64/skse64_common/bin/Debug-windows-x86_64/skse64_common",
			"Deps/skse64_2_00_19/src/skse64/skse64/bin/Debug-windows-x86_64/skse64",

			"Deps/Detours/lib.X64",
			"Deps/PolyHook_2_0/bin/Debug-windows-x86_64/PolyHook2",
			"Deps/EASTL/build/Debug"
		}

		links {
			"common_skse64.lib",
			"skse64_common.lib",
			"skse64.lib",

			"detours.lib",
			"PolyHook2.lib",
			"EASTL.lib"
		}

	filter "configurations:Release"
		defines { "NODEBUG", "NDEBUG", "SMOOTHCAM_IMPL" }
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
			"NoMinimalRebuild", "NoRuntimeChecks", "MultiProcessorCompile"
		}

		libdirs {
			"Deps/skse64_2_00_19/src/common/bin/Release-windows-x86_64/common_skse64",
			"Deps/skse64_2_00_19/src/skse64/skse64_common/bin/Release-windows-x86_64/skse64_common",
			"Deps/skse64_2_00_19/src/skse64/skse64/bin/Release-windows-x86_64/skse64",

			"Deps/Detours/lib.X64",
			"Deps/PolyHook_2_0/bin/Release-windows-x86_64/PolyHook2",
			"Deps/EASTL/build/Release"
		}

		links {
			"common_skse64.lib",
			"skse64_common.lib",
			"skse64.lib",

			"detours.lib",
			"PolyHook2.lib",
			"EASTL.lib"
		}