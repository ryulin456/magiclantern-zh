# Magic Lantern 简体中文汉化与 5D3 Live AF 源码

整理日期：2026-10-18。维护者：ryulin456。这里汇集本机最后保留的汉化源码、构建脚本、翻译资源及发布包对应证据，供社区审核与继续开发。

**源码覆盖有明确缺口：5D3 原始 experimental-r9 汉化核心及其旧模块来自另一台电脑，本机只有相应二进制，没有找到精确对应的完整修改源码。此仓库不能宣称是全部已分发二进制的完整对应源码。** Live AF r21 的源码独立保留，已可重新编译核对。

## 目录

| 路径 | 内容 |
|---|---|
| `magiclantern-zh-src/` | 2025 多机型公共汉化工程，含 6D r4、6D2 r3、550D r3、70D r3 的相关实现与 5D3 Live AF r21 |
| `work/src-lua-fix-zh/` | 7D 2.0.3 / 2020 lua_fix 独立汉化树 |
| `work/src-1200D-zh/` | 1200D 1.0.2 / 2018 Nightly 独立汉化树 |
| `magiclantern-5d4-zh-40c0a133-v2/` | 5D4 1.3.3 开发 r1 的最终独立源码；含安全隔离和字体验证修改 |
| `scripts/`、`work/policies/` | 历史翻译生成、审计、打包脚本及模块清单 |
| `publication-docs/` | 21 个目标的版本映射、哈希证据、构建说明、历史审计 |
| `reproduction-metadata/` | 保留的构建版本字符串及模块元数据，用于追查历史构建 |

先阅读 [各机型对应表](publication-docs/FIRMWARE-SOURCE-MAP.md)、[已知缺口与打包错误](publication-docs/KNOWN-ISSUES.md) 和 [构建说明](BUILDING.md)。

20 个目标的当前发布核心及全部模块与本机编译产物一致；其中 7D、1200D 已用 r2 修复旧 r1 的核心打包错误。这不是全机型清洁构建验证。5D3 r9 源码缺口仍然存在。普通机型共享源文件后来有更新，应以映射说明为准。

## 状态

本项目是非官方实验性中文化与模块开发工程。静态审计和编译不代表实机通过。5D4 仍是开发端口，不能当作普通可安装版本。源码归档不包含可直接安装的相机固件包。

源码保留各树原 `COPYING` 和文件内版权；字体见 `doc/OFL-NotoSansSC.txt`，第三方代码遵循各自许可。不能把全仓库统一改标为 MIT。项目署名邮箱属于原有公开署名并保留。

## English note for reviewers

This repository collects the latest locally preserved Simplified Chinese localization source trees and experimental 5D3 Live AF r21 source. It also includes a separate 5D4 development tree. It is not a complete corresponding-source archive for every historical binary: the original 5D3 experimental-r9 localized core and its old module modifications were supplied as binaries from another computer, and their exact modified source is missing locally. The exported r21 AF module can be rebuilt and compared separately. The historical 7D and 1200D r1 ZIPs contained an incorrectly selected 870-byte 5DC helper in place of their camera core; corrected r2 packages now use the exact preserved camera cores and keep all other members unchanged. See the source map, known issues and build notes before reviewing or redistributing a historical package.
