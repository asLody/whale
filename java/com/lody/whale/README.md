这个模块类似于Xposed的XposedCompact.jar的作用，提供hook接口

在模块里必须complieOnly！！！

dex路径依然写死：/data/local/tmp/whale.dex在native_onload.h的DEX_PATH宏

主要代码还是由asLody大佬写的，我的工作主要就是加一些jar包模式（AS的jar包默认不引入android类，只能改反射了）
和全局注入下的适配

--FKD

