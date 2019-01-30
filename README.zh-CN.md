# Whale
![logo][0]

## 概述
Whale是一个跨平台的Hook Framework，同时支持Android、IOS、Linux、MacOS。
Whale 支持**ARM/THUMB、ARM64、X86、X86_64 (AMD64)**，这几乎覆盖了目前所有主流的设备。

## 特性
#### Android
* **Xposed-Style** Method Hook
* 运行时修改类之间的继承关系
* 修改对象所属的类
* 绕过`Hidden API Policy`

#### Darwin/Linux Platforms
* Internal symbol resolver
* Native Hook

#### IOS的限制
IOS的InlineHook在非越狱设备上只限在debug编译模式下开启，
release编译模式下将无法正常工作。

为了解决这个问题，Whale将提供`Binary Static Inline Hook`。

IOS下的`Binary Static Inline Hook`将在近期开源。


## 你可以用它做什么？
* 开启App的上帝模式
* 监控或篡改软件的行为
* 即时生效的热修复
* SandBox
* 注入到系统代替Xposed

## Whale的兼容性
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
- (不在清单内表示 `未测试` ）

## InlineHook
对于`pcrel指令`, Whale会将其转换为`pc 无关指令`，
如果在Hook过程有遇到未转换的指令，请提`issue`。

## 关于Jit
Whale内置了 **Jit Engine**, 当你有更高级的Hook需求时可以通过Jit直接在内存中生成可执行的指令。
不再需要像从前那样通过工具来生成丑陋的hard code。

## 编译
我们已提前编译了Android & IOS的**二进制版本**，您可以在`built目录`找到它们。

Whale使用了CMake来构建项目，所以你需要在你的系统上安装CMake。

#### Android
1. 如果需要使用`Java Hook`, 请把java文件夹的代码复制到你的项目。

2. 直接使用二进制，你只需要复制 `built/Android` 下你所需的abi到你的项目的src/main/jniLibs下。

3. 如果需要编译源码，请在build.gradle中指定CMakelists.txt：
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
make -j8
```

## Technogy communication
> **QQ Group: 977793836**



[0]: https://github.com/asLody/whale/blob/master/LOGO.png?raw=true
