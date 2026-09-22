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

- 深度汉化主菜单、隐藏模块及模块内子选项
- 保留原有模块，不以删功能换取“可编译”
- 针对字体缺字、文字截断与显示笔画进行发布前检查
- 5D3 提供实验性的视频框选自动对焦模块
- 制作了5D4版本（纯理论LLM制造，适配版本1.3.3，风险高，不确定能否使用，欢迎反馈）
- 提供多机型汉化安装包，并持续收集实机反馈

## 测试状态

| 机型 | 当前状态 | 说明 |
| --- | --- | --- |
| Canon 5D Mark III | ✅ 已实机测试 | 汉化包与实验性框选自动对焦模块持续优化中 |
| Canon 5D Mark II | ✅ 已实机测试 | 基础汉化与模块加载已验证 |
| Canon 6D | ✅ 已实机测试 | 基础汉化与模块加载已验证 |
| Canon 6D Mark II | ✅ 已实机测试 | 基础汉化与模块加载已验证 |
| 仓库内其他机型 | 🚧 待社区反馈 | 请严格核对机型与佳能固件版本 |

“已实机测试”不代表适用于所有镜头、设置或使用场景；请从低风险场景开始验证。

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
