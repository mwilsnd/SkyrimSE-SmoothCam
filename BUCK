load("//:smoothcam_cxx.bzl", "define_targets")
#load("//:codegen.bzl", "define_tools")

# buck2 doesn't support D out of the box (need to build a toolchain), build the tools manually for now.
# define_tools()

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
