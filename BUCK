load("//:smoothcam_cxx.bzl", "define_targets")
load("//:codegen.bzl", "define_tools")

define_tools()
define_targets([
    {
        "name": "SSE",
        "extra_compiler_opts": [],
        "uses_pre_629_structs": False,
    },
    {
        "name": "AEPre629",
        "extra_compiler_opts": ["/DSKYRIM_SUPPORT_AE=1", "/DSKYRIM_IS_PRE629"],
        "uses_pre_629_structs": True,
    },
    {
        "name": "AE",
        "extra_compiler_opts": ["/DSKYRIM_SUPPORT_AE=1"],
        "uses_pre_629_structs": False,
    }
])