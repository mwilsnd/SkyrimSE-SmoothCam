DToolchainInfo = provider(fields = {
    "compiler": provider_field(cmd_args),
    "compiler_flags": provider_field(list),
})

DLibraryInfo = provider(fields = {
    "objects": provider_field(list),
    "import_roots": provider_field(list),
})

def srcs_arg():
    return {
        "srcs": attrs.named_set(
            attrs.source(),
            sorted = True,
            default = [],
            doc = "D source files. Keys are module/import paths, values are repo-relative."
        ),
    }

def deps_arg():
    return {
        "deps": attrs.list(
            attrs.dep(),
            default = [],
            doc = "D and/or C++ static-lib deps."
        ),
    }

def dflags_arg():
    return {
        "dflags": attrs.list(
            attrs.string(),
            default = [],
            doc = "Extra compile flags."
        ),
    }

def linker_flags_arg():
    return {
        "linker_flags": attrs.list(
            attrs.string(),
            default = [],
            doc = "Extra linker flags."    
        ),
    }
