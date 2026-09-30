load("//:smoothcam_cxx.bzl", "define_targets")
load("//:codegen.bzl", "define_tools")

define_tools()

define_targets([
    {
        "name": "SSE",
        "extra_compiler_opts": [],
    },
    {
        "name": "AE",
        "extra_compiler_opts": ["/DSKYRIM_SUPPORT_AE=1"],
    }
])
