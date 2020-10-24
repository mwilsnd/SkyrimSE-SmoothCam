do class "StructToVars" : namespace "papyrus.preproc" {
		m_tblStructTypes = {},
		m_tblStructImpls = {},
		m_tblStructMemberUsage = {},
		m_tblStructMacroUsage = {},

		m_strSource = "",
		m_strOrig = "",
	}

	getter "Source->m_strOrig"
	getter "Parsed->m_strSource"

	getter "StructTypes->m_tblStructTypes"
	getter "StructImpls->m_tblStructImpls"

	function Initialize( self, strSource )
		self:SetSource( strSource )
	end

	function SetSource( self, strSource )
		self.m_strOrig = strSource
		self.m_strSource = strSource
	end

	function Preproc( self )
		self.m_tblStructTypes = {}
		self.m_tblStructImpls = {}
		self.m_tblStructMemberUsage = {}
		self.m_tblStructMacroUsage = {}

		self:ParseStructTypes()
		self:ParseStructTypeImpls()
		self:FindAndGenerateStructImplMemberUsage()
		self:FindAndGenerateStructImplMacroUsage()
	end

	function Apply( self )
		self.m_strSource = self:ReplaceGeneratedStructImpls()
		self.m_strSource = self:ReplaceStructDecls()
		self.m_strSource = self:ReplaceGeneratedStructImplMembers()
		self.m_strSource = self:InsertStructImplMacros()
	end

	-- Break a var decl into (type, name, value)
	function ExtractVarDef( self, strVar )
		return strVar:match( "([%a%d_]+)%s+([%a%d_]+)%s*=%s*(.+)" )
	end

	function GenerateNameForStructMemberImpl( self, strStructDecl, strMember )
		local typeInfo = self.m_tblStructTypes[self.m_tblStructImpls[strStructDecl]]
		if not typeInfo.members[strMember] then
			error( "Invalid struct member - ".. strStructDecl.. "::".. strMember..
				" (Of type "..self.m_tblStructImpls[strStructDecl] ..")"
			)
		end
		return {
			name = strStructDecl.. "_".. strMember,
			type = typeInfo.members[strMember].type,
			value = typeInfo.members[strMember].value,
		}
	end

	-- Read all struct type defs into a table
	function ParseStructTypes( self )
		for def, cont in self.m_strSource:gmatch("#struct%s+([%a%d]+)%s*(%b{})") do
			local struct = {
				name = def,
				members = {},
				macros = {},
			}

			local cleaned, _ = cont:sub( 2, cont:len() -1 ):gsub("\t", "")
			-- Match and sub out MACRO defs first
			for name, impl in cleaned:gmatch("MACRO%s+([%a%d_]+)%s*=%s*(%b{})") do
				struct.macros[name] = impl:sub( 2, impl:len() -1 )
			end

			while true do
				local s, e = cleaned:find( "MACRO%s+([%a%d_]+)%s*=%s*(%b{})" )
				if not s then break end

				local prefix = cleaned:sub( 1, s-1 )
				local postfix = cleaned:sub( e+1 )
				cleaned = prefix.. postfix
			end

			for line in cleaned:gmatch("[^\n]+") do
				local type, name, value = self:ExtractVarDef( line )
				struct.members[name] = { type = type, value = value }
			end

			self.m_tblStructTypes[def] = struct
		end
	end

	-- Read all var decls that use a struct type we've seen
	function ParseStructTypeImpls( self )
		for type, name in self.m_strSource:gmatch("([%a%d]+)%s+([%a%d_]+)\n") do
			if self.m_tblStructTypes[type] then
				self.m_tblStructImpls[name] = type
			end
		end
	end

	-- Find all usages of myStructImpl.memberVar and make a generated name
	function FindAndGenerateStructImplMemberUsage( self )
		for decl, member in self.m_strSource:gmatch("([%a%d_]+)%.([%a%d_]+)") do
			if self.m_tblStructImpls[decl] and not self.m_tblStructMemberUsage[decl.."."..member] then
				if self.m_tblStructTypes[self.m_tblStructImpls[decl]] then
					local generated = self:GenerateNameForStructMemberImpl( decl, member )
					self.m_tblStructMemberUsage[decl.."."..member] = generated
				end
			end
		end
	end

	-- Find all usages of myStructImpl->!macro and generate meta data for it
	function FindAndGenerateStructImplMacroUsage( self )
		for decl, member in self.m_strSource:gmatch("([%a%d_]+)%->!([%a%d_]+)") do
			if self.m_tblStructImpls[decl] and not self.m_tblStructMacroUsage[decl.."->!"..member] then
				if self.m_tblStructTypes[self.m_tblStructImpls[decl]] then
					local code = self.m_tblStructTypes[self.m_tblStructImpls[decl]].macros[member]
					code = code:replace( "this->", decl.."_" )
					self.m_tblStructMacroUsage[decl.. "->!".. member] = code:trim()
				end
			end
		end
	end

	-- Replace struct decls with empty space
	function ReplaceStructDecls( self )
		local parsed = self.m_strSource
		for decl, _ in pairs( self.m_tblStructTypes ) do
			local s, e = parsed:find( "#struct%s+".. decl.. "%s*(%b{})" )
			local prefix = parsed:sub( 1, s-3 )
			local postfix = parsed:sub( e+1 )
			parsed = prefix.. postfix
		end
		return parsed
	end

	function ReplaceGeneratedStructImpls( self )
		local parsed = self.m_strSource
		for name, type in pairs( self.m_tblStructImpls ) do
			local s, e = parsed:find( type.."%s+"..name.."\n" )
			local prefix = parsed:sub( 1, s-1 )
			local postfix = parsed:sub( e+1 )

			local generated = {}
			for m_name, _ in pairs( self.m_tblStructTypes[type].members ) do
				local g = self:GenerateNameForStructMemberImpl( name, m_name )
				generated[#generated+1] = ("%s %s = %s"):format(
					g.type,
					g.name,
					g.value
				)
			end

			parsed = prefix.. table.concat( generated, "\n" ).. "\n".. postfix
		end
		return parsed
	end

	-- Replace all instances of struct.member with the generated name
	function ReplaceGeneratedStructImplMembers( self )
		local parsed = self.m_strSource
		for decl, generated in pairs( self.m_tblStructMemberUsage ) do
			parsed = parsed:gsub( decl, generated.name )
		end
		return parsed
	end

	-- Replace all instances of struct.macroMember with the macro code
	function InsertStructImplMacros( self )
		local parsed = self.m_strSource
		for decl, generated in pairs( self.m_tblStructMacroUsage ) do
			parsed = string.replace( parsed, decl, generated )
		end
		return parsed
	end
end