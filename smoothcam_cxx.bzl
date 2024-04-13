load("//Deps/skylib/lib:dicts.bzl", "dicts")
load("//buck2/subdir_glob.bzl", "subdir_glob")

DEFINES = [
    "/DSKSE_SUPPORT_XBYAK=1",
    "/DSPDLOG_COMPILED_LIB",
    "/DSPDLOG_FMT_EXTERNAL",
    "/DFMT_USE_WINDOWS_H=0",
    "/DBOOST_STL_INTERFACES_DISABLE_CONCEPTS",
    "/DASMJIT_STATIC",
    "/DBUCK",
]

def define_polyhook():
    native.cxx_library(
        name = "asmtk",
        headers = subdir_glob([("Deps/PolyHook_2_0/asmtk/src", "**/*.h")]),
        exported_headers = subdir_glob([("Deps/PolyHook_2_0/asmtk/src", "**/*.h")]),
        srcs = native.glob(["Deps/PolyHook_2_0/asmtk/src/**/*.cpp"]),
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES,
        deps = [":asmjit"],
        force_static = True,
    )

    native.cxx_library(
        name = "asmjit",
        headers = subdir_glob([("Deps/PolyHook_2_0/asmjit/src", "**/*.h")]),
        exported_headers = subdir_glob([("Deps/PolyHook_2_0/asmjit/src", "**/*.h")]),
        srcs = native.glob(["Deps/PolyHook_2_0/asmjit/src/**/*.cpp"]),
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES + [
            "/wd2220",
            "/wd5054",

        ],
        force_static = True,
    )

    native.cxx_library(
        name = "zycore",
        headers = dicts.add(
            subdir_glob([
                ("Deps/PolyHook_2_0/zydis/dependencies/zycore/include", "**/*.h"),
            ]),
            {
                "ZycoreExportConfig.h": "DepGenerated/ZycoreExportConfig.h",
            }
        ),
        exported_headers = dicts.add(
            subdir_glob([
                ("Deps/PolyHook_2_0/zydis/dependencies/zycore/include", "**/*.h"),
            ]),
            {
                "ZycoreExportConfig.h": "DepGenerated/ZycoreExportConfig.h",
            }
        ),
        srcs = native.glob(["Deps/PolyHook_2_0/zydis/dependencies/zycore/src/**/*.c"]),
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES + [
            "/std:c11",
            "/TC",
        ],
        force_static = True,
    )

    native.cxx_library(
        name = "zydis",
        headers = dicts.add(
            subdir_glob([
                ("Deps/PolyHook_2_0/zydis/include", "**/*.h"),
                ("Deps/PolyHook_2_0/zydis/src", "**/*.inc"),
            ]),
            {
                "ZydisExportConfig.h": "DepGenerated/ZydisExportConfig.h",
            }
        ),
        exported_headers = dicts.add(
            subdir_glob([
                ("Deps/PolyHook_2_0/zydis/include", "**/*.h"),
                ("Deps/PolyHook_2_0/zydis/src", "**/*.inc"),
            ]),
            {
                "ZydisExportConfig.h": "DepGenerated/ZydisExportConfig.h",
            }
        ),
        srcs = native.glob(["Deps/PolyHook_2_0/zydis/src/**/*.c"]),
        deps = [":zycore"],
        exported_deps = [":zycore"],
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES + [
            "/std:c11",
            "/TC",
        ],
        force_static = True,
    )

    native.cxx_library(
        name = "Polyhook2",
        headers = subdir_glob(
            [("Deps/PolyHook_2_0/polyhook2", "**/*.hpp")],
            prefix = "polyhook2"
        ),
        exported_headers = subdir_glob(
            [("Deps/PolyHook_2_0/polyhook2", "**/*.hpp")],
            prefix = "polyhook2"
        ),
        srcs = native.glob(["Deps/PolyHook_2_0/sources/**/*.cpp"]),
        deps = [":zydis", ":asmjit", ":asmtk"],
        exported_deps = [":zydis", ":asmjit"],
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES + [
            "/wd4717",
        ],
        force_static = True,
    )

def define_commonlib_deps():
    native.cxx_library(
        name = "fmt",
        headers = {
            "fmt/format-inl.h": "Deps/fmt/include/fmt/format-inl.h",
            "fmt/format.h": "Deps/fmt/include/fmt/format.h",
            "fmt/core.h": "Deps/fmt/include/fmt/core.h",
            "fmt/os.h": "Deps/fmt/include/fmt/os.h",
        },
        exported_headers = {
            "fmt/format-inl.h": "Deps/fmt/include/fmt/format-inl.h",
            "fmt/format.h": "Deps/fmt/include/fmt/format.h",
            "fmt/core.h": "Deps/fmt/include/fmt/core.h",
            "fmt/os.h": "Deps/fmt/include/fmt/os.h",
        },
        srcs = ["Deps/fmt/src/format.cc", "Deps/fmt/src/os.cc"],
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES + [
            "/D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING"
        ],
        force_static = True,
    )

    native.cxx_library(
        name = "stl_interfaces",
        headers = subdir_glob([("Deps/stl_interfaces/include", "**/*.hpp")]),
        exported_headers = subdir_glob([("Deps/stl_interfaces/include", "**/*.hpp")]),
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES,
        force_static = True,
    )

    native.cxx_library(
        name = "spdlog",
        headers = subdir_glob([("Deps/spdlog/include", "spdlog/**/*.h")]),
        exported_headers = subdir_glob([("Deps/spdlog/include", "spdlog/**/*.h")]),
        srcs = [
            "Deps/spdlog/src/spdlog.cpp",
            "Deps/spdlog/src/stdout_sinks.cpp",
            "Deps/spdlog/src/color_sinks.cpp",
            "Deps/spdlog/src/file_sinks.cpp",
            "Deps/spdlog/src/async.cpp",
            "Deps/spdlog/src/cfg.cpp",
            "Deps/spdlog/src/fmt.cpp",
        ],
        deps = [":fmt"],
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES + [
            "/D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING"
        ],
        force_static = True,
    )

    native.cxx_library(
        name = "binary_io",
        headers = subdir_glob(
            [("Deps/binary_io/include/binary_io", "**/*.hpp")],
            prefix = "binary_io",
        ),
        exported_headers = subdir_glob(
            [("Deps/binary_io/include/binary_io", "**/*.hpp")],
            prefix = "binary_io",
        ),
        srcs = ["Deps/binary_io/src/binary_io/binary_io.cpp"],
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES,
        force_static = True,
    )

    native.cxx_library(
        name = "xbyak",
        headers = subdir_glob([("Deps/xbyak", "xbyak/**/*.h")]),
        exported_headers = subdir_glob([("Deps/xbyak", "xbyak/**/*.h")]),
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES,
        force_static = True,
    )

def make_commonlib_target(target, extra_compiler_opts, visibility, pre629Variant=False):
    native.cxx_library(
        name = "CommonLib" + target,
        headers = subdir_glob([
            ("Deps/CommonLibSSEPre629/include", "RE/**/*.h"),
            ("Deps/CommonLibSSEPre629/include", "REL/**/*.h"),
            ("Deps/CommonLibSSEPre629/include", "SKSE/**/*.h"),
        ]) if pre629Variant else subdir_glob([
            ("Deps/CommonLibSSE/include", "RE/**/*.h"),
            ("Deps/CommonLibSSE/include", "REL/**/*.h"),
            ("Deps/CommonLibSSE/include", "SKSE/**/*.h"),
        ]),
        exported_headers = subdir_glob([
            ("Deps/CommonLibSSEPre629/include", "RE/**/*.h"),
            ("Deps/CommonLibSSEPre629/include", "REL/**/*.h"),
            ("Deps/CommonLibSSEPre629/include", "SKSE/**/*.h"),
        ]) if pre629Variant else subdir_glob([
            ("Deps/CommonLibSSE/include", "RE/**/*.h"),
            ("Deps/CommonLibSSE/include", "REL/**/*.h"),
            ("Deps/CommonLibSSE/include", "SKSE/**/*.h"),
        ]),
        srcs = native.glob(["Deps/CommonLibSSEPre629/src/**/*.cpp"]) if pre629Variant else native.glob(["Deps/CommonLibSSE/src/**/*.cpp"]),
        exported_deps = [
            ":fmt",
            ":spdlog",
            ":binary_io",
            ":xbyak",
            ":stl_interfaces",
        ],
        visibility = [visibility],
        compiler_flags = DEFINES + [
            "/D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",

            "/DWINVER=0x0601",
            "/D_WIN32_WINNT=0x0601",
            "/FISKSE/Impl/PCH.h",

            # warnings -> errors
            "/we4715",    # 'function' : not all control paths return a value

            # disable warnings
            "/wd4996", # std::aligned_storage and std::aligned_storage_t are deprecated in C++23
            "/wd4005", # macro redefinition
            "/wd4061", # enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
            "/wd4200", # nonstandard extension used : zero-sized array in struct/union
            "/wd4201", # nonstandard extension used : nameless struct/union
            "/wd4265", # 'type': class has virtual functions, but its non-trivial destructor is not virtual; instances of this class may not be destructed correctly
            "/wd4266", # 'function' : no override available for virtual member function from base 'type'; function is hidden
            "/wd4371", # 'classname': layout of class may have changed from a previous version of the compiler due to better packing of member 'member'
            "/wd4514", # 'function' : unreferenced inline function has been removed
            "/wd4582", # 'type': constructor is not implicitly called
            "/wd4583", # 'type': destructor is not implicitly called
            "/wd4623", # 'derived class' : default constructor was implicitly defined as deleted because a base class default constructor is inaccessible or deleted
            "/wd4625", # 'derived class' : copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted
            "/wd4626", # 'derived class' : assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted
            "/wd4710", # 'function' : function not inlined
            "/wd4711", # function 'function' selected for inline expansion
            "/wd4820", # 'bytes' bytes padding added after construct 'member_name'
            "/wd5026", # 'type': move constructor was implicitly defined as deleted
            "/wd5027", # 'type': move assignment operator was implicitly defined as deleted
            "/wd5045", # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
            "/wd5053", # support for 'explicit(<expr>)' in C++17 and earlier is a vendor extension
            "/wd5204", # 'type-name': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
            "/wd5220", # 'member': a non-static data member with a volatile qualified type no longer implies that compiler generated copy / move constructors and copy / move assignment operators are not trivial
        ] + extra_compiler_opts,
        force_static = True,
    )

def define_targets(targets):
    pre_629_mode = read_root_config("build", "pre629", "disabled") == "enabled"

    define_polyhook()
    define_commonlib_deps()

    native.cxx_library(
        name = "eternal",
        headers = subdir_glob([("Deps/eternal/include", "mapbox/**/*.hpp")]),
        exported_headers = subdir_glob([("Deps/eternal/include", "mapbox/**/*.hpp")]),
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES,
        force_static = True,
    )

    native.cxx_library(
        name = "EABase",
        headers = subdir_glob([("Deps/EABase/include/Common", "**/*.h")]),
        exported_headers = subdir_glob([("Deps/EABase/include/Common", "**/*.h")]),
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES,
        force_static = True,
    )

    native.cxx_library(
        name = "EASTL",
        headers = subdir_glob([("Deps/EASTL/include", "**/*.h")]),
        exported_headers = subdir_glob([("Deps/EASTL/include", "**/*.h")]),
        deps = [":EABase"],
        exported_deps = [":EABase"],
        linker_flags = [
            "/D_CHAR16T",
            "/D_CRT_SECURE_NO_WARNINGS",
            "/D_SCL_SECURE_NO_WARNINGS",
            "/DEASTL_OPENSOURCE=1"
        ],
        srcs = native.glob(["Deps/EASTL/source/**/*.cpp"]),
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES,
        force_static = True,
    )

    native.cxx_library(
        name = "glm",
        include_directories = ["Deps/glm", "Deps/glm/glm"],
        public_include_directories = ["Deps/glm", "Deps/glm/glm"],
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES,
        force_static = True,
    )

    native.cxx_library(
        name = "detours",
        headers = {
            "detours.h": "Deps/Detours/src/detours.h",
            "detver.h": "Deps/Detours/src/detver.h",
            "syelog.h": "Deps/Detours/samples/syelog/syelog.h",
            "disasm.cpp": "Deps/Detours/src/disasm.cpp",
            "uimports.cpp": "Deps/Detours/src/uimports.cpp",
        },
        exported_headers = {
            "detours/detours.h": "Deps/Detours/src/detours.h",
            "detours/detver.h": "Deps/Detours/src/detver.h",
            "detours/syelog.h": "Deps/Detours/samples/syelog/syelog.h",
        },
        srcs = [
            "Deps/Detours/src/creatwth.cpp",
            "Deps/Detours/src/detours.cpp",
            "Deps/Detours/src/disasm.cpp",
            "Deps/Detours/src/disolarm.cpp",
            "Deps/Detours/src/disolarm64.cpp",
            "Deps/Detours/src/disolia64.cpp",
            "Deps/Detours/src/disolx64.cpp",
            "Deps/Detours/src/disolx86.cpp",
            "Deps/Detours/src/image.cpp",
            "Deps/Detours/src/modules.cpp",
        ],
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES + [
            "/DDETOURS_VERSION=0x4c0c1",
            "/DDETOURS_64BIT",
            "/we4777",
            "/we4800",
            "/DWIN32_LEAN_AND_MEAN",
            "/D_WIN32_WINNT=0x501",
            "/wd4163",
        ],
        force_static = True,
    )

    native.cxx_library(
        name = "json",
        headers = subdir_glob([("Deps/json/include", "**/*.hpp")]),
        exported_headers = subdir_glob([("Deps/json/include", "**/*.hpp")]),
        visibility = ["PUBLIC"],
        compiler_flags = DEFINES,
        force_static = True,
    )

    for target in targets:
        if pre_629_mode and target == "SSE":
            continue
        
        make_commonlib_target(
            target["name"],
            (["/DSKYRIM_IS_PRE629"] if pre_629_mode else []) + target["extra_compiler_opts"],
            "//:SmoothCam" + target["name"] + "Module",
            pre_629_mode
        )

        native.cxx_library(
            name = "SmoothCam" + target["name"],
            srcs = [
                "SmoothCam/source/camera_states/thirdperson/dialogue/face_to_face.cpp",
                "SmoothCam/source/camera_states/thirdperson/dialogue/oblivion.cpp",
                "SmoothCam/source/camera_states/thirdperson/dialogue/skyrim.cpp",
                "SmoothCam/source/camera_states/thirdperson/thirdperson.cpp",
                "SmoothCam/source/camera_states/thirdperson/thirdperson_dialogue.cpp",
                "SmoothCam/source/camera_states/thirdperson/thirdperson_vanity.cpp",
                "SmoothCam/source/camera_states/base_first.cpp",
                "SmoothCam/source/camera_states/base_third.cpp",
                "SmoothCam/source/crosshair/base.cpp",
                "SmoothCam/source/crosshair/dot.cpp",
                "SmoothCam/source/crosshair/skyrim.cpp",
                "SmoothCam/source/debug/commands/dump_game_ini.cpp",
                "SmoothCam/source/debug/commands/dump_game_perfs_ini.cpp",
                "SmoothCam/source/debug/commands/get_setting.cpp",
                "SmoothCam/source/debug/commands/help.cpp",
                "SmoothCam/source/debug/commands/set_setting.cpp",
                "SmoothCam/source/debug/console.cpp",
                "SmoothCam/source/debug/eh.cpp",
                "SmoothCam/source/debug/ICommand.cpp",
                "SmoothCam/source/debug/registry.cpp",
                "SmoothCam/source/render/cbuffer.cpp",
                "SmoothCam/source/render/common.cpp",
                "SmoothCam/source/render/d2d.cpp",
                "SmoothCam/source/render/d3d_context.cpp",
                "SmoothCam/source/render/dwrite.cpp",
                "SmoothCam/source/render/gradbox.cpp",
                "SmoothCam/source/render/line_drawer.cpp",
                "SmoothCam/source/render/line_graph.cpp",
                "SmoothCam/source/render/mesh_drawer.cpp",
                "SmoothCam/source/render/model.cpp",
                "SmoothCam/source/render/ninode_tree_display.cpp",
                "SmoothCam/source/render/render_target.cpp",
                "SmoothCam/source/render/shader_cache.cpp",
                "SmoothCam/source/render/shader.cpp",
                "SmoothCam/source/render/srv.cpp",
                "SmoothCam/source/render/state_overlay.cpp",
                "SmoothCam/source/render/texture2d.cpp",
                "SmoothCam/source/render/vertex_buffer.cpp",
                "SmoothCam/source/trackir/trackir.cpp",
                "SmoothCam/source/arrow_fixes.cpp",
                "SmoothCam/source/camera.cpp",
                "SmoothCam/source/compat.cpp",
                "SmoothCam/source/config.cpp",
                "SmoothCam/source/crosshair.cpp",
                "SmoothCam/source/hooks.cpp",
                "SmoothCam/source/firstperson.cpp",
                "SmoothCam/source/game_state.cpp",
                "SmoothCam/source/main.cpp",
                "SmoothCam/source/mmath.cpp",
                "SmoothCam/source/modapi.cpp",
                "SmoothCam/source/papyrus.cpp",
                "SmoothCam/source/pch.cpp",
                "SmoothCam/source/raycast.cpp",
                "SmoothCam/source/thirdperson.cpp",
                "SmoothCam/source/timer.cpp",
                "SmoothCam/source/offset_ids.cpp",
                "SmoothCam/source/util.cpp",
            ],
            headers = subdir_glob([("SmoothCam/include", "**/*.h")]),
            compiler_flags = DEFINES + (["/DSKYRIM_IS_PRE629"] if pre_629_mode else []) + [
                "/D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
                "/FIpch.h",
            ] + target["extra_compiler_opts"],
            link_style = "shared",
            soname = "SmoothCam" + target["name"] + ("Pre629" if pre_629_mode else "") + ".dll",
            deps = [
                ":EASTL",
                ":Polyhook2",
                ":eternal",
                ":glm",
                ":detours",
                ":json",
                ":CommonLib" + target["name"],
            ],
        )
