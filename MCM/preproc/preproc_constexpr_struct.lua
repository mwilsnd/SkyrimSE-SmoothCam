do class "ConstexprStructToVars" : namespace "papyrus.preproc" {
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
		local typeInfo = self.m_tblStructTypes[self.m_tblStructImpls[strStructDecl].type]
		if not typeInfo.members[strMember] then
			error(
				"Invalid struct member - ".. strStructDecl.. "::".. strMember..
				" (Of type "..self.m_tblStructImpls[strStructDecl] ..")"
			)
		end
		return {
			name = strStructDecl.. "_".. strMember,
			type = typeInfo.members[strMember].type:sub( 6 ),
			value = typeInfo.members[strMember].value,
		}
	end

	-- Read all struct type defs into a table
	function ParseStructTypes( self )
		for def, cont in self.m_strSource:gmatch("#constexpr_struct%s+([%a%d]+)%s*(%b{})") do
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
				local isreal = type:sub( 1, 5 ) == "real_"
				struct.members[name] = { type = type, value = value, isreal = isreal }
			end

			self.m_tblStructTypes[def] = struct
		end
	end

	-- Read all var decls that use a struct type we've seen
	function ParseStructTypeImpls( self )
		for type, name, decl in self.m_strSource:gmatch("([%a%d_]+)%s+([%a%d_]+)%s*%->%s*(%b{})") do
			if self.m_tblStructTypes[type] then
				local t = {}
				local cleaned = decl:gsub("\r", ""):gsub("\t", "")
				cleaned = cleaned:sub(2, cleaned:len() -1)
				for _, member in pairs( cleaned:split("\n") ) do
					local var, value = member:match("([%a%d_]+)%s*:%s*(.*)%s*")
					assert(
						self.m_tblStructTypes[type].members[var],
						"Invalid assignment to var ".. var.. " of constexpr_struct ".. name
					)
					t[var] = value
				end

				self.m_tblStructImpls[name] = {
					type = type,
					members = t,
				}
			end
		end
	end

	-- Find all usages of myStructImpl.memberVar and make a generated name
	function FindAndGenerateStructImplMemberUsage( self )
		for decl, member in self.m_strSource:gmatch("([%a%d_]+)%.([%a%d_]+)") do
			if self.m_tblStructImpls[decl] and not self.m_tblStructMemberUsage[decl.."."..member] then
				local typeInfo = self.m_tblStructTypes[self.m_tblStructImpls[decl].type]
				assert(
					typeInfo,
					"Unknown var ".. member.. " of constexpr_struct ".. decl
				)

				if typeInfo.members[member].isreal then
					local generated = self:GenerateNameForStructMemberImpl( decl, member )
					self.m_tblStructMemberUsage[decl.."."..member] = generated
				else
					-- Insert the value literal
					local value = self.m_tblStructImpls[decl].members[member]
					if not value then
						self.m_tblStructMemberUsage[decl.."."..member] = { name = typeInfo.members[member].value }
					else
						self.m_tblStructMemberUsage[decl.."."..member] = { name = value }
					end
				end
			end
		end
	end

	-- Find all usages of myStructImpl->!macro and generate meta data for it
	function FindAndGenerateStructImplMacroUsage( self )
		local generated = {}
		for indent, decl, member in self.m_strSource:gmatch("(%s*)([%a%d_]+)%->!([%a%d_]+)") do
			if self.m_tblStructImpls[decl] and not self.m_tblStructMacroUsage[decl.. "->!".. member] then
				local code = self.m_tblStructTypes[self.m_tblStructImpls[decl].type].macros[member]
				code = code:replace( "this->", decl.."." )

				indent = indent:gsub( "\n", "" )
				local code_t = {}
				for k, line in ipairs( code:split("\n") ) do
					code_t[k] = indent.. line
				end
				code = table.concat( code_t, "\n" )

				for gen_decl, gen_member in code:gmatch("([%a%d_]+)%.([%a%d_]+)") do
					gen_decl = gen_decl:trim()
					gen_member = gen_member:trim()

					if not generated[member.."."..gen_decl.."."..gen_member] then
						local typeInfo = self.m_tblStructTypes[self.m_tblStructImpls[gen_decl].type]

						local usage = self.m_tblStructMemberUsage[gen_decl.."."..gen_member]
						if usage then
							code = code:replace( gen_decl.."."..gen_member, usage.name )
						else
							if typeInfo.members[gen_member].isreal then
								code = code:replace( gen_decl.."."..gen_member, gen_decl.. "_".. gen_member )
							else
								-- Value literal
								local value = self.m_tblStructImpls[gen_decl].members[gen_member]
								if not value then
									value = typeInfo.members[gen_member].value
								end
								code = code:replace( gen_decl.."."..gen_member, value )
							end
						end

						generated[member.."."..gen_decl.."."..gen_member] = true
					end
				end

				self.m_tblStructMacroUsage[decl.. "->!".. member] = code:trim()
			end
		end
	end

	-- Replace struct decls with empty space
	function ReplaceStructDecls( self )
		local parsed = self.m_strSource
		for decl, _ in pairs( self.m_tblStructTypes ) do
			local s, e = parsed:find( "#constexpr_struct%s+".. decl.. "%s*(%b{})" )
			local prefix = parsed:sub( 1, s-3 )
			local postfix = parsed:sub( e+1 )
			parsed = prefix.. postfix
		end
		return parsed
	end

	function ReplaceGeneratedStructImpls( self )
		local parsed = self.m_strSource
		for name, meta in pairs( self.m_tblStructImpls ) do
			local s, e = parsed:find( meta.type.."%s+"..name.."%s*%->%s*(%b{})" )
			local prefix = parsed:sub( 1, s-1 )
			local postfix = parsed:sub( e+1 )

			local generated = {}
			for m_name, m_meta in pairs( self.m_tblStructTypes[meta.type].members ) do
				if m_meta.isreal then
					local g = self:GenerateNameForStructMemberImpl( name, m_name )
					generated[#generated+1] = ("%s %s = %s"):format(
						g.type,
						g.name,
						g.value
					)
				end
			end

			parsed = prefix.. table.concat( generated, "\n" ).. postfix
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