do class "AllOfStructImpl" : namespace "papyrus.preproc" {
		m_tblCalls = {},

		m_strOrig = "",
		m_strSource = "",

		m_strPattern = "IMPL_ALL_IMPLS_OF_STRUCT%s*%(%s*([%a%d_]+)%s*%)",
		m_pStructParser = nil,
		m_pConstStructParser = nil,
	}

	getter "Source->m_strOrig"
	getter "Parsed->m_strSource"

	function Initialize( self, strSource, pStructParser, pConstStructParser )
		self.m_strOrig = strSource
		self.m_strSource = strSource
		self.m_pStructParser = pStructParser
		self.m_pConstStructParser = pConstStructParser
		self:GenerateCalls()
		self.m_strSource = self:ReplaceCalls()
	end

	function GenerateCalls( self )
		for type in self.m_strSource:gmatch( self.m_strPattern ) do
			local t = {}
			for impl, impl_type in pairs( self.m_pStructParser:GetStructImpls() ) do
				if impl_type == type then
					t[#t +1] = impl
				end
			end

			for impl, impl_type in pairs( self.m_pConstStructParser:GetStructImpls() ) do
				if impl_type.type == type then
					t[#t +1] = impl
				end
			end

			self.m_tblCalls[#self.m_tblCalls +1] = {
				type = type,
				output = table.concat( t, "," )
			}
		end
	end

	function ReplaceCalls( self )
		local parsed = self.m_strSource
		for _, call in pairs( self.m_tblCalls ) do
			while true do
				local str = ("IMPL_ALL_IMPLS_OF_STRUCT%s*%(%s*(_1_)%s*%)")
					:replace( "_1_", call.type )

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