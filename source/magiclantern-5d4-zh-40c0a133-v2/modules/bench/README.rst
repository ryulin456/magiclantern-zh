Benchmarks
==========

:Author: a1ex
:License: GPL
:Summary: 5D4 1.3.3 基准测试安全锁
:Forum: http://www.magiclantern.fm/forum/index.php?topic=12630.0

上游 5D4 构建清单保留了基准测试模块，但平台内存、缓存、存储卡、
图像缓冲区和 EDMAC 地址尚未完成实机验证。

本开发测试包在构建时排除所有测试实现，仅编入安全锁提示。
没有内存、存储卡、缓存、EDMAC 或显示测试回调，不能通过菜单取消锁定。
