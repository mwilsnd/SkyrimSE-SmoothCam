WINDOWS_BASE_LIBS = [
    "kernel32.lib",
    "user32.lib",
    "shell32.lib",
    "uuid.lib",
    "ole32.lib",
    "rpcrt4.lib",
    "advapi32.lib",
    "wsock32.lib",
    "Version.lib",
]

BASE_COMPILER_OPTS = [
    "/await",
    "/FS",
    "/EHsc",
    "/permissive-",
    "/Zc:__cplusplus",
    "/Zc:externConstexpr",
    "/Zc:inline",
    "/Zc:lambda",
    "/Zc:referenceBinding",
    "/Zc:rvalueCast",
    "/Zc:strictStrings",
    "/Zc:ternary",
    "/Zc:wchar_t",
    "/std:c++latest",
    "/D_UNICODE",
    "/DUNICODE",
    "/DBUCK",
]

NO_RTTI = [
    "/GR-",
]

WARNINGS = [
    "/W4",
    "/WX",
]