这是Cydia的inline hook模块，做了一些小修改好让它能在最新的clang上编译通过（abs函数似乎被新的c++标准重载了？）
whale的那个WInlineHookFunction在我的所有能拿到的测试环境上（SM-P550真机，AOSP，GENYMOTION，雷电模拟器）只要调用原函数必定段错误。。。
为了hook JNI_CreateJVM混进java世界，只能先上MSHook了。其实xhook也可以，但是iqiyi的东西。。。协议不是人话看不懂，有版权问题就不大好了
--FKD
