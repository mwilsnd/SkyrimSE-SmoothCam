load("//Deps/skylib/lib:dicts.bzl", "dicts")
load("//buck2/subdir_glob.bzl", "subdir_glob")
load("@prelude//decls:d_rules.bzl", "d_rules")

def define_tools():
    native.cxx_library(
        name = "pugixml",
        srcs = ["Deps/assimp/contrib/pugixml/src/pugixml.cpp"],
        headers = {
            "pugiconfig.hpp": "Deps/assimp/contrib/pugixml/src/pugiconfig.hpp",
            "pugixml.hpp": "Deps/assimp/contrib/pugixml/src/pugixml.hpp",
        },
        exported_headers = {
            "pugiconfig.hpp": "Deps/assimp/contrib/pugixml/src/pugiconfig.hpp",
            "pugixml.hpp": "Deps/assimp/contrib/pugixml/src/pugixml.hpp",
            "pugixml.cpp": "Deps/assimp/contrib/pugixml/src/pugixml.cpp",
        },
        link_style = "static",
        link_whole = False,
        visibility = ["PUBLIC"],
    )

    native.cxx_library(
        name = "stb_image",
        exported_headers = {
            "stb/stb_image.h": "Deps/assimp/contrib/stb/stb_image.h",
        },
        link_style = "static",
        link_whole = False,
        visibility = ["PUBLIC"],
    )

    native.cxx_library(
        name = "utf8cpp",
        exported_headers = {
            "utf8.h": "Deps/assimp/contrib/utf8cpp/source/utf8.h",
            "utf8/checked.h": "Deps/assimp/contrib/utf8cpp/source/utf8/checked.h",
            "utf8/core.h": "Deps/assimp/contrib/utf8cpp/source/utf8/core.h",
            "utf8/cpp11.h": "Deps/assimp/contrib/utf8cpp/source/utf8/cpp11.h",
            "utf8/unchecked.h": "Deps/assimp/contrib/utf8cpp/source/utf8/unchecked.h",
        },
        link_style = "static",
        link_whole = False,
        visibility = ["PUBLIC"],
    )

    native.cxx_library(
        name = "zlib",
        srcs = native.glob(["Deps/assimp/contrib/zlib/*.c"]),
        headers = dicts.add(
            subdir_glob([("Deps/assimp/contrib/zlib/", "*.h")]),
            {
                "zconf.h": "DepGenerated/zconf.h",
            }
        ),
        exported_headers = {
            "zconf.h": "DepGenerated/zconf.h",
            "zlib.h": "Deps/assimp/contrib/zlib/zlib.h",
        },
        compiler_flags = [
            "/D_CRT_SECURE_NO_DEPRECATE",
            "/D_CRT_NONSTDC_NO_DEPRECATE",
            "/DNO_FSEEKO",
            "/DNDEBUG",
            "/wd4131",
            "/wd4127",
            "/wd4244",
        ],
        link_style = "static",
        link_whole = False,
        visibility = ["PUBLIC"],
    )

    native.cxx_library(
        name = "unzip",
        srcs = native.glob(["Deps/assimp/contrib/unzip/*.c"]),
        headers = subdir_glob([("Deps/assimp/contrib/unzip", "*.h")]),
        exported_headers = {
            "crypt.h": "Deps/assimp/contrib/unzip/crypt.h",
            "ioapi.h": "Deps/assimp/contrib/unzip/ioapi.h",
            "unzip.h": "Deps/assimp/contrib/unzip/unzip.h",
        },
        compiler_flags = [
            "/D_CRT_SECURE_NO_DEPRECATE",
            "/D_CRT_NONSTDC_NO_DEPRECATE",
            "/DNO_FSEEKO",
            "/DNDEBUG",
            "/wd4131",
            "/wd4127",
            "/wd4244",
        ],
        link_style = "static",
        link_whole = False,
        visibility = ["PUBLIC"],
        deps = [":zlib"],
    )

    # HACK: broken relative includes
    AIpatchedSources = native.glob([
        "Deps/assimp/code/CApi/**/*.cpp",
        "Deps/assimp/code/Common/**/*.cpp",
        "Deps/assimp/code/Material/**/*.cpp",
        "Deps/assimp/code/Pbrt/**/*.cpp",
        "Deps/assimp/code/PostProcessing/**/*.cpp",
        "Deps/assimp/code/AssetLib/Ply/**/*.cpp",
    ])
    AIpatchedSources.remove("Deps/assimp/code/Common/BaseImporter.cpp")
    AIpatchedSources.append(("Deps/assimp/code/Common/BaseImporter.cpp", ["/DASSIMP_USE_HUNTER"]))
    AIdontCare = [
        "XGL", "X3D", "X", "3D", "TERRAGEN", "STL", "SMD", "SIB", "RAW", "Q3D", "Q3BSP",
        "OPENGEX", "OGRE", "OFF", "OBJ", "NFF", "NDO", "MS3D", "MMD", "MDL", "MDC", "MD5",
        "MD3", "MD2", "M3D", "LWS", "LWO", "IRR", "IRRMESH", "IQM", "IFC", "HMP", "GLTF",
        "GLTF2", "FBX", "DXF", "CSM", "COLLADA", "COB", "C4D", "BVH", "BLEND", "B3D",
        "ASSBIN", "ASE", "3DS", "AMF", "AC", "3MF",
    ]

    AInoImporters = []
    for fmt in AIdontCare:
        AInoImporters.append("/DASSIMP_BUILD_NO_" + fmt + "_IMPORTER")

    native.cxx_library(
        name = "assimp",
        srcs = AIpatchedSources,
        headers = dicts.add(
            subdir_glob([
                ("Deps/assimp/code", "CApi/**/*.h"),
                ("Deps/assimp/code", "CApi/**/*.hpp"),
                ("Deps/assimp/code", "CApi/**/*.inl"),
                ("Deps/assimp/code", "Common/**/*.h"),
                ("Deps/assimp/code", "Common/**/*.hpp"),
                ("Deps/assimp/code", "Common/**/*.inl"),
                ("Deps/assimp/code", "Material/**/*.h"),
                ("Deps/assimp/code", "Material/**/*.hpp"),
                ("Deps/assimp/code", "Material/**/*.inl"),
                ("Deps/assimp/code", "Pbrt/**/*.h"),
                ("Deps/assimp/code", "Pbrt/**/*.hpp"),
                ("Deps/assimp/code", "Pbrt/**/*.inl"),
                ("Deps/assimp/code", "PostProcessing/**/*.h"),
                ("Deps/assimp/code", "PostProcessing/**/*.hpp"),
                ("Deps/assimp/code", "PostProcessing/**/*.inl"),
                ("Deps/assimp/code", "AssetLib/**/*.h"),
                ("Deps/assimp/code", "AssetLib/**/*.hpp"),
                ("Deps/assimp/code", "AssetLib/**/*.inl"),

                ("Deps/assimp/code/CApi", "**/*.h"),
                ("Deps/assimp/code/CApi", "**/*.hpp"),
                ("Deps/assimp/code/CApi", "**/*.inl"),
                ("Deps/assimp/code/Common", "**/*.h"),
                ("Deps/assimp/code/Common", "**/*.hpp"),
                ("Deps/assimp/code/Common", "**/*.inl"),
                ("Deps/assimp/code/Material", "**/*.h"),
                ("Deps/assimp/code/Material", "**/*.hpp"),
                ("Deps/assimp/code/Material", "**/*.inl"),
                ("Deps/assimp/code/Pbrt", "**/*.h"),
                ("Deps/assimp/code/Pbrt", "**/*.hpp"),
                ("Deps/assimp/code/Pbrt", "**/*.inl"),
                ("Deps/assimp/code/PostProcessing", "**/*.h"),
                ("Deps/assimp/code/PostProcessing", "**/*.hpp"),
                ("Deps/assimp/code/PostProcessing", "**/*.inl"),
                ("Deps/assimp/code/AssetLib", "**/*.h"),
                ("Deps/assimp/code/AssetLib", "**/*.hpp"),
                ("Deps/assimp/code/AssetLib", "**/*.inl"),

                ("Deps/assimp/include", "**/*.h"),
                ("Deps/assimp/include", "**/*.hpp"),
                ("Deps/assimp/include", "**/*.inl"),
            ]),
            {
                "assimp/config.h": "DepGenerated/assimp/config.h",
                "assimp/revision.h": "DepGenerated/assimp/revision.h",
                "config.h": "DepGenerated/assimp/config.h",
                "revision.h": "DepGenerated/assimp/revision.h",
            }
        ),
        compiler_flags = [
            "/DASSIMP_BUILD_NO_M3D_IMPORTER",
            "/DASSIMP_BUILD_NO_M3D_EXPORTER",
            "/D_CRT_SECURE_NO_WARNINGS",
            "/D_SCL_SECURE_NO_WARNINGS",
            "/DWIN32_LEAN_AND_MEAN",
            "/DASSIMP_BUILD_NO_OWN_ZLIB=1",
            "/DNDEBUG",
            "/bigobj",
            "/wd4244",
            "/DASSIMP_BUILD_NO_EXPORT",
        ] + AInoImporters,
        link_style = "static",
        link_whole = False,
        visibility = ["PUBLIC"],
        deps = [
            ":zlib",
            ":unzip",
            ":pugixml",
            ":stb_image",
            ":utf8cpp",
        ],
    )

    d_rules.d_library(
        name = "bindbc-loader",
        srcs = {
            "bindbc/loader/package.d": "Deps/bindbc-loader/source/bindbc/loader/package.d",
            "bindbc/loader/sharedlib.d": "Deps/bindbc-loader/source/bindbc/loader/sharedlib.d",
            "bindbc/loader/system.d": "Deps/bindbc-loader/source/bindbc/loader/system.d",
        },
        visibility = ["PUBLIC"],
    )

    d_rules.d_library(
        name = "bindbc-assimp",
        srcs = {
            "bindbc/assimp/binddynamic.d": "Deps/bindbc-assimp/source/bindbc/assimp/binddynamic.d",
            "bindbc/assimp/bindstatic.d": "Deps/bindbc-assimp/source/bindbc/assimp/bindstatic.d",
            "bindbc/assimp/package.d": "Deps/bindbc-assimp/source/bindbc/assimp/package.d",
            "bindbc/assimp/types.d": "Deps/bindbc-assimp/source/bindbc/assimp/types.d",
        },
        visibility = ["PUBLIC"],
        deps = [":assimp", ":bindbc-loader"],
    )

    d_rules.d_library(
        name = "intel-intrinsics",
        srcs = {
            "inteli/avx2intrin.d": "Deps/intel-intrinsics/source/inteli/avx2intrin.d",
            "inteli/avxintrin.d": "Deps/intel-intrinsics/source/inteli/avxintrin.d",
            "inteli/bmi2intrin.d": "Deps/intel-intrinsics/source/inteli/bmi2intrin.d",
            "inteli/emmintrin.d": "Deps/intel-intrinsics/source/inteli/emmintrin.d",
            "inteli/internals.d": "Deps/intel-intrinsics/source/inteli/internals.d",
            "inteli/math.d": "Deps/intel-intrinsics/source/inteli/math.d",
            "inteli/mmx.d": "Deps/intel-intrinsics/source/inteli/mmx.d",
            "inteli/nmmintrin.d": "Deps/intel-intrinsics/source/inteli/nmmintrin.d",
            "inteli/package.d": "Deps/intel-intrinsics/source/inteli/package.d",
            "inteli/pmmintrin.d": "Deps/intel-intrinsics/source/inteli/pmmintrin.d",
            "inteli/shaintrin.d": "Deps/intel-intrinsics/source/inteli/shaintrin.d",
            "inteli/smmintrin.d": "Deps/intel-intrinsics/source/inteli/smmintrin.d",
            "inteli/tmmintrin.d": "Deps/intel-intrinsics/source/inteli/tmmintrin.d",
            "inteli/types.d": "Deps/intel-intrinsics/source/inteli/types.d",
            "inteli/xmmintrin.d": "Deps/intel-intrinsics/source/inteli/xmmintrin.d",
        },
        visibility = ["PUBLIC"],
    )

    d_rules.d_library(
        name = "gfm",
        srcs = {
            "gfm/math/box.d": "Deps/gfm/math/gfm/math/box.d",
            "gfm/math/funcs.d": "Deps/gfm/math/gfm/math/funcs.d",
            "gfm/math/matrix.d": "Deps/gfm/math/gfm/math/matrix.d",
            "gfm/math/package.d": "Deps/gfm/math/gfm/math/package.d",
            "gfm/math/quaternion.d": "Deps/gfm/math/gfm/math/quaternion.d",
            "gfm/math/shapes.d": "Deps/gfm/math/gfm/math/shapes.d",
            "gfm/math/simplerng.d": "Deps/gfm/math/gfm/math/simplerng.d",
            "gfm/math/vector.d": "Deps/gfm/math/gfm/math/vector.d",
        },
        visibility = ["PUBLIC"],
        deps = [":intel-intrinsics"],
    )

    d_rules.d_binary(
        name = "ModelBaker.exe",
        srcs = {
            "mesh/basic_writer.d": "CodeGen/ModelBaker/source/mesh/basic_writer.d",
            "mesh/loader.d": "CodeGen/ModelBaker/source/mesh/loader.d",
            "app.d": "CodeGen/ModelBaker/source/app.d",
        },
        linker_flags = [
            "kernel32.lib",
            "user32.lib",
            "shell32.lib",
            "uuid.lib",
            "ole32.lib",
            "rpcrt4.lib",
            "advapi32.lib",
            "wsock32.lib",
            "Version.lib",
            "/SUBSYSTEM:console",
        ],
        deps = [":assimp", ":bindbc-assimp", ":gfm"],
    )

    d_rules.d_binary(
        name = "paper.exe",
        srcs = {
            "constructs/all_of_struct.d": "CodeGen/MCM/paper/source/constructs/all_of_struct.d",
            "constructs/arena.d": "CodeGen/MCM/paper/source/constructs/arena.d",
            "constructs/auto_array.d": "CodeGen/MCM/paper/source/constructs/auto_array.d",
            "constructs/const_struct.d": "CodeGen/MCM/paper/source/constructs/const_struct.d",
            "constructs/decl_offset_group.d": "CodeGen/MCM/paper/source/constructs/decl_offset_group.d",
            "constructs/iconstruct.d": "CodeGen/MCM/paper/source/constructs/iconstruct.d",
            "constructs/impl_offset_group.d": "CodeGen/MCM/paper/source/constructs/impl_offset_group.d",
            "constructs/include.d": "CodeGen/MCM/paper/source/constructs/include.d",
            "constructs/struct_alias.d": "CodeGen/MCM/paper/source/constructs/struct_alias.d",
            "constructs/struct_invoke_on.d": "CodeGen/MCM/paper/source/constructs/struct_invoke_on.d",
            "constructs/struct_invoke_switchifeq.d": "CodeGen/MCM/paper/source/constructs/struct_invoke_switchifeq.d",
            "keywords/package.d": "CodeGen/MCM/paper/source/keywords/package.d",
            "tokenizer/package.d": "CodeGen/MCM/paper/source/tokenizer/package.d",
            "tokenizer/tokens.d": "CodeGen/MCM/paper/source/tokenizer/tokens.d",
            "tokenizer/utils.d": "CodeGen/MCM/paper/source/tokenizer/utils.d",
            "app.d": "CodeGen/MCM/paper/source/app.d",
            "lex_machine.d": "CodeGen/MCM/paper/source/lex_machine.d",
            "mangler.d": "CodeGen/MCM/paper/source/mangler.d",
            "result.d": "CodeGen/MCM/paper/source/result.d",
        },
        linker_flags = [
            "kernel32.lib",
            "user32.lib",
            "shell32.lib",
            "uuid.lib",
            "ole32.lib",
            "rpcrt4.lib",
            "advapi32.lib",
            "wsock32.lib",
            "Version.lib",
            "/SUBSYSTEM:console",
        ],
    )
