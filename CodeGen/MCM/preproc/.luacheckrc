std = {
	read_globals = {
		"abstract", "class", "namespace", "extends", "implements", "from", "interface", "singleton",
		"mixin", "accessor", "getter", "setter", "super", "this", "shared", "shared_block",

		"coroutine", "assert", "tostring", "tonumber", "rawget", "xpcall", "ipairs", "print",
		"pcall", "gcinfo", "pairs", "package", "error", "debug", "loadfile", "rawequal",
		"loadstring", "rawset", "unpack", "table", "require", "_VERSION",
		"newproxy", "collectgarbage", "dofile", "next", "load", "_G", "select",
		"type", "getmetatable", "setmetatable",

		"io",
		"bit",
		"math",
		"os",
		"string",
		"jit",

		"arg",
		"getfenv",
		"setfenv",
		"module"
	}
}

allow_defined = true
max_cyclomatic_complexity = 15

ignore = {"212/self", "131", "113"}