提供静态注入，全局hook，hook模块动态加载等功能

Hook模块路径目前写死：'/data/local/tmp/HMTest.dex'，一个dex里可以包含多个hook模块

静态注入的实现参考 [https://bbs.pediy.com/thread-252266.htm]

然后把这个项目编译出来的so改个名丢进/system/lib即可

没有root权限或者/system不可写的可以改目标app或者在VA框架里hook

--FKD
