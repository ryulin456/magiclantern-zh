# 5D3 Live AF r21 源码对应验证

验证日期：2026-10-18。

从本次导出的源码副本重新编译 `af_live.mo`，使用 xPack ARM GNU Toolchain 12.2.1-1.1 和原模块编译选项，保留历史生成的模块信息头文件。

对比对象：`magiclantern.2025-06-20.5D3.123.zh-CN.experimental-r9.live-af-lv-gated-raw-compatible-r21-TEST-ONLY.zip` 中的 `ML/modules/af_live.mo`。

重新编译结果与 r21 发布目录对应文件的 SHA-256 相同：

```text
81E00FCD8C430E5C9F6A2DC006A091BD86508F1A7346BCB8BCF2DD2F49F57E09
```

Windows / PowerShell 构建命令（从快照根目录运行）：

```powershell
& ./publication-docs/Build-AF-r21.ps1 -ArmBin 'D:/toolchains/xpack-arm-none-eabi-gcc-12.2.1-1.1/bin'
```

把工具链路径换成自己的安装位置。结果位于 `magiclantern-zh-src/modules/af_live/build/af_live.mo`。

常规 Makefile 的元数据生成流程另外依赖可调用的 `rst2html`（docutils）、Python 和 Unix shell 工具；Windows 中文环境还需要 `PYTHONUTF8=1`。若重新生成带日期的元数据，所得模块哈希可能不同。本脚本使用随包保留的 `r21-module_strings.h`，用于核对历史版本。

这项验证只证明 AF 模块源码与 r21 模块二进制相符，不证明 r21 整包内的核心和其他模块均可由当前源码重现，也不代表本次进行了相机或 RAW 录像实机测试。
