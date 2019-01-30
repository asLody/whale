# Whale
![logo][0]

[Chinese Version](https://github.com/asLody/whale/blob/master/README.zh-CN.md)

[![license](http://img.shields.io/badge/license-Apache2.0-brightgreen.svg?style=flat)](https://github.com/alibaba/atlas/blob/master/LICENSE)
## Overview
Whale is a cross-platform Hook Framework, allowed to run *Android、IOS、Linux、MacOS*.
Whale support both **ARM/THUMB, ARM64, X86, X86_64 (AMD64)**, This covers almost all the major devices available today.

## Feature
#### Android
* **Xposed-Style** Method Hook
* Modify the inheritance relationship between classes at runtime
* Modifies the class to which the object belongs at runtime
* bypass `Hidden API Policy`

#### Darwin/Linux Platforms
* Internal symbol resolver
* Native Hook

#### IOS Restrictions
InlineHook on IOS is only usable in `debug compile mode` on non-jailbreak devices.
Release compilation mode will not work properly.

To solve this problem，Whale will provide a new core named `Binary Static Inline Hook`.

`Binary Static Inline Hook` will open source in the near future.


## What can you do with it?
* Turn on the god mode of an app
* The act of monitoring or tampering with app
* Instant hotfix
* SandBox
* Inject to system and instead of Xposed

## Compatibility
- [x] Android 5.0.0
- [x] Android 5.1.1
- [x] Android 6.0
- [x] Android 6.0.1
- [x] Android 7.1.2
- [x] Android 8.1.0
- [x] Android 9.0.0
- [x] IOS 11.3
- [x] IOS 12.0
- [x] MacOS mojave (10.14)
- (Not in the list means `untested` ）

## InlineHook
For `pcrel instruction`, Whale will convert it to `pc-independent instruction`，
If the Hook procedure have not convert instructions, please feedback to ` issue `.

## About Jit
Whale has a `built-in Jit Engine`, When you have more advanced Hook requirements, you can directly **generate executable instructions** in memory through the Jit.
There is no longer the need to generate ugly hard code through tools as before.

## Compile
We have pre-built binary versions of Android & IOS. You can find them in the built directory.

Whale uses CMake to build projects, so you need to install CMake on your system.

#### Android
1. If you need to use ` Java Hook ` please copy java folder to your project.

2. Direct use of binary，You just copy the files under ++built/Android++ to ++src/main/jniLibs++ in your project.

3. If you need to compile the source code, specify `CMakeLists.txt` in build.gradle:
```
externalNativeBuild {
cmake {
path "your/whale/path/CMakeLists.txt"
}
}
```

#### IOS
```
cd toolchain

cmake .. \
-DCMAKE_TOOLCHAIN_FILE=ios.toolchain.cmake \
-DIOS_PLATFORM=OS64 \
-DPLATFORM=IOS \
-DIOS_ARCH=arm64 \
-DENABLE_ARC=0 \
-DENABLE_BITCODE=0 \
-DENABLE_VISIBILITY=0 \
-DIOS_DEPLOYMENT_TARGET=9.3 \
-DSHARED=ON \
-DCMAKE_BUILD_TYPE=Release

make -j4
```

#### Ohter platforms
```
cmake .
make -j4
```

## Technogy communication
> [GOTO => Discord](https://discord.gg/j2Cdy2g)

> Email: imlody@foxmail.com


[0]: https://github.com/asLody/whale/blob/master/LOGO.png?raw=true
