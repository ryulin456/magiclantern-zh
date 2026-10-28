# 5D3 四语言即时切换 r4

适用机型：Canon EOS 5D Mark III。对应佳能原厂固件：**1.2.3**。

维护者已在本次发布前报告 r4 实机测试正常。这里只确认本次报告的 5D3
测试结果；没有将其他型号、所有模块组合或所有拍摄场景标为已验证。
德语、波兰语仍需要母语校对。

## 下载与对应源码

- [完整测试包](../../中文版魔灯/magiclantern.2026-10-27.5D3.123.LANG4-LIVE-r4-TEST-ONLY.zip)
- [固定版本完整源码](https://github.com/ryulin456/magiclantern_simplified/tree/6f1bb16b853c8046a05c68f86f7ac02e4a727691)
- [完整源码 ZIP](https://github.com/ryulin456/magiclantern_simplified/archive/6f1bb16b853c8046a05c68f86f7ac02e4a727691.zip)
- [6 个后续提交的差异](https://github.com/ryulin456/magiclantern_simplified/compare/8809eccfb5f8d8ab71d19559c9efc88e5ed0610b...6f1bb16b853c8046a05c68f86f7ac02e4a727691)
- [构建命令与自动检查说明](https://github.com/ryulin456/magiclantern_simplified/blob/6f1bb16b853c8046a05c68f86f7ac02e4a727691/doc/LOCALIZATION-5D3-R4.md)

源码分支保留真实上游历史，包含本次核心、19 个随包模块的源码、可编辑 JSON
词库、语言编译工具、字体资源及新字形的字体输入和许可证。自动对焦修改在独立
分支，不混入本地化审阅。原有各机型包作为历史版本保留。

## 使用

先备份原卡，使用已经能够启动魔灯的 5D3／1.2.3 测试卡。将包内完整
`autoexec.bin` 和 `ML` 文件夹一起放到卡根目录，不要只换语言文件。
包内 19 个匹配模块默认关闭；按需启用时保留随包的 `5D3_123.sym`。
本包没有安装器，也没有旧 r21 自定义自动对焦改动。

默认英文。在魔灯 `Prefs → Language` 按 SET，立即循环切换：
English → Simplified Chinese → Deutsch → Polski → English。
正常关机保存选择。需要回退时恢复完整原卡备份，保持核心与模块配套。

## 本次修正及检查

新增德语、波兰语各 278 个词条，中文新增 281 个词条；另修订既有德语 109 条、
波兰语 102 条译文，包括词库中仍为英文的文字。修正选项拆分帮助、截短原文键、
模块信息值未翻译及网格 UTF-8 截取问题。

实际 ARM C 代码测试通过 6858 次词条查找、100 次语言选择、设置恢复和加载失败
回退；显示辅助函数通过 750 次截短键、231 次选项单行说明、195 次模块文字检查。
对编译后的核心与 19 个模块核对的 1733 个静态键，三个词库没有缺失。这不包括
所有动态消息或图片式手册页面，不能等同于相机全部状态均已翻译或测试。

ZIP 保持与维护者测试的文件完全一致，没有在测试之后重打包。ZIP 内“尚未实机
测试”的说明是生成时的记录；维护者随后确认实机测试正常。自动化报告中的
hardware_test:false 表示那次测试使用 ARM 模拟和卡 I/O 桩，不是实机测试报告。

## 校验

文件：`magiclantern.2026-10-27.5D3.123.LANG4-LIVE-r4-TEST-ONLY.zip`

SHA256：`01c79179014f3529aa643b886108ef1b7f92ab7e8064e2e7e7cdcdcd22d43fd2`

源码提交：`6f1bb16b853c8046a05c68f86f7ac02e4a727691`

