do class "GenerateOffsetGroup" : namespace "papyrus.preproc" {
		m_tblCalls = {},
		m_tblVarIDs = { Offsets = {}, Interp = {} },

		m_strOrig = "",
		m_strSource = "",

		m_strPattern = "DECLARE_OFFSET_GROUP_CONTROLS%s*%(%s*([%a%d_]+)%s*,%s*([%a%d\"%s_]+)%s*,?%s*([%a%d_,%s]*)%s*%)",
		m_pStructParser = nil,
		m_pConstStructParser = nil,
	}

	getter "Source->m_strOrig"
	getter "Parsed->m_strSource"
	getter "VarIDs->m_tblVarIDs"

	function Initialize( self, strSource )
		self.m_strOrig = strSource
		self.m_strSource = strSource
		self:GenerateCalls()
		self.m_strSource = self:ReplaceCalls()
	end

	function GenerateCalls( self )
		for type, matchingPage, flags in self.m_strSource:gmatch( self.m_strPattern ) do
			local flagList = flags:gsub("\n", ""):gsub("\r", ""):gsub("\t", ""):split(",")
			local flagMap = {}
			for k, f in pairs(flagList) do
				flagMap[f:trim()] = true
			end

			self.m_tblCalls[#self.m_tblCalls +1] = {
				type = type,
				page = matchingPage,
				flags = flags,
				output = self:GenerateOffsetGroupString(type, matchingPage, flagMap)
			}
		end
	end

	function CreateSliderSetting( self, strName, strGroup, strSubGroup, strDisplayName, strDesc, nDefault,
		nMin, nMax, strPage )

		local fmt = [[SliderSetting %s -> {
	settingName: "%s"
	displayName: "%s"
	desc: "%s"
	defaultValue: %f
	min: %f
	max: %f
	page: %s
}]]

		local var = (strSubGroup and strSubGroup:gsub(":", "")) or ""
		local varName = strGroup:lower().. "_".. strName.. var
		self.m_tblVarIDs.Offsets[strGroup] = self.m_tblVarIDs.Offsets[strGroup] or {}
		table.insert(self.m_tblVarIDs.Offsets[strGroup], varName)
		return fmt:format(
			varName,
			strSubGroup and (strGroup.. strSubGroup.. ":".. strName) or (strGroup.. ":".. strName),
			strDisplayName,
			strDesc,
			nDefault,
			nMin,
			nMax,
			strPage
		)
	end

	function CreateToggleSetting( self, strName, strGroup, strSubGroup, strDisplayName, strDesc, strPage )
		local fmt = [[ToggleSetting %s -> {
	settingName: "%s"
	displayName: "%s"
	desc: "%s"
	page: %s
}]]
		local varName = strGroup:lower().. "_".. (strSubGroup or "").. strName
		self.m_tblVarIDs.Interp[strGroup] = self.m_tblVarIDs.Interp[strGroup] or {}
		table.insert(self.m_tblVarIDs.Interp[strGroup], varName)
		return fmt:format(
			varName,
			strName.. strGroup.. (strSubGroup or ""),
			strDisplayName,
			strDesc,
			strPage
		)
	end

	function GenerateOffsetGroupString( self, strGroupName, strPage, tblFlags )
		local out = {}
		-- Normal
		table.insert(out, self:CreateSliderSetting(
			"SideOffset",
			strGroupName,
			nil,
			"Side Offset",
			"The amount to move the camera to the right.",
			25,
			-100,
			100,
			strPage
		))
		table.insert(out, self:CreateSliderSetting(
			"UpOffset",
			strGroupName,
			nil,
			"Up Offset",
			"The amount to move the camera up.",
			0,
			-100,
			100,
			strPage
		))
		table.insert(out, self:CreateSliderSetting(
			"ZoomOffset",
			strGroupName,
			nil,
			"Zoom Offset",
			"The amount to offset the camera zoom by.",
			0,
			-200,
			200,
			strPage
		))
		table.insert(out, self:CreateSliderSetting(
			"FOVOffset",
			strGroupName,
			nil,
			"FOV Offset",
			"The amount to offset the camera FOV by."..
			" Note this will be clamped to a lower bound of 10 and an upper bound of 170.",
			0,
			-120,
			120,
			strPage
		))

		-- Ranged combat
		if not tblFlags.NoRanged then
			table.insert(out, self:CreateSliderSetting(
				"SideOffset",
				strGroupName,
				"Combat:Ranged",
				"Ranged Combat Side Offset",
				"The amount to move the camera to the right when in ranged combat.",
				25,
				-100,
				100,
				strPage
			))
			table.insert(out, self:CreateSliderSetting(
				"UpOffset",
				strGroupName,
				"Combat:Ranged",
				"Ranged Combat Up Offset",
				"The amount to move the camera up when in ranged combat.",
				0,
				-100,
				100,
				strPage
			))
			table.insert(out, self:CreateSliderSetting(
				"ZoomOffset",
				strGroupName,
				"Combat:Ranged",
				"Ranged Combat Zoom Offset",
				"The amount to offset the camera zoom by when in ranged combat.",
				0,
				-200,
				200,
				strPage
			))
			table.insert(out, self:CreateSliderSetting(
				"FOVOffset",
				strGroupName,
				"Combat:Ranged",
				"Ranged Combat FOV Offset",
				"The amount to offset the camera FOV by when in ranged combat."..
				" Note this will be clamped to a lower bound of 10 and an upper bound of 170.",
				0,
				-120,
				120,
				strPage
			))
		end

		-- Magic combat
		if not tblFlags.NoMagic then
			table.insert(out, self:CreateSliderSetting(
				"SideOffset",
				strGroupName,
				"Combat:Magic",
				"Magic Combat Side Offset",
				"The amount to move the camera to the right when in magic combat.",
				25,
				-100,
				100,
				strPage
			))
			table.insert(out, self:CreateSliderSetting(
				"UpOffset",
				strGroupName,
				"Combat:Magic",
				"Magic Combat Up Offset",
				"The amount to move the camera up when in magic combat.",
				0,
				-100,
				100,
				strPage
			))
			table.insert(out, self:CreateSliderSetting(
				"ZoomOffset",
				strGroupName,
				"Combat:Magic",
				"Magic Combat Zoom Offset",
				"The amount to offset the camera zoom by when in magic combat.",
				0,
				-200,
				200,
				strPage
			))
			table.insert(out, self:CreateSliderSetting(
				"FOVOffset",
				strGroupName,
				"Combat:Magic",
				"Magic Combat FOV Offset",
				"The amount to offset the camera FOV by when in magic combat."..
				" Note this will be clamped to a lower bound of 10 and an upper bound of 170.",
				0,
				-120,
				120,
				strPage
			))
		end

		-- Melee combat
		if not tblFlags.NoMelee then
			table.insert(out, self:CreateSliderSetting(
				"SideOffset",
				strGroupName,
				"Combat:Melee",
				"Melee Combat Side Offset",
				"The amount to move the camera to the right when in melee combat.",
				25,
				-100,
				100,
				strPage
			))
			table.insert(out, self:CreateSliderSetting(
				"UpOffset",
				strGroupName,
				"Combat:Melee",
				"Melee Combat Up Offset",
				"The amount to move the camera up when in melee combat.",
				0,
				-100,
				100,
				strPage
			))
			table.insert(out, self:CreateSliderSetting(
				"ZoomOffset",
				strGroupName,
				"Combat:Melee",
				"Melee Combat Zoom Offset",
				"The amount to offset the camera zoom by when in melee combat.",
				0,
				-200,
				200,
				strPage
			))
			table.insert(out, self:CreateSliderSetting(
				"FOVOffset",
				strGroupName,
				"Combat:Melee",
				"Melee Combat FOV Offset",
				"The amount to offset the camera FOV by when in melee combat."..
				" Note this will be clamped to a lower bound of 10 and an upper bound of 170.",
				0,
				-120,
				120,
				strPage
			))
		end

		-- Toggles
		if not tblFlags.NoInterpToggles then
			table.insert(out, self:CreateToggleSetting(
				"Interp",
				strGroupName,
				nil,
				"Enable Interpolation",
				"Enables interpolation in this state.",
				strPage
			))
			if not tblFlags.NoRanged then
				table.insert(out, self:CreateToggleSetting(
					"Interp",
					strGroupName,
					"RangedCombat",
					"Enable Ranged Interpolation",
					"Enables interpolation in this state.",
					strPage
				))
			end
			if not tblFlags.NoMagic then
				table.insert(out, self:CreateToggleSetting(
					"Interp",
					strGroupName,
					"MagicCombat",
					"Enable Magic Interpolation",
					"Enables interpolation in this state.",
					strPage
				))
			end
			if not tblFlags.NoMelee then
				table.insert(out, self:CreateToggleSetting(
					"Interp",
					strGroupName,
					"MeleeCombat",
					"Enable Melee Interpolation",
					"Enables interpolation in this state.",
					strPage
				))
			end
		end
		return table.concat(out, "\n")
	end

	function ReplaceCalls( self )
		local parsed = self.m_strSource
		for _, call in pairs( self.m_tblCalls ) do
			while true do
				local str = ("DECLARE_OFFSET_GROUP_CONTROLS%s*%(%s*(_1_)%s*,%s*(_2_)%s*,?%s*(_3_)%s*%)")
					:replace( "_1_", call.type )
					:replace( "_2_", call.page )
					:replace( "_3_", call.flags )

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