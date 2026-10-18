# 5D Mark IV 1.3.3 简体中文开发构建 r1 静态审计

审计日期：2026-09-22（Asia/Shanghai）  
版本：`dev-40c0a133-derived.5D4.133.zh-CN.r1`  
构建标识：`40c0a133-derived+5D4-safe+zh-r1`  
目标：Canon EOS 5D Mark IV，佳能固件 1.3.3

## 结论

本构建完成电脑端编译、链接、资源和 ZIP 静态审计，只能称为“开发者存储卡启动测试包”。它没有经过 5D4 实机启动、菜单、拍摄、回放或关机测试，不能保证不会黑屏、持续亮红灯、触发异常快门动作或影响佳能菜单，也不能称为可用版、稳定版、全功能版或接近 5D3 功能。

## 源码与工具链

* 参考上游提交：`40c0a133b15ea4dfa0139578c0b2791fef67f6d7`。
* 本地基础提交：`1d4bc8bce27dbbce04a67b6c166a500eaa3ff875`；这是选择性派生树，不是上游提交的逐字节检出。
* 编译器：xPack GNU Arm Embedded GCC 14.2.1（20241119）。
* 目标 ELF：32 位 little-endian ARM，入口 `0x800000`。
* 核心尺寸：text 190586、data 31489、bss 46848 字节；链接得到 `_bss_end=0x0020df00`。
* 链接器仍报告核心和 autoexec 的 RWX LOAD 段警告；在取得上游布局解释和实机证据前，此项保留为待确认风险。

## 已通过的电脑端检查

* 核心与全部默认模块从源码完成编译；最终包只收录 5D4 上游清单中的 `bench.mo`、`dual_iso.mo` 和 `file_man.mo`。
* `bench.mo` 只剩菜单接口依赖，不含内存、缓存、ROM、EDMAC、显示或存储卡基准测试依赖。
* `dual_iso.mo` 是无外部依赖、无传感器回调的不可用占位模块；原实现源码仍保留，但没有装入该二进制。
* `file_man.mo` 保留目录和内置文本读取；最终未解析符号不含 `FIO_CopyFile`、`FIO_MoveFile`、`FIO_RemoveFile`、`FIO_WriteFile` 或 `FIO_CreateFile`。
* ZIP 共 66 项，无重复路径、路径穿越、CRC 错误、FIR、`.en`、旧 `ML/SETTINGS` 或额外 `.mo`。
* 中文语言表含 2052 项，哈希严格递增且唯一，所有偏移、终止符和 UTF-8 均合法。
* 中文字体含 934 个字形，ASCII 0x20–0x7e 完整；语言表、实际生成的模块说明和可见中文源码字符串无缺失字形。
* 外部字体校验拒绝了截断头、错误魔数、零宽度、未对齐字符表、超大字符表、超大位图和未终止名称七类损坏样本。
* 包内三个显眼位置均使用同一份 5D4 开发风险说明；没有普通 FIR 安装说明。
* `git diff --check` 通过。

复核命令：

```text
python tools/audit_5d4_release.py <zip> --nm <arm-none-eabi-nm>
```

## 安全限制

* 包内没有 5D4 安装 FIR，不支持从佳能“固件升级”菜单安装。
* 本项目不提供创建/修改 bootflag、格式化启动卡或改写启动扇区的方法。
* 截图、内存浏览、ROM/RAM/图像缓冲区导出和拍摄内存余量探测在 5D4 上关闭。
* 中文字体和语言表使用单一持久池；分块读取使用单一 DMA 池，不回退到拍摄/SRM 池。
* 外部字体损坏时保留内嵌 ASCII 应急字体；语言表损坏时回退英文菜单。
* 包内没有自动启用模块的 `.en` 文件；首次实机测试仍必须使用干净卡和空设置目录。

## 尚未解决、不得掩盖的风险

* 上游 5D4 端口本身仍是开发状态，存在未完成或借用的常量、空实现和未经本项目 ROM 核验的接口。
* `take_semaphore=0x2c7`、`take_semaphore_now=0x297` 来自同代平台模式推断，不是 5D4 1.3.3 ROM 反汇编确认结果。
* 尚未生成同一派生树、同一工具链、仅去除汉化改动的英文安全对照包。
* 尚未验证 CF/SD 卡槽差异、启动链、佳能固件版本菜单、实时取景、回放、关机、冷启动或字体实际像素显示。
* 保留隐藏模块标记和源码不代表这些模块已经能在 5D4 加载或工作；本 r1 只有上述三个 `.mo`，其中 `bench` 被安全锁定，`dual_iso` 明确不可用，`file_man` 仅为只读候选。

发现装卡黑屏、持续红灯、异常快门动作、佳能固件版本项消失或只能无卡启动时，应立即停止测试，取出两张卡和电池；不要反复开机，不要尝试其他机型 FIR，也不要执行任何基准测试。

## 参考

* 上游固定源码：<https://github.com/reticulatedpines/magiclantern_simplified/tree/40c0a133b15ea4dfa0139578c0b2791fef67f6d7/platform/5D4.133>
* 5D4 开发讨论：<https://www.magiclantern.fm/forum/index.php?topic=17695.250>
* 5D4 overlay 开发问题：<https://github.com/reticulatedpines/magiclantern_simplified/issues/30>
