load(":d_common.bzl", "DLibraryInfo", "DToolchainInfo", "dflags_arg", "deps_arg", "linker_flags_arg", "srcs_arg")
load("@prelude//cxx:cxx_toolchain_types.bzl", "PicBehavior")
load(
    "@prelude//linking:link_info.bzl",
    "LibOutputStyle",
    "LinkInfo",
    "LinkInfos",
    "LinkStrategy",
    "MergedLinkInfo",
    "ObjectsLinkable",
    "create_merged_link_info",
    "get_link_args_for_strategy",
    "unpack_link_args",
)
load("@prelude//linking:types.bzl", "Linkage")
load("@prelude//utils:utils.bzl", "flatten")

def _d_compile(ctx, toolchain, srcs_map, dflags, extra_import_roots = []):
    srcdir = ctx.actions.symlinked_dir("d_src", srcs_map)
    objects = []

    for src_key in sorted(srcs_map.keys()):
        src = srcs_map[src_key]
        obj_name = (src_key[:-2] + ".o") if src_key.endswith(".d") else (src_key + ".o")
        out = ctx.actions.declare_output("obj/{}".format(obj_name))

        cmd = cmd_args(
            toolchain.compiler,
            toolchain.compiler_flags,
            dflags,
            "-c",
            cmd_args(out.as_output(), format = "-of={}"),
        )

        # The target's own import root first, then every transitive dep root
        cmd.add(cmd_args([srcdir] + list(extra_import_roots), format = "-I{}"))
        cmd.add(src)
        ctx.actions.run(
            cmd,
            category = "d_compile",
            identifier = "d-compile-{}".format(src_key.replace("/", "-").replace("\\", "-")),
        )
        objects.append(out)

    return objects, srcdir

def _d_library_impl(ctx):
    toolchain = ctx.attrs._d_toolchain[DToolchainInfo]
    d_deps = [dep[DLibraryInfo] for dep in ctx.attrs.deps if DLibraryInfo in dep]
    import_roots = flatten([d.import_roots for d in d_deps])

    objects, srcdir = _d_compile(
        ctx,
        toolchain,
        ctx.attrs.srcs,
        ctx.attrs.dflags,
        import_roots
    )

    linkable = ObjectsLinkable(objects = objects, linker_type = "windows", link_whole = False)
    link_info = LinkInfo(name = str(ctx.label), linkables = [linkable])

    return [
        DefaultInfo(default_outputs = objects),
        DLibraryInfo(objects = objects, import_roots = [srcdir] + import_roots),
        create_merged_link_info(
            ctx,
            PicBehavior("not_supported"),
            link_infos = {
                LibOutputStyle("archive"): LinkInfos(default = link_info),
                LibOutputStyle("pic_archive"): LinkInfos(default = link_info),
            },
            preferred_linkage = Linkage("static"),
            deps = [dep[MergedLinkInfo] for dep in ctx.attrs.deps if MergedLinkInfo in dep],
        ),
    ]

def _d_link(ctx, toolchain, objects, dep_objects, cxx_deps, linker_flags, exe_name):
    out = ctx.actions.declare_output(exe_name + ".exe")
    link_cmd = cmd_args(toolchain.compiler, toolchain.compiler_flags)
    link_cmd.add(objects)
    link_cmd.add(dep_objects) # Deps after our own objects

    if cxx_deps:
        link_args = get_link_args_for_strategy(ctx, cxx_deps, LinkStrategy("static"))
        link_cmd.add(unpack_link_args(link_args))
    
    # Flags last
    link_cmd.add(linker_flags)
    link_cmd.add(cmd_args(out.as_output(), format = "-of={}"))

    ctx.actions.run(
        link_cmd,
        category = "d_link",
        identifier = "d-link-{}".format(exe_name),
    )

    return out

def _wrap_link_flags(flags):
    out = []
    for f in flags:
        out.append(f if f.endswith(".lib") else ("-L" + f))
    return out

def _d_binary_impl(ctx):
    toolchain = ctx.attrs._d_toolchain[DToolchainInfo]
    d_deps = [dep[DLibraryInfo] for dep in ctx.attrs.deps if DLibraryInfo in dep]

    objects, _ = _d_compile(
        ctx,
        toolchain,
        ctx.attrs.srcs,
        ctx.attrs.dflags,
        flatten([d.import_roots for d in d_deps])
    )
    out = _d_link(
        ctx,
        toolchain,
        objects,
        flatten([d.objects for d in d_deps]),
        [dep[MergedLinkInfo] for dep in ctx.attrs.deps if MergedLinkInfo in dep],
        _wrap_link_flags(ctx.attrs.linker_flags),
        ctx.attrs.name
    )

    return [
        DefaultInfo(default_outputs = [out]),
        RunInfo(args = cmd_args(out)),
    ]

d_library = rule(
    impl = _d_library_impl,
    attrs = srcs_arg() | deps_arg() | dflags_arg() | {
        "_d_toolchain": attrs.default_only(
            attrs.toolchain_dep(default = "toolchains//:d", providers = [DToolchainInfo]),
        ),
    },
)

d_binary = rule(
    impl = _d_binary_impl,
    attrs = srcs_arg() | deps_arg() | dflags_arg() | linker_flags_arg() | {
        "_d_toolchain": attrs.default_only(
            attrs.toolchain_dep(default = "toolchains//:d", providers = [DToolchainInfo]),
        ),
    },
)
