# bindbc-assimp
This project provides both static and dynamic bindings to the [Assimp](http://www.assimp.org/). They are `@nogc` and `nothrow` compatible and can be compiled for compatibility with `-betterC`. 
## Usage
By default, `bindbc-assimp` is configured to compile as a dynamic binding that is not `-betterC` compatible. The dynamic binding has no link-time dependency on the Assimp library, so the Assimp shared library must be manually loaded at runtime. When configured as a static binding, there is a link-time dependency on the Assimp library---either the static library or the appropriate file for linking with shared libraries on your platform (see below).

When using DUB to manage your project, the static binding can be enabled via a DUB `subConfiguration` statement in your project's package file. `-betterC` compatibility is also enabled via subconfigurations.

To use Assimp, add `bindbc-assimp` as a dependency to your project's package config file. For example, the following is configured to Assimp as a dynamic binding that is not `-betterC` compatible:

__dub.json__
```
dependencies {
    "bindbc-assimp": "~>0.1.0",
}
```

__dub.sdl__
```
dependency "bindbc-assimp" version="~>0.1.0"
```

### The dynamic binding
The dynamic binding requires no special configuration when using DUB to manage your project. There is no link-time dependency. At runtime, the Assimp shared library is required to be on the shared library search path of the user's system. On Windows, this is typically handled by distributing the Assimp DLL with your program. On other systems, it usually means the user must install the Assimp runtime library through a package manager.

To load the shared library, you need to call the `loadAssimp` function. This returns a member of the `AssimpSupport` enumeration (See [the README for `bindbc.loader`](https://github.com/BindBC/bindbc-loader/blob/master/README.md) for the error handling API):

* `AssimpSupport.noLibrary` indicating that the library failed to load (it couldn't be found)
* `AssimpSupport.badLibrary` indicating that one or more symbols in the library failed to load
* a member of `AssimpSupport` indicating a version number that matches the version of Assimp that `bindbc-assimp` was configured at compile-time to load.
```d
import bindbc.assimp;

/*
This version attempts to load the Assimp shared library using well-known variations
of the library name for the host system.
*/
AssimpSupport ret = loadAssimp();

// Handle error. For most use cases, its reasonable to use the the error handling API in
// bindbc-loader to retrieve error messages for logging and then abort. If necessary, it's
// possible to determine the root cause via the return value:

if(ret == AssimpSupport.noLibrary) {
// Assimp shared library failed to load
}
else if(AssimpSupport.badLibrary) {
// One or more symbols failed to load. The likely cause is that the
// shared library is for a lower version than bindbc-assimp was configured
// to load.

/*
This version attempts to load the Assimp library using a user-supplied file name.
Usually, the name and/or path used will be platform specific, as in this example
which attempts to load `assimp.dll` from the `libs` subdirectory, relative
to the executable, only on Windows.
*/
// version(Windows) loadAssimp("libs/assimp.dll")
```

No matter which version was configured, the successfully loaded version can be obtained via a call to `loadedAssimpVersion`. It returns one of the following:

* `AssimpSupport.noLibrary` if `loadAssimp` returned `AssimpSupport.noLibrary`
* `AssimpSupport.badLibrary` if `loadAssimp` returned `AssimpSupport.badLibrary` and no version of Assimp successfully loaded
* a member of `AssimpSupport` indicating the version of Assimp that successfully loaded. When `loadAssimp` returns `AssimpSupport.badLibrary`, this will be a version number lower than that configured at compile time. Otherwise, it will be the same as the manifest constant `fiSupport`.

The function `isAssimpLoaded` returns `true` if any version of Assimp was successfully loaded and `false` otherwise.

## The static binding
The static binding has a link-time dependency on either the shared or the static Assimp library. On Windows, you can link with the static library or, to use the shared library (`assimp.dll`), with the import library. On other systems, you can link with either the static library or directly with the shared library. This requires the Assimp development package be installed on your system at compile time, either by compiling the Assimp source yourself, downloading the Assimp precompiled binaries for Windows, or installing via a system package manager. [See the Assimp download page](http://www.assimp.org/index.php/downloads) for details.

When linking with the static library, there is no runtime dependency on Assimp. When linking with the shared library (or the import library on Windows), the runtime dependency is the same as the dynamic binding, the difference being that the shared library is no longer loaded manually---loading is handled automatically by the system when the program is launched.

Enabling the static binding can be done in two ways.

### Via the compiler's `-version` switch or DUB's `versions` directive
Pass the `BindAssimp_Static` version to the compiler and link with the appropriate library.

When using the compiler command line or a build system that doesn't support DUB, this is the only option. The `-version=BindAssimp_Static` option should be passed to the compiler when building your program. All of the required C libraries, as well as the `bindbc-assimp` and `bindbc-loader` static libraries must also be passed to the compiler on the command line or via your build system's configuration.

When using DUB, its `versions` directive is an option. For example, when using the static binding:

__dub.json__
```
"dependencies": {
    "bindbc-assimp": "~>0.1.0"
},
"versions": ["BindAssimp_Static"],
"libs": ["assimp"]
```

__dub.sdl__
```
dependency "bindbc-assimp" version="~>0.1.0"
versions "BindAssimp_Static"
libs "assimp"
```

### Via DUB subconfigurations
Instead of using DUB's `versions` directive, a `subConfiguration` can be used. Enable the `static` subconfiguration for the `bindbc-assimp` dependency:

__dub.json__
```
"dependencies": {
    "bindbc-assimp": "~>0.1.0"
},
"subConfigurations": {
    "bindbc-assimp": "static"
},
"libs": ["assimp"]
```

__dub.sdl__
```
dependency "bindbc-assimp" version="~>0.1.0"
subConfiguration "bindbc-assimp" "static"
libs "assimp"
```

This has the benefit that it completely excludes from the build any source modules related to the dynamic binding, i.e. they will never be passed to the compiler.

## `betterC` support

`betterC` support is enabled via the `dynamicBC` and `staticBC` subconfigurations, for dynamic and static bindings respectively. To enable the static binding with `-betterC` support:

__dub.json__
```
"dependencies": {
    "bindbc-assimp": "~>0.1.0"
},
"subConfigurations": {
    "bindbc-assimp": "staticBC"
},
"libs": ["assimp"]
```

__dub.sdl__
```
dependency "bindbc-assimp" version="~>0.1.0"
subConfiguration "bindbc-assimp" "staticBC"
libs "assimp"
```
When not using DUB to manage your project, first use DUB to compile the BindBC libraries with the `dynamicBC` or `staticBC` configuration, then pass `-betterC` to the compiler when building your project.
