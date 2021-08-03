local outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
local loc = "/Deps/EASTL"
local locBase = "/Deps/EABase"

local function readFile(strPath)
	local f = io.open(strPath, "r")
	assert( f )

	local t = {}
	for line in f:lines() do
		t[#t+1] = line
	end

	local src = table.concat(t, "\n")
	f:close()
	return src
end

local function writeFile(strPath, strData)
	local f = io.open(strPath, "w")
	assert(f, "Failed to open file ".. strPath.. "!\n")
	f:write(strData)
	f:close()
end

return function( root, platform )
	local args = "-G"

	if platform == "vs2019" then
		args = args.. "Visual Studio 16 2019"
	else
		args = args.. "Visual Studio 15 2017 Win64"
	end

	-- CMake blows
	-- No way (that I can see) to force the static runtime on the command line
	-- Rewrite the cmakelists file to do this for the targets we care about
	local flags = [[set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")]]
	-- Lua also blows
	local flagsEscaped = [[set%(CMAKE_CXX_FLAGS_RELEASE "%${CMAKE_CXX_FLAGS_RELEASE} /MT"%)
set%(CMAKE_CXX_FLAGS_DEBUG "%${CMAKE_CXX_FLAGS_DEBUG} /MTd"%)]]

	local data = readFile(root.. loc.. "/CMakeLists.txt")
	if not data:find(flagsEscaped) then
		data = data.. "\n".. flags
		writeFile(root.. loc.. "/CMakeLists.txt", data)
	end

	os.execute(("call \"%s\" \"%s\" \"%s\" \"%s\""):format(
		"BuildScripts/build_eastl.bat",
		root.. loc .. "/build",
		root.. loc,
		args
	))
	externalproject "EASTL"
	   location(root.. loc .. "/build")
	   uuid "5BC9CEB8-8B4A-11D0-8D21-01A0C91FF942"
	   kind "StaticLib"
	   language "C++"
end