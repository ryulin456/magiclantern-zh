# 各机型最后发布版本与源码（含 7D / 1200D r2 修复）

桌面 20 个目标及独立 5D4，共 21 个目标。7D / 1200D 在本次修复后以 outputs 中的 r2 为最新；桌面旧 r1 尚未覆盖。

| 机型 / 原厂版本 | 最后修订 | 源码目录 | 核对结果 |
|---|---|---|---|
| 100D.101 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 1100D.105 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 1200D.102 | r2 CORE-PACKAGING-FIX LOW-MEMORY | `work/src-1200D-zh` | r2 已修复核心选择；核心和全部模块匹配 |
| 200D.101 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 500D.111 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 50D.109 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 550D.109 | r3 LOW-MEMORY | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 5D2.212 | r1 | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 5D3.123 | experimental-r9 + Live AF r21 | `magiclantern-zh-src (AF and later localization only)` | AF r21 可复现；旧 r9 汉化核心源码缺失 |
| 5D4.133 | r1 DEVELOPER-BOOT-TEST | `magiclantern-5d4-zh-40c0a133-v2` | 核心和全部模块匹配本地编译产物 |
| 600D.102 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 60D.111 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 650D.104 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 6D.116 | r4 | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 6D2.111 | r3 font-layout | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 700D.115 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 70D.112 | r3 MEMORY-SAFE | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 750D.110 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| 7D.203 | r2 CORE-PACKAGING-FIX | `work/src-lua-fix-zh` | r2 已修复核心选择；核心和全部模块匹配 |
| 7D2.112 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |
| EOSM.202 | r2 FULL-MODULES | `magiclantern-zh-src` | 核心和全部模块匹配本地编译产物 |

完整包名、包 SHA-256 见同目录 CSV；逐成员哈希见 PACKAGE-MEMBER-AUDIT.json，7D / 1200D 历史错误证据保留在 previous_release。匹配保留编译产物不等于本次全机型清洁重编译或实机验证。
