# 历史机型汉化发布审计（2026-08-26）

本批次按原始安装包对应的历史源码分支构建，资源仅增加中文语言包、BFNT 字体和许可证文件；原包条目逐项保留，`.mo` 模块不删减。

| 原始包 | 汉化包 | 模块 | 版本字符串 | SHA-256 |
|---|---|---:|---|---|
| `magiclantern-lua_fix.2020Dec28.7D203.zip` | [7D203 汉化包](magiclantern-lua_fix.2020Dec28.7D203.zh-CN.r1-FULL-MODULES-TEST-ONLY.zip) | 19/19 | `lua_fix.2020Dec28.7D203` | `75E57A7B8062903F6A10D7C283D2893D6CBB5D97AFF5B1FCC1EEEA46919B1BA6` |
| `magiclantern-Nightly.2018Aug21.1200D102.zip` | [1200D102 汉化包](magiclantern-Nightly.2018Aug21.1200D102.zh-CN.r1-FULL-MODULES-LOW-MEMORY-TEST-ONLY.zip) | 17/17 | `Nightly.2018Aug21.1200D102` | `D51BC85FC94C8E5532F3F474FD174AEB3F53867701D881F520D17BC0503517AE` |
| `magiclantern.2025-06-20.5D2.212.zip` | [5D2.212 汉化包](magiclantern.2025-06-20.5D2.212.zh-CN.r1-FULL-MODULES-TEST-ONLY.zip) | 19/19 | `2025-06-20.5D2.212` | `580C318570C691F7F65ADEA5D19360B3BB18B0D2345D05CEDD6D55B902DEFCE6` |

## 自动检查结果

- 语言字典：2049 条；BFNT：921 个字形；缺字检查为 0。
- 核心菜单：7D、1200D、5D2 均 0 条缺失；历史模块菜单、动态格式串、占位符审计均 100%。
- 模块符号：7D 19/19、1200D 17/17、5D2 19/19；所有未定义符号均有提供者。
- 原包条目：分别 62→65、60→63、61→64；仅新增 `ML/doc/OFL-NotoSansSC.txt`、`ML/fonts/zh_cn.lng`、`ML/fonts/zh_cn.bfn`，无删除。

## 内存代理检查

- 7D：原版 LOAD `MemSiz=0x820a8`，汉化 `0x82ca8`，增长 `0xC00`（3072 字节）；动态 `_bss_end` 模式通过。
- 1200D：原版 `0x7be28`，汉化 `0x7ca28`，增长 `0xC00`；固定约 512 KiB 预留区剩余 `0x35D8`（13784 字节），因此包名标为 `LOW-MEMORY-TEST-ONLY`，应先实机冷启动、插卡启动和逐模块加载测试。
- 5D2：原版 `0x8b200`，汉化 `0x8be00`，增长 `0xC00`；动态 `_bss_end` 成对构建通过。

以上是构建和静态代理检查结果，不等同于已在真实相机上完成冷启动、插卡启动、录像和全部模块操作测试；发布前仍应按标准逐机实测。
