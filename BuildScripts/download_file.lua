return function( tempLocation, extractLocation, fileName, url )
	local cwd = os.getcwd()
	os.mkdir( tempLocation )

	local result_str, response_code = http.download( url, tempLocation.. "/".. fileName, {
		progress = 	function( total, current )
			local ratio = current / total
			ratio = math.min( math.max(ratio, 0), 1 )
			local percent = math.floor( ratio * 100 )
			print( "Download progress (" .. percent .. "%/100%)" )
		end,
	})

	if result_str ~= "OK" then
		term.pushColor( term.errorColor )
			print( "Error downloading ".. url.. " - Response code ".. tostring(response_code) )
		term.popColor()
		return false
	end

	local ok = os.executef(
		"7z x \"%s\" -o\"%s\" skse64_2_00_17\\src\\* -r",
		cwd.. "/".. tempLocation.. "/".. fileName,
		cwd.. "/".. extractLocation
	)
	return ok
end