load(":d_common.bzl", "DToolchainInfo")

def _d_toolchain_impl(ctx):
    return [
        DefaultInfo(),
        DToolchainInfo(
            compiler = cmd_args(ctx.attrs.compiler),
            compiler_flags = ctx.attrs.compiler_flags,
        ),
    ]

_d_toolchain = rule(
    impl = _d_toolchain_impl,
    attrs = {
        "compiler": attrs.string(doc = "Compiler binary"),
        "compiler_flags": attrs.list(attrs.string(), default = [], doc = "Base compiler flags"),
    },
    is_toolchain_rule = True,
)

def d_toolchain(name, compiler, compiler_flags = [], **kwargs):
    mode = read_root_config("build", "mode", "debug")
    flags = compiler_flags
    if mode == "release":
        flags = flags + ["-O3"]
        
    _d_toolchain(
        name = name,
        compiler = compiler,
        compiler_flags = flags,
        **kwargs
    )
