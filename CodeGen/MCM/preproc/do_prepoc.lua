--[[
	This is a dumb little tool I made to do some basic code generation
	for papyrus, to make building the MCM easier

	Running the preprocessor:
		lua do_preproc.lua path/to/my/script.psc path/to/my/output.psc
]]
package.path = "../../shared/?.lua;"

local args = { ... }
assert( args[1] )
assert( args[2] )

function string.replace( text, old, new )
	while true do
		local b, e = text:find( old, 1, true )
		if not b then break end
		text = text:sub( 1, b-1 ).. new.. text:sub( e+1 )
	end
	return text
end

function string.trim( s )
	return string.match( s, "^%s*(.-)%s*$" ) or s
end

function string.split( str, sep )
	local t = {}
	for s in string.gmatch( str, "([^"..sep.."]+)" ) do
		table.insert( t, s )
	end
	return t
end

function table.count( t )
	local i = 0
	for _, _ in pairs( t ) do i = i + 1 end
	return i
end

lava = require "lava"
class = lava.class
lava.loadClass "preproc_array_init_list.lua"
lava.loadClass "preproc_struct.lua"
lava.loadClass "preproc_constexpr_struct.lua"
lava.loadClass "preproc_ifchain_macro_invoke.lua"
lava.loadClass "preproc_all_of_struct_impl.lua"
lava.loadClass "preproc_group_struct_macro_invoke.lua"
lava.loadClass "preproc_generate_offsetGroup.lua"
lava.loadClass "preproc_impl_offsetGroup_page.lua"

local function readFile( strPath )
	local f = io.open( strPath, "r" )
	assert( f )

	local t = {}
	for line in f:lines() do
		t[#t+1] = line
	end

	local src = table.concat( t, "\n" )
	f:close()
	return src
end

local function writeFile( strPath, strData )
	local f = io.open( strPath, "w" )
	assert( f )
	f:write( strData )
	f:close()
end

local function run()
	local src = readFile(args[1])

	-- Group generators
	local groupGen = papyrus.preproc.GenerateOffsetGroup:New( src )
	local groupPageImpl = papyrus.preproc.ImplOffsetGroupPage:New( groupGen:GetParsed(), groupGen )

	-- Run the array initializer list tool
	local initList = papyrus.preproc.ArrayInitList:New( groupPageImpl:GetParsed() )

	-- Run the parsing part of the struct tool
	local structParser = papyrus.preproc.StructToVars:New( initList:GetParsed() )
	structParser:Preproc()

	-- And the constexpr struct tool
	local constStructParser = papyrus.preproc.ConstexprStructToVars:New( initList:GetParsed() )
	constStructParser:Preproc()

	-- Feed the struct type info into the all of struct type macro tool
	local allOfStructParser = papyrus.preproc.AllOfStructImpl:New( initList:GetParsed(), structParser, constStructParser )

	-- Run the group macro invoke tool
	local groupMacroInvoke = papyrus.preproc.GroupStructMacroInvoke:New( allOfStructParser:GetParsed() )

	-- Feed the processed code into the if chain macro tool
	local ifChainParser = papyrus.preproc.IfChainMacroInvoke:New( groupMacroInvoke:GetParsed(), constStructParser )

	-- Now re-parse and apply the struct tool
	structParser:SetSource( ifChainParser:GetParsed() )
	structParser:Preproc()
	structParser:Apply()

	-- And the constexpr struct tool
	constStructParser:SetSource( structParser:GetParsed() )
	constStructParser:Preproc()
	constStructParser:Apply()

	-- Done
	writeFile( args[2], constStructParser:GetParsed() )
end

run()