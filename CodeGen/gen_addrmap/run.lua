local fs = require "fs"
local args = {...}
local path = args[1]

local function enum(strDir, contents)
	for name, d in fs.dir(strDir) do
		if d:attr("type") == "dir" then
			enum(strDir.. "\\".. name, contents)
		elseif d:attr("type") == "file" and (
			name:match(".+%.cpp") or
			name:match(".+%.h") or
			name:match(".+%.hpp") or
			name:match(".+%.inl")
		) then

			contents[#contents+1] = strDir.. "\\".. name
		end
	end
end

local fileList = {}
enum(path, fileList)

print "Scanning the following files..."
for _, v in ipairs(fileList) do
	print(v)
end

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
	assert(f)
	f:write(strData)
	f:close()
end

local offsets = {}
local ofs = readFile("offsets.txt")
for k, v in ofs:gmatch("(%w+)\t(%w+)") do
	offsets[tonumber(v, 16)] = k
end

-- Scan each file for member funs and Relocs
local numMemberFuns = 0
local numRelocAddrs = 0
local numRelocPtrs = 0
local addrs = {}
for _, v in ipairs(fileList) do
	local s = readFile(v)

	for addr in s:gmatch("DEFINE_MEMBER_FN%s*%(%s*[%w%d%s%*_<>]+%s*,%s*[%w%d%s%*_<>]+%s*,%s*([%dxXABCDEFabcdef0]+%s*)") do
		if addr == "0" then addr = "0x00000000" end
		addrs[addr] = tonumber(addr, 16) == 0 and 0 or offsets[tonumber(addr, 16)]
		if not addrs[addr] then
			print("WARNING: Address ".. addr.. " is not in the offsets db! Mapping this value to 0. File: ".. v)
			addrs[addr] = 0
		else
			numMemberFuns = numMemberFuns +1
		end
	end

	for addr in s:gmatch("RelocAddr%s*<%s*[%w%d%s%*_<>]+%s*>%s*[%w%d%s%*_<>]+%s*%(%s*([%dxABCDEFabcdef]+)%s*%)") do
		if addr == "0" then addr = "0x00000000" end
		addrs[addr] = tonumber(addr, 16) == 0 and 0 or offsets[tonumber(addr, 16)]
		if not addrs[addr] then
			print("WARNING: Address ".. addr.. " is not in the offsets db! Mapping this value to 0. File: ".. v)
			addrs[addr] = 0
		else
			numRelocAddrs = numRelocAddrs +1
		end
	end

	for addr in s:gmatch("RelocPtr%s*<%s*[%w%d%s%*_<>]+%s*>%s*[%w%d%s%*_<>]+%s*%(%s*([%dxABCDEFabcdef]+)%s*%)") do
		if addr == "0" then addr = "0x00000000" end
		addrs[addr] = tonumber(addr, 16) == 0 and 0 or offsets[tonumber(addr, 16)]
		if not addrs[addr] then
			print("WARNING: Address ".. addr.. " is not in the offsets db! Mapping this value to 0. File: ".. v)
			addrs[addr] = 0
		else
			numRelocPtrs = numRelocPtrs +1
		end
	end
end

local sorted = {}
for k, v in pairs(addrs) do
	sorted[#sorted+1] = {
		addr = k,
		addrN = tonumber(k, 16),
		id = v
	}
end

table.sort(sorted, function(a, b)
	return a.addrN < b.addrN
end)

print(("Found %d member functions, %d reloc addrs, %d reloc ptrs"):format(
	numMemberFuns, numRelocAddrs, numRelocPtrs
))
print(("%d unique addresses found"):format(#sorted))
print "Generating code..."

local addrLines = {}
for _, addr in ipairs(sorted) do
	addrLines[#addrLines+1] = ("\t{ %s, %s }"):format(addr.addr, addr.id)
end

writeFile(
	"addrmap.txt", ("// Generated code from code_gen/gen_addrmap\nconstexpr const auto addrMap = mapbox::eternal::map<uintptr_t, uintptr_t>({\n%s\n});"):format(
		table.concat(addrLines, ",\n")
	)
)