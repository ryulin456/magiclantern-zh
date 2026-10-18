# 中文版魔灯安装包

## 2026-10-18 修正

- [7D 2.0.3 r2](./magiclantern.2020Dec28.7D.203.zh-CN.r2-CORE-PACKAGING-FIX-TEST-ONLY.zip)：修复核心误装，19 个模块保留。
- [1200D 1.0.2 r2](./magiclantern.2018Aug21.1200D.102.zh-CN.r2-CORE-PACKAGING-FIX-LOW-MEMORY-TEST-ONLY.zip)：修复核心误装，17 个模块保留，仍为低内存测试版。
- [5D3 1.2.3 Live AF r21](./magiclantern.2025-06-20.5D3.123.zh-CN.experimental-r9.live-af-lv-gated-raw-compatible-r21-TEST-ONLY.zip)：正确文件名入口。

旧 7D / 1200D r1 包含错误的 870 字节核心，请停止使用。两份 r2 与旧 r1 相比仅 autoexec.bin 改变，其余成员字节一致；静态校验及模块符号依赖通过，尚未做本次实机启动测试。详见[修复说明](../source/publication-docs/LEGACY-R2-PACKAGING-FIX.md)。

旧 5D3 文件名写成 .113，但其 Git 文件哈希与本地 .123 包完全相同；实际对应佳能固件 1.2.3。旧链接为兼容已有引用而保留，请使用上方 .123 入口，不能用于 1.1.3。

[源代码与构建说明](../source/README.md) · [本次 r2 校验值](./SHA256SUMS-r2.txt)
