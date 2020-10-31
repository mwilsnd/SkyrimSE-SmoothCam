do class "ImplOffsetGroupPage" : namespace "papyrus.preproc" {
		m_tblCalls = {},
		m_tblVarIDs = {},

		m_strOrig = "",
		m_strSource = "",

		m_strPattern = "(%s*)IMPL_OFFSET_GROUP_PAGE%s*%(%s*([%a%d_,%s]+)%s*%)",
		m_pStructParser = nil,
		m_pConstStructParser = nil,
	}

	getter "Source->m_strOrig"
	getter "Parsed->m_strSource"

	function Initialize( self, strSource, pOffsetGroupGenerator )
		self.m_strOrig = strSource
		self.m_strSource = strSource
		self.m_tblVarIDs = pOffsetGroupGenerator:GetVarIDs()
		self:GenerateCalls()
		self.m_strSource = self:ReplaceCalls()
	end

	function GenerateCalls( self )
		for indent, type in self.m_strSource:gmatch( self.m_strPattern ) do
			indent = indent:gsub("\n", "")
			local firstDecl
			local params = {}
			for decl in type:gmatch("[%a%d_]+") do
				if not firstDecl then
					firstDecl = decl
				end

				params[decl] = true
			end

			self.m_tblCalls[#self.m_tblCalls +1] = {
				type = type,
				indent = indent,
				output = self:GeneratePage(firstDecl, params, indent)
			}
		end
	end

	function GeneratePage( self, strName, tblLevels, strIndent )
		local indent = ""

		local out = {}
		if not tblLevels["NoSliderHeader"] then
			table.insert(out, ("AddHeaderOption(\"%s Offsets\")"):format(strName))
			indent = strIndent
		end

		table.insert(out, indent.. "IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {")
			local decls = {}
			for _, name in ipairs(self.m_tblVarIDs.Offsets[strName] or {}) do
				table.insert(decls, name)
			end
			table.insert(out, table.concat(decls, ","))

			indent = strIndent
		table.insert(out, "})")

		if not tblLevels["NoInterpToggles"] then
			table.insert(out, indent.."SetCursorPosition(1)")
			indent = strIndent
			if not tblLevels["NoInterpHeader"] then
				table.insert(out, indent.. "AddHeaderOption(\"Interpolation\")")
			end

			table.insert(out, indent.. "IMPL_STRUCT_MACRO_INVOKE_GROUP(implControl, {")
				decls = {}
				for _, name in ipairs(self.m_tblVarIDs.Interp[strName] or {}) do
					table.insert(decls, indent.. name)
				end
				table.insert(out, table.concat(decls, ","))
			table.insert(out, indent.. "})")
		end

		return table.concat(out, "\n")
	end

	-- We need to group invocations based on the struct page member
	-- Then create an if block to compare pageName agains each unique page member found
	-- THEN invoke macros under that page name

	function ReplaceCalls( self )
		local parsed = self.m_strSource
		for _, call in pairs( self.m_tblCalls ) do
			while true do
				local str = ("IMPL_OFFSET_GROUP_PAGE%s*%(%s*(_1_)%s*%)")
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