do class "GroupStructMacroInvoke" : namespace "papyrus.preproc" {
		m_tblCalls = {},

		m_strOrig = "",
		m_strSource = "",

		m_strPattern = "(%s*)IMPL_STRUCT_MACRO_INVOKE_GROUP%s*%(%s*([%a%d_]+)%s*,%s*(%b{})%s*%)",
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
		for indent, macro, structs in self.m_strSource:gmatch( self.m_strPattern ) do
			indent = indent:gsub( "\n", "" )

			local t = {}
			local first = true
			local cleaned = structs:gsub("\n", ""):gsub("\r", ""):gsub("\t", "")
			cleaned = cleaned:sub(2, cleaned:len() -1)
			for _, struct in pairs( cleaned:split(",") ) do
				struct = struct:trim()
				t[#t +1] = (first and "" or indent).. struct.. "->!".. macro
				first = false
			end

			self.m_tblCalls[#self.m_tblCalls +1] = {
				macro = macro,
				structs = structs,
				output = table.concat( t, "\n" )
			}
		end
	end

	function ReplaceCalls( self )
		local parsed = self.m_strSource
		for _, call in pairs( self.m_tblCalls ) do
			while true do
				local str = ("IMPL_STRUCT_MACRO_INVOKE_GROUP%s*%(%s*_1_%s*,%s*_2_%s*%)")
					:replace( "_1_", call.macro )
					:replace( "_2_", call.structs )

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