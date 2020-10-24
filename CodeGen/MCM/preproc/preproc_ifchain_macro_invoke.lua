do class "IfChainMacroInvoke" : namespace "papyrus.preproc" {
		m_tblCalls = {},

		m_strOrig = "",
		m_strSource = "",

		m_strPattern = "(%s*)IMPL_IFCHAIN_MACRO_INVOKE%s*%(%s*([%a%d_]+)%s*,%s*([%a%d_]+)%s*,%s*([%a%d_]+)%s*,%s*(%b{})%s*%)"
	}

	getter "Source->m_strOrig"
	getter "Parsed->m_strSource"

	function Initialize( self, strSource )
		self.m_strOrig = strSource
		self.m_strSource = strSource
		self:GenerateCalls()
		self.m_strSource = self:ReplaceCalls()
	end

	function GenerateCalls( self )
		for indent, lhs, rhs_member, rhs_macro, structs in self.m_strSource:gmatch( self.m_strPattern ) do
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
				lhs = lhs, rhs_member = rhs_member,
				rhs_macro = rhs_macro, structs = structs,
				output = output
			}
		end
	end

	function ReplaceCalls( self )
		local parsed = self.m_strSource
		for _, call in pairs( self.m_tblCalls ) do
			while true do
				local str = ("IMPL_IFCHAIN_MACRO_INVOKE%s*%(%s*_1_%s*,%s*_2_%s*,%s*_3_%s*,%s*_4_%s*%)")
					:replace( "_1_", call.lhs )
					:replace( "_2_", call.rhs_member )
					:replace( "_3_", call.rhs_macro )
					:replace( "_4_", call.structs )

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