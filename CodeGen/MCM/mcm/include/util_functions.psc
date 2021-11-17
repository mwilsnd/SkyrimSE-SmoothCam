int Function GetCurrentArrayIndex(string setting, string[] array)
	string value = SmoothCam_GetStringConfig(setting)
	
	Int i = array.Length
	While i
		i -= 1
		if (array[i] == value)
			return i
		endIf
	endWhile

	return 0
endFunction

int Function GetCurrentArrayIndexLocal(string value, string[] array)
	Int i = array.Length
	While i
		i -= 1
		if (array[i] == value)
			return i
		endIf
	endWhile

	return 0
endFunction