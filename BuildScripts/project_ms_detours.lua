return function( loc, platform )
	if platform == "vs2019" or platform == "vs2017" then
		os.execute( ("call \"%s\" \"%s\" \"%s\""):format(
			"BuildScripts/build_detours.bat",
			loc,
			platform
		) )
	else
		return error( "Failed building Microsoft Detours - unknown visual studio platform ".. platform.. "!" )
	end
end