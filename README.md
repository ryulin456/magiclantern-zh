<p align="center">
  <img src="imagesbanner.png" alt="Magic Lantern 中文汉化项目" width="100%" />
</p>

<h1 align="center">Magic Lantern 中文汉化版</h1>

<p align="center">让老单反的进阶功能，更容易被中文用户理解和使用。</p>

<p align="center">
  <a href="https://github.com/ryulin456/magiclantern-zh"><img src="https://img.shields.io/badge/Platform-Canon-red" alt="Platform: Canon" /></a>
  <a href="https://github.com/ryulin456/magiclantern-zh"><img src="https://img.shields.io/badge/Language-简体中文-1677ff" alt="Language: 简体中文" /></a>
  <a href="https://github.com/ryulin456/magiclantern-zh/stargazers"><img src="https://img.shields.io/github/stars/ryulin456/magiclantern-zh?style=flat" alt="GitHub stars" /></a>
  <a href="https://github.com/ryulin456/magiclantern-zh/issues"><img src="https://img.shields.io/github/issues/ryulin456/magiclantern-zh?style=flat" alt="GitHub issues" /></a>
  <img src="https://img.shields.io/badge/License-GPL--2.0-green" alt="License: GPL-2.0" />
</p>

<p align="center">
  <a href="#项目介绍">项目介绍</a> ·
  <a href="#测试状态">测试状态</a> ·
  <a href="#源码与构建">源码</a> ·
  <a href="#安装前必读">安装说明</a> ·
  <a href="#问题反馈">问题反馈</a> ·
  <a href="SUPPORT.md">捐赠支持</a>
</p>

---

## 项目介绍

[Magic Lantern](https://www.magiclantern.fm/) 是为部分 Canon 单反相机提供进阶功能的第三方开源固件项目。本仓库在保留原有功能与模块的前提下，持续进行简体中文菜单、模块说明和使用提示的汉化与整理。

本项目的目标是让中文用户更容易理解功能，同时坚持以下原则：

- 不因汉化删除原安装包中的模块或功能；
- 中文文字尽量完整显示，并检查缺字、笔画和菜单布局；
- 每个发布包保留原始机型与佳能固件版本标识；
- 对已发现的风险、限制和未测试项目如实标注。

> [!WARNING]
> Magic Lantern 属于第三方固件，刷写和使用均有风险。仅使用与**相机型号、佳能官方固件版本完全一致**的安装包；请先备份存储卡内容，并自行评估风险。项目与 Canon 无隶属关系。

## 项目特点

- 5D3／1.2.3 新增英语、中文、德语、波兰语即时切换测试版
- 深度汉化主菜单、隐藏模块及模块内子选项
- 保留原有模块，不以删功能换取“可编译”
- 针对字体缺字、文字截断与显示笔画进行发布前检查
- 5D3 提供实验性的视频框选自动对焦模块
- 制作了5D4版本（纯理论LLM制造，适配版本1.3.3，风险高，不确定能否使用，欢迎反馈）
- 提供多机型汉化安装包，并持续收集实机反馈

## 测试状态

| 机型 | 当前状态 | 说明 |
| --- | --- | --- |
| Canon 5D Mark III | ✅ 已实机测试 | 四语言 r4 已在 1.2.3 实测；旧版对焦包独立保留 |
| Canon 5D Mark II | ✅ 已实机测试 | 基础汉化与模块加载已验证 |
| Canon 6D | ✅ 已实机测试 | 基础汉化与模块加载已验证 |
| Canon 6D Mark II | ✅ 已实机测试 | 基础汉化与模块加载已验证 |
| 仓库内其他机型 | 🚧 待社区反馈 | 请严格核对机型与佳能固件版本 |

“已实机测试”不代表适用于所有镜头、设置或使用场景；请从低风险场景开始验证。

## 源码与构建

当前新增 **5D3／佳能固件 1.2.3 四语言即时切换 r4 测试包**。默认英文，在魔灯 `Prefs → Language` 按 SET 即时切换英语、简体中文、德语和波兰语。维护者已报告 r4 实机测试正常；其他机型尚未验证此框架版本。

- [下载 5D3 四语言 r4 测试包](中文版魔灯/magiclantern.2026-10-27.5D3.123.LANG4-LIVE-r4-TEST-ONLY.zip)
- [对应完整源码（固定提交）](https://github.com/ryulin456/magiclantern_simplified/tree/6f1bb16b853c8046a05c68f86f7ac02e4a727691) · [下载完整源码 ZIP](https://github.com/ryulin456/magiclantern_simplified/archive/6f1bb16b853c8046a05c68f86f7ac02e4a727691.zip)
- [本次增量审阅：6 个独立提交](https://github.com/ryulin456/magiclantern_simplified/compare/8809eccfb5f8d8ab71d19559c9efc88e5ed0610b...6f1bb16b853c8046a05c68f86f7ac02e4a727691)
- [从上游基线查看完整本地化差异](https://github.com/ryulin456/magiclantern_simplified/compare/ed3e7c0dfacfdeaf3349dee73edf9e3621217462...6f1bb16b853c8046a05c68f86f7ac02e4a727691)
- [r4 使用方法、测试范围和 SHA256](source/publication-docs/5D3-FOUR-LANGUAGE-R4.md)
- [5D3 Live AF r21 对焦模块：独立分支](https://github.com/ryulin456/magiclantern_simplified/compare/ed3e7c0dfacfdeaf3349dee73edf9e3621217462...55381a86f2f357aaaf812db0d8c07cecb28a02ca)
- [2026-10-19 旧版审阅补丁归档（含 7D / 1200D 及 5D4 归档差异）](source/review/magiclantern-review-patches-2026-10-19.zip)

r4 已修正子菜单选项、逐行帮助和模块介绍的漏译，词库与可选字体仍从卡中加载。请将包内 `autoexec.bin` 和 `ML` 文件夹配套更新。**此语言测试包不包含旧 r21 自动对焦改动，不是其他型号或 5D3／1.1.3 的安装包。**构建与验证细节见[源码说明](https://github.com/ryulin456/magiclantern_simplified/blob/6f1bb16b853c8046a05c68f86f7ac02e4a727691/doc/LOCALIZATION-5D3-R4.md)。

这两个分支保留真实上游提交历史；提交按现存源码重新分组，不代表找回了最初的开发提交记录。7D / 1200D 基于另一个旧上游，补丁注明各自基线。5D4 差异尚未整理为干净的上游提交。下列源码目录继续作为归档与构建资料。

2026-10-18 已整理并公开四套源码树，包含多机型中文版工程、独立 5D4 开发工程，以及 **5D3 Live AF r21 自动对焦模块源码**。

- [源码总览与目录](source/README.md)
- [5D3 Live AF r21 实现](source/magiclantern-zh-src/modules/af_frame_common.c) · [模块入口与 Makefile](source/magiclantern-zh-src/modules/af_live/)
- [21 个目标的版本与源码对应表](source/publication-docs/FIRMWARE-SOURCE-MAP.md)
- [构建说明](source/BUILDING.md) · [r21 模块复现验证](source/publication-docs/AF-BUILD-VERIFICATION.md)

r21 自动对焦模块已从导出源码重新编译，结果与发布模块哈希一致。**原始 5D3 experimental-r9 汉化核心及旧模块的精确修改源码尚未找回**，因此本目录不能声称是所有历史二进制的完整对应源码。具体边界见[已知问题](source/publication-docs/KNOWN-ISSUES.md)。

**7D / 1200D 请使用修正后的 r2：**旧 r1 包误选了 870 字节的 5DC 辅助文件作为核心，现已修复。新包只更换正确机型核心，原字体、翻译、安装文件和全部模块保持不变，仍待实机验证。

- [7D 2.0.3 — r2 修正版](中文版魔灯/magiclantern.2020Dec28.7D.203.zh-CN.r2-CORE-PACKAGING-FIX-TEST-ONLY.zip)
- [1200D 1.0.2 — r2 修正版（低内存测试）](中文版魔灯/magiclantern.2018Aug21.1200D.102.zh-CN.r2-CORE-PACKAGING-FIX-LOW-MEMORY-TEST-ONLY.zip)
- [修复记录与校验值](source/publication-docs/LEGACY-R2-PACKAGING-FIX.md)

5D3 对焦包已补上[正确的 `.123` 文件名入口](中文版魔灯/magiclantern.2025-06-20.5D3.123.zh-CN.experimental-r9.live-af-lv-gated-raw-compatible-r21-TEST-ONLY.zip)，对应佳能固件 **1.2.3**。旧 `.113` 命名文件与它内容完全相同，是文件名标记错误，不能据此用于佳能固件 1.1.3。

## 安装前必读

1. 在相机菜单中确认**准确机型**和**佳能官方固件版本**。
2. 在 [中文版魔灯](./中文版魔灯/) 中选择文件名完全匹配的安装包；不要凭相近机型或版本号猜测。
3. 先阅读包内说明和仓库中的机型提示，再解压到已备份的存储卡根目录。
4. 按 Magic Lantern 与相机对应版本的安装流程操作；安装、升级或降级佳能官方固件前，请先核对官方说明。
5. 第一次使用建议只启用必要模块，并在非重要拍摄任务中测试启动、菜单、录像和存储卡读写。

如果相机无法正常启动，请先取出存储卡，按对应机型的官方恢复与卸载说明处理；不要反复盲目刷写。

## 5D3 实验性视频框选自动对焦

5D3 的“实时框选自动对焦（实验）”仅面向 5D3 / 1.2.3 的对应测试包。它会在视频实时取景中依据佳能 AF 框的位置触发一次佳能原生对焦；录像与 RAW 视频兼容性仍在持续实机验证。

请在非重要拍摄中使用，避免与不熟悉的实验性模块同时启用。录像中出现异常、对焦失败、掉帧或停止录制时，请立即停止测试并提交信息反馈。

## 问题反馈

请通过 [Issues](https://github.com/ryulin456/magiclantern-zh/issues) 反馈，并尽量附上：

- 相机型号、佳能官方固件版本、所用汉化包完整文件名
- 已开启的 Magic Lantern 模块与复现步骤
- 屏幕照片、错误提示或日志（请先遮盖个人信息）
- 是否插卡启动、是否录像、是否使用 RAW 视频等关键信息

反馈成功机型同样很有价值：请告诉我机型、佳能固件版本和测试结果。

## 捐赠支持

如果这个项目帮到了你，欢迎前往 [捐赠支持页面](SUPPORT.md) 请我继续购买算力。完全自愿，感谢每一份支持。

## 致谢与许可

- 感谢 [Magic Lantern](https://www.magiclantern.fm/) 社区与所有测试、反馈贡献者。
- Magic Lantern 原项目及其衍生代码遵循 GNU GPL v2 许可；本项目不主张 Magic Lantern、Canon 或其商标的所有权。
