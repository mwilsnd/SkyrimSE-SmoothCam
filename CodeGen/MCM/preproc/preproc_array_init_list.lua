do class "ArrayInitList" : namespace "papyrus.preproc" {
		m_tblArrays = {},

		m_strOrig = "",
		m_strSource = "",

		m_strPattern = "(%s*)([%a%d_]+)%s*=%s*new%s+(%a+)%s*%[%s*%]%s*%->%s*(%b{})",
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
		for indent, decl, type, members in self.m_strSource:gmatch( self.m_strPattern ) do
			indent = indent:gsub( "\n", "" )
			local decl_nice = decl:trim()
			local t = {}

			local cleaned = members:gsub("\n", ""):gsub("\r", ""):gsub("\t", "")
			cleaned = cleaned:sub(2, cleaned:len() -1)
			for _, member in pairs( cleaned:split(",") ) do
				local sz_t = #t
				t[sz_t +1] = ("%s%s[%d] = %s"):format( indent, decl_nice, sz_t, member:trim() )
			end

			self.m_tblArrays[#self.m_tblArrays +1] = {
				decl = decl,
				decl_nice = decl_nice,
				type = type,
				members = members,
				size = #t,
				output = table.concat( t, "\n" )
			}
		end
	end

	function ReplaceCalls( self )
		local parsed = self.m_strSource
		for _, arr in pairs( self.m_tblArrays ) do
			while true do
				local str = ("_1_%s*=%s*new%s+_2_%s*%[%s*%]%s*%->%s*_3_")
					:replace( "_1_", arr.decl )
					:replace( "_2_", arr.type )
					:replace( "_3_", arr.members )

				local s, e = parsed:find( str )
				if not s then break end

				local generated = ("%s = new %s[%d]\n%s"):format(
					arr.decl_nice, arr.type, arr.size, arr.output
				)

				local prefix = parsed:sub( 1, s-1 )
				local postfix = parsed:sub( e+1 )
				parsed = prefix.. generated.. postfix
			end
		end
		return parsed
	end
end