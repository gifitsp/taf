## 简介
taf是一个交易应用基础库(Trading Application Foundations)，基于Intel x86-64体系，为交易系统提供在x64上的低延迟优化解决方案。

## 特点
* 使用Intel TBB并行库、TBB内存池、SIMD（AVX2+）、无锁并发队列等优化，尽可能的降低应用执行时间（提供纳秒级时间精度）
* 非常轻量级的代码框架，尽可能减少不必要的代码
* 使用标准C++跨平台编译运行（windows/linux），编译安装非常简单，除TBB外无任何依赖（下载包已包括编译好的TBB库文件，无需自己编译安装）
* 提供基本的订单管理（增、删、查询等），使用者只需关注应用（策略）本身的逻辑
* 可动态运行和停止应用（策略）实例
* 提供C接口，可使用Python等其他语言调用（核心代码建议用C++，以提高性能）
* 对于简单的应用（策略），平均延迟 < 10微秒
* 只提供基础框架，不涉及任何业务和交易所。可用于任何交易系统

## 使用准备
* 需要支持AVX2或AVX512F的机器，可参考：  
  https://stackoverflow.com/questions/28939652/how-to-detect-sse-sse2-avx-avx2-avx-512-avx-128-fma-kcvi-availability-at-compile
* 字符串的simd搜索可参考：  
  http://0x80.pl/articles/simd-friendly-karp-rabin.html
* 支持C++17、x64位的编译器
* TBB已编译依赖库在taf/libs/tbb/bin/下

## 安装编译

**Windows**
  * 请使用Visual Studio 2019或以上版本，版本号不低于16.7.3
  * sln文件在taf/master/build/msvs/taf.sln，包括libs文件目录和example工程
  * 只能选择x64编译
  * 需要手动检查机器支持avx2或avx512的情况（比如用cpuz），在taf/master/src/libs/taf/common/simd/simd.hpp中手动开关USE_AVX512或USE_AVX512_BELOW宏（其中之一）
  * 需要配置TBB的库文件路径以及环境变量（Configration->Debugging->Environment: path=...），具体可参考example工程属性中的配置
  
**Linux**
  * 最好是gcc 8.2或以上版本
  * 简单的example的makefile在taf/master/build/下
  * 由于被依赖代码的原因，请使用g++而不是gcc编译，并且必须添加-mavx2（-mavx512f） -std=c++17开关，其中avx2或avx512取决于机器支持的情况（不支持的情况下运行会崩溃）
  * debug/release或其他编译选项等可直接修改makefile
  * 需要配置TBB库文件的环境变量，比如export LD_LIBRARY_PATH=... : ./taf/libs/tbb/bin/linux
  * 编译运行example：  
    $ cd taf/master/build  
    $ make  
    $ ./example
    
## 示例说明
  示例文件(taf/master/build/example.cpp)演示了如何定义交易所API类（无状态）、应用（策略）类等，以及多个交易所、多个API组合（在同一个策略类中），包括下单、撤单、查询等（未提供获取行情相关的功能）
  
## 其他讨论
  * 未使用TBB的的concurrent容器，因为在并行量不是很大的情况下，编译器优化的STL容器的消耗更小
  * TBB的pipeline模式需要额外几微秒到10微秒的消耗，故未使用
  * 经测试，example中的测试用例运行稳定后在win10中比在centos7的平均延时更小（均为AVX2）
