# 构建说明

这是汇总工作副本，先按 `publication-docs/FIRMWARE-SOURCE-MAP.md` 选择源码树。不要在公共树的 5D4 平台目录构建并声称得到独立开发 r1。

## 5D3 Live AF r21：可逐字节核对的独立模块

安装 xPack GNU Arm Embedded GCC **12.2.1-1.1**。PowerShell 从仓库根目录运行：

```powershell
& ./publication-docs/Build-AF-r21.ps1 -ArmBin 'D:/toolchains/xpack-arm-none-eabi-gcc-12.2.1-1.1/bin'
```

结果：`magiclantern-zh-src/modules/af_live/build/af_live.mo`。
目标 SHA-256：`81e00fcd8c430e5c9f6a2dc006a091bd86508f1a7346bcb8bcf2dd2f49f57e09`。
该脚本冻结历史 `module_strings.h`；常规 Makefile 会重新生成带日期的元数据。`VIDAF_LEGACY_R9` 必须保留。此模块不能独立替代 Magic Lantern 核心。

## 2025 系列公共工程

GNU Make、ARM none-eabi GNU 工具链、Python 3、Unix shell 工具；模块说明转换还需要 docutils（`rst2html`）。Windows 使用 MSYS2 风格 shell，并设置 `PYTHONUTF8=1`。原树的构建说明与 `Makefile.globals` 为参数依据。

```sh
cd magiclantern-zh-src/platform/6D.116
make ARM_BINPATH=/path/to/arm/bin READELF=/path/to/arm/bin/arm-none-eabi-readelf
```

替换为映射表中的目标目录，输出在相应 `build/`。不同编译器、版本日期和元数据会改变二进制。此处给出真实构建入口，不声称本次已完成全部核心的清洁构建。

## 7D、1200D 旧构建系统

使用独立树的根 `Makefile`、`Makefile.setup.toolchain` 与 `Makefile.user.default` 配置交叉编译工具链；先阅读这些文件的变量定义。其规则与新公共树不同。

```sh
cd work/src-lua-fix-zh
make 7D.203
```

1200D 使用 `work/src-1200D-zh` 和 `make 1200D.102`。7D 还有 `7D_MASTER.203`，按该工程原有双处理器构建规则处理，不要把 MASTER 核心当作主 `autoexec.bin`。

打包工具已修复，明确按平台选文件；资源默认取各自旧源码树，避免误用后来改动的公共字体。它支持未汉化原包，以及资源与所选源码树一致的既有汉化包。示例（从仓库根目录）：

```sh
python scripts/package_legacy_localized_release.py /path/to/old-7D.zip work/src-lua-fix-zh /path/to/new-7D-r2.zip 7D_203.sym lua_fix.2020Dec28.7D203 --platform 7D.203
python scripts/package_legacy_localized_release.py /path/to/old-1200D.zip work/src-1200D-zh /path/to/new-1200D-r2.zip t5_102.sym Nightly.2018Aug21.1200D102 --platform 1200D.102
```

当前此工具仅接受上述两个旧平台；其他平台使用其对应构建/打包流程。新 ZIP 生成后仍应运行符号依赖、字体及 ZIP 审计。

## 5D4 开发 r1

树：`magiclantern-5d4-zh-40c0a133-v2`。历史工具链为 xPack GNU Arm Embedded GCC **14.2.1-1.1**。

```sh
cd magiclantern-5d4-zh-40c0a133-v2/platform/5D4.133
make ARM_BINPATH=/path/to/arm/bin READELF=/path/to/arm/bin/arm-none-eabi-readelf
```

保留此树 Makefile 中的安全配置，不能用其他树同名模块覆盖。审计入口为 `tools/audit_5d4_release.py`，命令见归档审计报告。该包没有安装 FIR，也没有完整实机验证。

## 翻译与字体

已生成资源在各树 `data/fonts/` 和 `src/zh_cn*.h`；翻译维护脚本位于根 `scripts/`。`generate_zh_sources.py` 需要明确参考二进制、符号和重启地址，参数不可跨机型套用。`extend_zh_cn_bfnt.py` 依赖 Pillow 和相应 Noto Sans SC 字体文件。

`reproduction-metadata/` 保存部分历史 `version.c`、`module_strings.h`，仅供追溯或有针对性的复现，不自动覆盖新构建文件。
