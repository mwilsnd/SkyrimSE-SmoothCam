do class "IfChainMacroInvoke" : namespace "papyrus.preproc" {
		m_tblCalls = {},

		m_strOrig = "",
		m_strSource = "",
		m_pConstStructParser = nil,

		m_strPattern = "(%s*)IMPL_IFCHAIN_MACRO_INVOKE%s*%(%s*([%a%d_]+)%s*,%s*([%a%d_]+)%s*,%s*([%a%d_]+)%s*,%s*(%b{})%s*,%s*([%a%d_]+)%s*%)"
	}

	getter "Source->m_strOrig"
	getter "Parsed->m_strSource"

	function Initialize( self, strSource, pConstStructParser )
		self.m_strOrig = strSource
		self.m_strSource = strSource
		self.m_pConstStructParser = pConstStructParser
		self:GenerateUsage()
		--self:GenerateCalls()
		self.m_strSource = self:ReplaceCalls()
	end

	function GenerateUsage( self )
		for indent, lhs, rhs_member, rhs_macro, structs, pageVarName in self.m_strSource:gmatch( self.m_strPattern ) do
			indent = indent:gsub("\n", "")
			local cleaned = structs:gsub("\n", ""):gsub("\r", ""):gsub("\t", "")
			cleaned = cleaned:sub(2, cleaned:len() -1)

			local invocations = {}
			for _, struct in pairs( cleaned:split(",") ) do
				if self.m_pConstStructParser:GetStructImpls()[struct] then
					local members = self.m_pConstStructParser:GetStructImpls()[struct].members
					if members.page and members.page:len() > 0 then
						invocations[members.page] = invocations[members.page] or {}
						invocations[members.page][struct] = {
							lhs = lhs,
							member_compare = rhs_member,
							macro = rhs_macro,
						}
					else
						print(
							"Warning: Member `page` was not found or defined while reading struct "..
							struct.. " for macro IMPL_IFCHAIN_MACRO_INVOKE"
						)
					end
				else
					print(
						"Warning: Type info was not found while reading struct "..
						struct.. " for macro IMPL_IFCHAIN_MACRO_INVOKE"
					)
				end
			end

			local output = {}
			local first = true
			for page, collection in pairs( invocations ) do
				local case = indent.. "elseIf"
				if first then
					case = "if"
					first = false
				end

				case = case.. (" (%s == %s)"):format(pageVarName, page)
				table.insert(output, case)

				local subFirst = true
				for structName, context in pairs(collection) do
					local case = "elseIf"
					if subFirst then
						case = "if"
						subFirst = false
					end

					table.insert(output, ("%s\t%s (%s == %s.%s)"):format(
						indent, case, lhs:trim(), structName, context.member_compare:trim()
					))
					table.insert(output, ("%s\t\t%s->!%s"):format(
						indent, structName, context.macro
					))
				end
				table.insert(output, indent.. "\tendIf")
			end

			table.insert(output, indent.. "endIf")
			--print( table.concat(output, "\n") )
			table.insert(self.m_tblCalls, {
				lhs = lhs,
				rhs_member = rhs_member,
				rhs_macro = rhs_macro,
				structs = structs,
				pageName = pageVarName,
				output = table.concat(output, "\n")
			})
		end
	end

	function GenerateCalls( self )
		for indent, lhs, rhs_member, rhs_macro, structs, pageName in self.m_strSource:gmatch( self.m_strPattern ) do
			indent = indent:gsub( "\n", "" )

			local output = ""
			local first = true
			local cleaned = structs:gsub("\n", ""):gsub("\r", ""):gsub("\t", "")
			cleaned = cleaned:sub(2, cleaned:len() -1)
			for _, struct in pairs( cleaned:split(",") ) do
				local case = indent.. "elseIf"
				if first then
					case = "if"
					first = false
				end

				output = output.. ("%s (%s == %s.%s)\n"):format(
					case,
					lhs:trim(),
					struct:trim(),
					rhs_member:trim()
				)
				output = output.. ("%s\t%s->!%s\n"):format( indent, struct, rhs_macro )
			end
			output = output.. indent.. "endIf"

			self.m_tblCalls[#self.m_tblCalls +1] = {
				lhs = lhs,
				rhs_member = rhs_member,
				rhs_macro = rhs_macro,
				structs = structs,
				pageName = pageName,
				output = output
			}
		end
	end

	function ReplaceCalls( self )
		local parsed = self.m_strSource
		for _, call in pairs( self.m_tblCalls ) do
			while true do
				local str = ("IMPL_IFCHAIN_MACRO_INVOKE%s*%(%s*_1_%s*,%s*_2_%s*,%s*_3_%s*,%s*_4_%s*,%s*_5_%s*%)")
					:replace( "_1_", call.lhs )
					:replace( "_2_", call.rhs_member )
					:replace( "_3_", call.rhs_macro )
					:replace( "_4_", call.structs )
					:replace( "_5_", call.pageName )

				local s, e = parsed:find( str )
				if not s then break end

				local prefix = parsed:sub( 1, s-1 )
				local postfix = parsed:sub( e+1 )
				parsed = prefix.. call.output.. postfix
			end
		end
		return parsed
	end
end