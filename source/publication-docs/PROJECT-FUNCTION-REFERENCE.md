# Magic Lantern（魔灯）功能说明书

版本：资料汇编 v1.0（2026-08-27）  
适用范围：本项目当前源码中的 Canon 相机平台目录与其 `modules.included` 列表。  
资料边界：功能描述只采用 Magic Lantern 英文源码/README、官方 Wiki、官方开发者文档和 magiclantern.fm 官方论坛专题；没有官方依据的内容不写成结论。

## 1. 重要说明

Magic Lantern 是从存储卡加载、与 Canon 原厂软件并行工作的开源固件扩展。它不是 Canon 官方产品；不同相机型号、原厂固件版本、卡片和模块组合的行为可能不同。任何“支持”都必须以对应平台目录中的实际文件和实机测试为准，不能把别的机型的经验直接套用。

安装和刷机前，必须使用与相机型号/原厂固件匹配的完整包，不要混用不同日期或不同平台的 `autoexec.bin`、模块和设置文件。相机冻结时应按官方安装说明处理：停止继续操作，必要时取下电池；不要在不清楚后果时反复断电或拔卡。旧版 Wiki 的安装页含有历史版本限制，本文只引用其安全处置原则，不把旧限制当作当前发行包的保证。

本文把三类事实分开：

* “源码/README 明确”——可以按文档复现，但仍受机型支持矩阵约束；
* “官方论坛报告”——是开发者或用户的实测经验，可能与版本、设置有关；
* “本项目静态检查”——只说明源码包中有什么，不能代替上机测试。

## 2. 基本使用方法

1. 将对应机型的完整发行包解压到卡根目录，保留官方包要求的启动文件和 `ML` 目录结构。
2. 开机后进入 Magic Lantern 菜单，在 Modules/模块页查看并加载需要的 `.mo` 文件；菜单名称会随平台和模块是否隐藏而变化。
3. 先加载依赖模块，再使用上层功能。例如 `file_man` 与 `pic_view` 配合浏览图片，`mlv_lite` 与 `mlv_play` 配合录制/回放；需要声音时使用该平台提供的 `mlv_snd`。同一功能若在包内同时存在旧模块和新模块，必须遵守该包的说明，论坛曾明确提醒不要同时加载 `mlv_lite` 和 `mlv_rec`。
4. 第一次启用实验模块时只改一个选项，先拍摄/录制小样本并确认能正常回放、关机和再次开机，再扩大使用范围。
5. 任何模块出现黑屏、红灯持续、无响应、文件损坏或无法启动，立即停止使用该模块，按相机官方安全方式退出；不要把失败配置复制到其他机型。

## 3. 当前源码中的发行模块

当前源码各平台 `modules.included` 的并集为 19 个模块。下面的功能、菜单和限制均来自对应英文 README、源码菜单字符串或官方专题；“可能问题”只列出资料明确提到的风险。

### 3.1 Advanced Intervalometer（`adv_int`，高级间隔拍摄）

**功能**：在间隔拍摄过程中，按关键帧对曝光参数（Av、Tv、ISO）、焦点、间隔周期以及部分版本支持的 Bulb/WB 参数做线性变化。

**使用方法**：先在间隔拍摄第 1 帧建立初始关键帧；进入 Shoot → Intervalometer → Advanced Intervalometer → New Keyframe，设置下一关键帧及其持续帧数，再开始拍摄。焦点渐变需在 LiveView 中操作，负值向近处、正值向无穷远；设置后要留出对焦和曝光稳定时间。序列可保存/加载到 `ML/SETTINGS/SEQ.TXT`。

**限制/风险**：不要让 AutoETTR 与高级间隔拍摄同时改变同一曝光参数；焦点、间隔和曝光变化是否可用取决于平台。官方论坛专题：[Advanced Intervalometer](https://www.magiclantern.fm/forum/index.php?topic=8431.0)。

### 3.2 Arkanoid（`arkanoid`）

**功能**：简化版 1980 年代砖块游戏，用于演示菜单/输入功能，不是摄影功能。

**使用方法**：进入模块菜单后启动，用方向键或摇杆控制挡板。

**限制/风险**：游戏运行时相机处于模块程序界面；不要把它当作稳定性测试。官方论坛专题：[Arkanoid](https://www.magiclantern.fm/forum/index.php?topic=7847.0)。

### 3.3 Auto Exposure（`autoexpo`）

**功能**：依据预设曲线自动选择曝光参数，源码 README 将其定义为类似“可编程 P”模式的自动曝光算法。

**使用方法**：在模块菜单中按需启用 `Use in LV`，然后配置 `Lens aperture`、`TV minimum`、`AV range/curve`、`ISO range/curve`、`EC` 和 `EC curve`；`Show graph`、`Browse` 用于查看曲线。

**限制/风险**：曲线和可调范围是算法参数，不代表相机测光结果在所有场景都正确；应先用短片段验证。本文没有把它描述成 Canon 原厂曝光模式。

### 3.4 Bench（`bench`，基准测试）

**功能**：测试存储卡、内存、缓存、部分 EDMAC/显示路径和菜单速度。菜单包括：Card Benchmarks（Quick R/W、CF+SD write、Buffer R/W、Buffer write）、Memory Benchmarks（Memcpy、Memory、Cache RAM、Cache RAM no cache、Cache ROM）以及 Focus peaking、Menu 等 Misc 测试。

**使用方法**：先确保电池电量充足、卡内无重要写入任务；在 Bench 中只运行一个测试，等待其正常结束，再查看结果。Quick R/W 可用于初步判断卡片是否可能成为 RAW 视频速度瓶颈。

**限制/风险**：这是诊断模块，不是普通拍摄功能。官方源码的内存测试会使用大块 DMA 缓冲区、关闭/控制中断并点亮信息灯；源码还包含固定的 RAM/ROM 地址和校验流程。因此不同机型不能互换测试假设。测试过程中不要拔卡、按快门或强行断电；出现黑屏、快门动作、红灯持续或卡无法访问时停止使用并记录机型、原厂固件、模块文件哈希和测试项。官方源码：[mem_bench.c](https://raw.githubusercontent.com/reticulatedpines/magiclantern_simplified/dev/modules/bench/mem_bench.c)、[mem_perf.c](https://raw.githubusercontent.com/reticulatedpines/magiclantern_simplified/dev/modules/bench/mem_perf.c)；论坛专题：[Bench](https://www.magiclantern.fm/forum/index.php?topic=12630.0)。

本项目另有一条用户实机报告：5D3 打开内存测试后曾出现黑屏、快门声和红灯持续。该现象与诊断测试对内存/DMA/指示灯的操作相符，但仅凭现象不能证明是汉化造成，也不能外推到所有 5D3 或其他机型；发布前应把对应模块、核心和原包做哈希对比，并在实机上逐项复测。

### 3.5 Crop Recording（`crop_rec`）

**功能**：通过传感器寄存器把 1080p/720p 视频改为约 3× 的 1:1 传感器裁切；Canon 的分辨率、H.264、RAW、音频、HDMI、预览和叠加层行为仍由平台决定。

**使用方法**：进入 Crop Rec，先选择 `Crop mode`，确认 `Shutter range` 与 `Target YRES`，再开始很短的测试录制；`Crop info` 只用于查看当前值。`Delta ADTG`、`Delta HEAD` 和 CMOS 参数是实验调节项。

**限制/风险**：README 明确警告，参数错误可能造成损坏帧、奇怪颜色或白条；若出现损坏帧应降低 `Target YRES`。官方论坛也有 5D3 裁切录制闪烁、文件异常的报告；不要照搬其他机型的寄存器值。专题：[Crop Rec](https://www.magiclantern.fm/forum/index.php?topic=17021.250)。

### 3.6 Deflicker（`deflick`）

**功能**：为后期去闪烁生成 XMP/UFRaw sidecar（旁车）文件，不在相机内直接重写原始照片。

**使用方法**：设置 `Sidecar file type`、`Deflicker percentile` 和 `Deflicker target level`，拍摄后把旁车文件交给支持相应格式的后期流程。

**限制/风险**：生成旁车不等于完成后期渲染；后期软件是否读取 XMP/UFRaw 由软件本身决定。专题：[Deflicker](https://www.magiclantern.fm/forum/index.php?topic=12585.0)。

### 3.7 Dot-Tune（`dot_tune`）

**功能**：自动进行镜头 AF 微调（AFMA）扫描；源码 README 说明另有配套视频说明。

**使用方法**：在稳定目标、固定机位和充足电量下选择 `Start Scan`，设置 `Scan type`、`Scan passes` 和 `AFMA mode`，扫描结束后检查结果再决定是否写入相机微调值。

**限制/风险**：扫描依赖镜头、目标、光线和平台 AF 支持；它不是保证所有镜头都能得到正确值的校准仪。专题：[Dot Tune](https://www.magiclantern.fm/forum/index.php?topic=4648.0)。

### 3.8 Dual ISO（`dual_iso`）

**功能**：交替使用两档 ISO 读取相邻行对，以提高动态范围；代价是垂直分辨率下降并可能增加锯齿、摩尔纹。支持 RAW 照片 CR2 和 RAW 视频 DNG，需用 `cr2hdr` 等工具后处理。

**使用方法**：常见官方流程是先用 Canon ISO 100，尽量向右曝光但避免高光溢出；在 Dual ISO 中启用 `Every other frame` 并选择备用 ISO；拍摄后用 `cr2hdr` 处理。菜单还包括 `Recovery ISO`、`Custom file prefix`，`Dynamic range gained` 和 `Midtone overlapping` 是只读信息。

**限制/风险**：相机内缩略图/直方图不能代表处理后的最终画面；论坛提醒备用 ISO 过高会增加伪影，建议先用小批量 RAW 验证。不要把“Every other frame”理解成隔一张照片拍摄，它指交替行读取。论坛：[Dual ISO 最佳实践](https://www.magiclantern.fm/forum/index.php?topic=9458.0)、[Every other frame 讨论](https://github.com/reticulatedpines/magiclantern_simplified/issues/257)。

### 3.9 EDMAC Tools（`edmac`，开发/调试模块）

**功能**：查看和记录 EDMAC 通道活动，查找空闲通道，并运行 EDMAC 模型假设测试；对开发者定位图像缓冲区、LiveView 通道问题有用。

**使用方法**：在 `EDMAC tools` 中选择 `Show EDMAC channels`（显示通道）、`Find free EDMAC channels`（查找空闲通道）、`Log EDMAC activity` → `Start logging`，并设置 `Log every` 采样间隔；`EDMAC model test` 用于运行假设测试。

**限制/风险**：这是调试模块，不应在不了解 DMA 的情况下修改或长时间记录；不同平台的通道、地址和权限不同，不能把日志解释成跨机型结论。

### 3.10 ETTR（`ettr`）

**功能**：Expose To The Right（向右曝光），在不裁切高光的前提下尽可能提高曝光，以降低阴影噪声。Auto ETTR 会自动调整曝光，使直方图接近右侧；后期通常需要压暗。

**使用方法**：先选择 `Trigger mode`，设置 `Slowest shutter`、`Exposure target`、`Highlight ignore`、`Allow clipping` 和明/暗部 SNR 限制；需要时打开 `Show metered areas`、`Allow beeps`、`Show debug info`。`Link to Canon shutter`、`Link to Dual ISO` 会改变联动方式。先用静态场景拍摄并检查高光，再用于正式拍摄。

**限制/风险**：ETTR 目标是 RAW 数据，不保证 JPEG 预览观感；移动主体、闪烁光源和高光容忍度设置会影响结果。官方 Wiki：[ETTR](https://wiki.magiclantern.fm/ettr)，论坛专题（由 README 链接）：[ETTR](https://www.magiclantern.fm/forum/index.php?topic=5693.0)。

### 3.11 File Manager（`file_man`）

**功能**：浏览卡上文件系统，显示名称/大小/日期；按文件或扩展名选择，复制、移动、删除和查看文本，并为图片查看器等模块提供文件入口。

**使用方法**：进入 File Manager，选择目录或文件后使用模块菜单提供的操作；先确认路径和文件名，再执行复制、移动或删除。与 `pic_view` 配合可查看支持的图片格式，与 `mlv_play` 配合可选择 MLV/RAW 文件。

**限制/风险**：删除和移动是实际写卡操作；文件操作期间不要拔卡或断电。论坛专题：[File Manager](https://www.magiclantern.fm/forum/index.php?topic=5522.0)。

### 3.12 Image Name（`img_name`）

**功能**：自定义照片文件名前缀、文件编号和文件夹编号。

**使用方法**：在 `Image file naming` 中设置 `Image file prefix`、`Image file number`、`Image folder number`；修改文件编号后按 README 要求重启相机使其生效。

**限制/风险**：编号规则仍受 Canon 文件系统和文件夹容量限制；不要设置会与现有文件冲突的编号。专题：[Image naming](https://www.magiclantern.fm/forum/index.php?topic=21111.0)。

### 3.13 Lua（`lua`）

**功能**：从 `ML/SCRIPTS` 运行 Lua 脚本；脚本可在启动时自动执行，主体通常负责创建菜单和行为。全局状态会持续到关机。

**使用方法**：把脚本放入 `ML/SCRIPTS`，在 Lua 菜单中选择 `Run Script`；可用 `Edit Script`、`Autorun`、`Show console` 调试。API 文档地址由 README 给出：[ml-lua API](http://davidmilligan.github.io/ml-lua/)。

**限制/风险**：脚本是可执行代码，来源不明的脚本可能改变相机状态或导致冻结；先阅读源码并单独测试。不同平台的 Lua API 能力可能不同。专题：[Lua](https://www.magiclantern.fm/forum/index.php?topic=14828.0)。

### 3.14 MLV Lite（`mlv_lite`）

**功能**：以 MLV 容器录制 14-bit RAW 视频；README 将其定位为替代旧 raw_rec 的精简实现，性能目标与 raw_rec v1.0 相当。可配合 `mlv_play` 回放，声音支持取决于平台是否提供 `mlv_snd`。

**使用方法**：在对应平台启用 `mlv_lite`，先低分辨率/短时录制测试卡片写入速度、停止录制和文件完整性；用 `mlv_play` 或电脑端工具检查 MLV。官方论坛曾说明该模块在 nightly 中以 `mlv_lite` 名称提供，并提醒不要与 `mlv_rec` 同时加载。

**限制/风险**：RAW 写入对卡片速度和相机缓冲压力高；掉电、拔卡或写入速度不足会损坏当前文件。论坛：[MLV Lite](https://www.magiclantern.fm/forum/index.php?topic=16650.100)、[6D 音频/模块组合讨论](https://www.magiclantern.fm/forum/index.php?topic=15088.775)。

### 3.15 MLV Player（`mlv_play`）

**功能**：在相机上播放 MLV/RAW。可从 File Manager 选择，也可在 LiveView 中对最近一次录制使用 PLAY。

**使用方法**：选择文件后，`all` 播放每一帧，`exact` 按录制帧率丢弃多余帧；SET 打开菜单，PLAY 暂停，INFO 查看信息，拨轮切换前后文件。

**限制/风险**：回放能力取决于文件格式、分辨率、卡片和平台；不能把“能回放”当成文件一定无损。录制模块和回放模块必须使用同一包的兼容版本。专题：[MLV Player](https://www.magiclantern.fm/forum/index.php?topic=9062.0)。

### 3.16 MLV Sound（`mlv_snd`）

**功能**：为 MLV RAW 录制扩展声音记录支持，源码 README 将其描述为扩展 mlv_rec 的声音模块。

**使用方法**：只在该平台的 `modules.included` 中存在并且包说明允许时启用；先做短录音，再在电脑端检查音视频是否同时写入。论坛对 6D 的建议是使用该平台提供的 `mlv_lite/mlv_snd` 组合，并避免加载冲突模块。

**限制/风险**：并非所有平台都提供 `mlv_snd`；声音、采样率和同步表现不能跨机型推断。专题：[6D 音频/模块组合讨论](https://www.magiclantern.fm/forum/index.php?topic=15088.775)。

### 3.17 Picture Viewer（`pic_view`）

**功能**：与 File Manager 集成，查看 PPM、DNG、BMP（未压缩/RLE）、BMH、422 等格式。

**使用方法**：在 File Manager 中选择支持的图片文件，调用 Picture Viewer；使用相机播放/信息控制查看或退出。

**限制/风险**：支持列表是源码 README 的格式列表，不代表所有分辨率和编码都能流畅显示；未知或损坏文件不要反复打开。与文件管理器的实际集成取决于两个模块都已加载。

### 3.18 Selftest（`selftest`，自检/开发测试）

**功能**：提供 stubs、线程安全、RPC、菜单后端、重绘、矩形、信息屏、SRM 内存、EDMAC 屏幕、空指针和故障模拟等测试，另有约 15 分钟的 Quick test、无限循环 Random tests 等。

**使用方法**：只在开发、排障或得到明确测试方案时运行；先保存设置和取出重要照片。选择单个测试，阅读菜单中显示的持续时间/“infinite”提示，测试结束后记录结果。

**限制/风险**：部分测试故意冻结 GUI、锁死 ARM CPU、除零、分配 1MB/10MB 或无限循环，普通用户不要尝试；失败时不要把结果解释为“汉化导致”。如果相机无响应，按安全处置原则停止并取下电池。

### 3.19 Silent Picture（`silent`）

**功能**：在 LiveView 中尽量不驱动机械快门拍照，输出低分辨率 14-bit DNG；模式包括 Simple、Burst、End Trigger、Best Shots、Slit-Scan。

**使用方法**：设置 `Silent Picture`、`Silent Mode`、`Slit-Scan Mode`、`Trigger Mode` 和 `File Format`，先使用 Simple 做单张测试，再尝试 Burst/Slit-Scan；用 `pic_view`/`file_man` 检查文件。

**限制/风险**：分辨率、读出方式、滚动快门和可用模式受平台/LiveView 状态限制；它不等同于 Canon 的电子快门，也不保证所有场景无声或无畸变。专题：[Silent Picture](https://www.magiclantern.fm/forum/index.php?topic=5240.0)。

## 4. 本源码中的机型与模块配置

下面是本项目当前源码各平台目录的**精确 `modules.included` 分组**。它是静态可用性清单，不是实机兼容性保证。目录名中的数字同时包含相机型号和固件版本，例如 `5D3.123`。

| 配置组 | 平台目录 | 包含模块 |
|---|---|---|
| A（17） | `50D.109` | adv_int、arkanoid、autoexpo、bench、deflick、dot_tune、dual_iso、edmac、ettr、file_man、img_name、lua、mlv_lite、mlv_play、pic_view、selftest、silent |
| B（3） | `5D4.133`、`6D2.111`、`750D.110` | bench、dual_iso、file_man |
| C（19） | `5D2.212`、`5D3.113`、`5D3.123`、`6D.116`、`70D.112`、`7D.203`、`7D2.112`、`XF605.101` | adv_int、arkanoid、autoexpo、bench、crop_rec、deflick、dot_tune、dual_iso、edmac、ettr、file_man、img_name、lua、mlv_lite、mlv_play、mlv_snd、pic_view、selftest、silent |
| D（18） | `500D.111`、`550D.109`、`600D.102`、`60D.111` | adv_int、arkanoid、autoexpo、bench、deflick、dual_iso、edmac、ettr、file_man、img_name、lua、mlv_lite、mlv_play、mlv_snd、pic_view、selftest、silent |
| E（2） | `5DSR.112`、`77D.110`、`80D.103`、`850D.100`、`M50.110`、`R.180`、`R5.152`、`R6.150`、`RP.160`、`SX70.111`、`SX740.102` | bench、file_man |
| F（4） | `200D.101` | bench、dual_iso、file_man、mlv_lite |
| G（18） | `100D.101`、`650D.104`、`700D.115`、`EOSM.202` | adv_int、arkanoid、autoexpo、bench、crop_rec、deflick、dual_iso、edmac、ettr、file_man、img_name、lua、mlv_lite、mlv_play、mlv_snd、pic_view、selftest、silent |
| H（13） | `1100D.105` | adv_int、arkanoid、autoexpo、bench、deflick、dual_iso、ettr、file_man、img_name、lua、mlv_play、mlv_snd、pic_view |

源码还包含 `all`、`unmaintained` 等非具体机型目录；它们不应被当作可刷入的相机型号。`modules.hidden`/`.hid` 只影响菜单显示或实验策略，不改变本文对“包内是否存在模块”的判定：实际刷入前必须检查目标 ZIP 的 `ML/modules` 和平台清单。

`7D_MASTER.203` 是 7D 的构建辅助平台目录，当前没有独立的 `modules.included` 文件；不能把它误列为一套额外模块配置，实际模块以生成的目标 ZIP 清单为准。

**5D4 特别说明**：`5D4.133/modules.included` 中的 `bench`、`dual_iso`、`file_man` 只是该源码版本的构建清单，不等于三项功能均已在 5D4 实机验证。`modules.hidden` 是菜单隐藏标记清单，不是模块二进制包含清单；不得因为名称出现在其中，就从 5D3 或其他机型复制对应 `.mo`、地址、stubs 或功能实现。

## 5. 源码中存在、但不在当前 19 个发行模块并集中的模块

这些目录有英文 README 或源码，但当前平台清单没有把它们列入所有机型的通用发行模块；它们不能被本说明书写成“当前所有机型均可用”。

| 模块 | README/源码明确用途 | 发布状态说明 |
|---|---|---|
| `bolt_rec` | 检测闪电并触发视频开始 | 实验模块；未列入当前通用平台清单 |
| `bulb_nd` | 为长时间曝光/ND 使用提供辅助 | 实验/平台相关；未列入当前通用平台清单 |
| `fpu_emu` | 为不带硬件 VFP 的 FPA 提供软件浮点仿真，便于 TCC 脚本使用浮点 | 开发用途；未列入当前通用平台清单 |
| `io_crypt` | 对文件 I/O 做专有加密钩子；README 提到 5D3，可能还有 600D/7D | 明确是平台/用途特定，不可泛化 |
| `plot` | 图形 API，主要用于调试/绘图 | 开发组件，不是通用用户模块 |
| `raw_twk` | 实验性的 MLV/RAW 回放加速；README 明确警告可能出现任何结果 | 高风险实验，不纳入发行保证 |
| `script` | 脚本 GUI，README 标注为 WIP | 未完成开发组件 |
| `tcc`、`tinypy`、`yolo` | 脚本/实验运行环境或测试组件 | 仅按源码状态处理，不视为机型支持 |
| `mlv_rec` | 旧的 RAW/MLV 录制实现 | 当前发行配置使用 `mlv_lite`；不要与 `mlv_lite` 混载 |
| `modules/dev_tools/*` | ADTG、EDMAC、内存、日志、MPU 等底层调试工具 | 开发者工具，平台相关且可能造成冻结/异常 |
| `modules/input/ime_*` | 输入法后端（标准、旋转、空实现等） | 内部组件，不是用户菜单功能 |

## 6. 发布前的事实核对清单

每一个汉化包在发布前都应完成并留存以下证据：

* 用目标 ZIP 的文件列表核对相机型号、原厂固件版本、`autoexec.bin`、`ML/modules` 和 `modules.included`；不从其他机型复制缺失模块。
* 对 19 个发行模块逐一检查：主菜单名、模块介绍、隐藏模块名、模块内子选项、帮助文本和单位是否都有翻译；不可见/调试模块也要翻译，除非明确在发布说明中说明“未提供该模块”。
* 对每条中文菜单做屏幕宽度检查：介绍文字保持在一行或按源码允许的换行；中文字符、标点、单位和数值必须完整显示，不得出现缺笔画、方框、截断或覆盖相邻菜单。
* 静态检查模块依赖和符号：每个 `.mo` 必须来自同一平台、同一源码/构建批次；不能因“能加载”就推断功能可用。
* 逐模块做最小风险测试：开机/关机、菜单打开、选项读写、返回；录制/文件操作/RAW/自检等高风险功能只做短时、可恢复测试。
* 记录异常：机型、原厂固件、包名、模块 SHA-256、菜单路径、操作步骤、LED/快门/黑屏现象和是否能取电池恢复。没有这些信息，不得把“无问题”写进发布说明。
* 发布包的构建版本号必须显示原始安装包名称（例如 `magiclantern.2025-06-20.6D.116`），不能用汉化日期覆盖原始版本标识；汉化修订号另放在发布说明中。
* **无原始发行包时的版本规则**：若上游没有发布对应机型的原始安装 ZIP，不得虚构日期包名或冒充官方发行版。菜单版本号应显示 `dev-<源码提交>.<平台>`，完整源码提交、工具链和汉化修订号写入清单；ZIP 文件名必须含 `DEVELOPER-BOOT-TEST` 或 `REALCAM-TEST`。
* **持久资源内存规则**：字体、语言表、索引等会在函数返回后继续使用的资源，不得保留 `read_entire_file()`、`fio_malloc()`、`tmp_malloc()` 或 `shoot_malloc` 返回的缓冲区。必须使用经过该平台验证的单一持久内存池，并禁止分配失败后回退到拍摄/SRM/DMA 池；文件读取可分块使用单一 DMA 池，但每块读完即释放，最终驻留缓冲区必须由与其分配器严格配对的释放函数处理。代码审计必须证明短读、损坏头、分配失败和所有错误路径都会关闭文件并释放对应内存。
* **旧卡隔离规则**：发布包不得携带任何默认启用模块的 `.en` 文件。升级测试不得直接覆盖旧 `ML` 目录；必须先备份旧目录，再用相机格式化或在电脑上移走整个旧 `ML` 目录后复制新包。旧 `ML/SETTINGS/*.en`、模块配置和崩溃锁文件不得进入首次启动测试。
* **三段式启动验收**：先确认无卡可正常进入佳能菜单；再用干净卡测试对应机型/固件的原英文包；最后用干净卡测试汉化核心，且首次启动不启用任何模块。只有三段均通过，才允许开始逐个模块测试。
* **冷启动与佳能菜单验收**：至少完成 10 次关机、取电池后冷启动；分别检查 M/P/Tv/Av、实时取景、回放以及佳能菜单中的固件版本项。黑屏、持续红灯、异常快门动作、菜单项消失或必须无卡才能启动，任一出现即判定失败并停止分发。
* **模块逐项启用规则**：包内应保留平台清单要求的全部模块和隐藏模块，但首次基线测试不得自动加载它们。之后一次只启用一个模块，重启并验证菜单、子选项、拍摄/回放、正常关机和再次开机；再测试资料明确要求的依赖组合。不得用“19 个模块均能链接”代替实机功能验收。
* **发布等级**：仅完成电脑端构建、校验、符号、布局和资源审计的包只能标记为“静态检查通过 / 实机测试包”；完成目标机型基线启动后可标记“启动实测通过”；完成逐模块测试后才可标记“该机型实机验证通过”。不得把“可构建”“可链接”或其他机型通过写成该机型“测试无误”。
* **事故处置与撤回**：收到可复现的黑屏、持续红灯、无卡才能开机或佳能菜单异常报告时，立即停止分发对应修订版并保留原包 SHA-256。修复版必须使用新修订号，不能覆盖旧文件；发布说明要列出故障包哈希、修复点和仍待实机确认的项目。

### 6.1 70D 黑屏事件加入的强制检查

`magiclantern.2025-06-20.70D.112.zh-CN.r2-FULL-MODULES-TEST-ONLY.zip` 已收到“装卡黑屏、红灯、原厂菜单异常”的实机报告，因此不得继续作为可用版本分发。静态复核确认它的机型/固件标识、核心 XOR、安装 FIR、19 个模块及符号依赖没有发现错配；同时发现中文字体和语言表曾通过临时/DMA 分配器读入后长期保留。Magic Lantern 内存代码明确说明该临时拍摄内存在进入佳能菜单时应保持空闲，因此这是与现象一致的具体缺陷，但修复效果仍必须由 70D 1.1.2 实机确认。

70D 修复版除执行本节全部通用规则外，还必须留存以下证据：中文字体和语言表不再长期占用临时/DMA/`shoot_malloc` 池；`.bss` 未越过平台固定预留末端；安装包没有 `.en` 设置；19 个模块全部保留且符号可解析；用全新卡完成原英文包与汉化包的对照冷启动。未取得这些实机证据前，文件名和说明必须包含 `REALCAM-TEST` 或同等明确标记。

### 6.2 5D Mark IV 1.3.3 专项发布门槛

以下是本项目的风险控制门槛，不代表 Magic Lantern 官方已经把 5D4 列为正式稳定机型。

* 只允许从 `platform/5D4.133` 为 Canon 1.3.3 构建。当前平台没有启用 `CANON_FIR`，也没有 5D4 安装器或 `ML-SETUP.FIR`，因此只能发布为“开发者存储卡启动测试包”；严禁复制或改名使用 5D3、其他机型的 FIR、`autoexec.bin`、平台地址、stubs 或模块。
* 该包只适用于已经通过独立、可核验的开发流程准备好 bootflag 和可启动存储卡、并了解恢复方法的测试者。汉化项目不得提供创建或修改 5D4 bootflag、格式化启动卡或改写启动扇区的工具和步骤；不得把它写成普通用户可直接安装的完整固件包。
* 必须保存源码提交、构建命令、工具链版本、完整文件清单及发布 ZIP 的 SHA-256。包内不得包含 `.en`、旧 `ML/SETTINGS`、崩溃锁文件或其他机型遗留配置。
* 必须保持 `CONFIG_PROP_REQUEST_CHANGE`、`CONFIG_AUTOBACKUP_ROM` 和图像捕获相关危险功能的上游禁用状态；属性写入白名单为空时不得绕过。未经 5D4 反汇编与实机验证，不得启用视频录制、图像捕获、SRM、MVR、YUV、EDMAC 或从其他机型复制的常量。
* `bench` 在 5D4 上必须通过编译期专用安全分支排除内存、缓存、ROM、EDMAC、显示和存储卡测试实现，只保留不可解除的说明菜单；不能仅依赖启动后才判断机型或固件的运行时锁。二进制符号审计必须确认危险测试依赖没有进入 `.mo`。每项测试只有在地址、分配器、缓存行为和恢复流程分别通过实机验证后才能单独开放。
* 当前 `dual_iso` 没有 5D4 专用 CMOS ISO 地址，只能构建为无传感器回调、无功能菜单且初始化拒绝加载的元数据占位模块。原上游源码和模块清单应保留以便后续开发，但“保留源码/占位模块”不等于功能已保留、已经汉化或可在 5D4 使用，发布说明必须逐项披露。
* `file_man` 的首轮 5D4 包必须通过编译期专用只读分支排除复制、移动、删除、多选写入、目标目录写入和第三方查看器回调，只允许浏览和内置只读查看；不能只依赖运行时机型/固件判断。二进制符号审计必须确认写卡入口和写文件依赖没有进入 `.mo`。写卡功能必须在启动、关机和只读测试全部通过后另行逐项验收。
* 5D4 使用 `CONFIG_NO_BFNT`，中文外部字体属于关键启动资源。核心必须内嵌可读的 ASCII 应急字体并在读取存储卡前初始化全部字体槽；外部 RBF/BFN 和语言表必须验证魔数、头大小、偏移、乘加溢出、排序、重复项、UTF-8 合法性、字形尺寸及文件边界。缺失或损坏时必须安全回退，不能黑屏、越界访问或占用拍摄/SRM/DMA 池；必须保存截断、多字节 UTF-8、损坏头和越界偏移的电脑端负向测试结果。
* 模块说明生成链必须固定使用 UTF-8，禁止把电脑用户名、主机名或绝对路径写入模块元数据。发布前应从实际生成的 `module_strings.h` 和可见中文字符串重新统计字形覆盖，不得只检查翻译源文件；出现乱码、缺字、方框、缺笔画或被截断即判失败。
* 静态验收至少包括：目标机型和固件标识、ARM 架构、链接符号、模块批次、`.bss`/重定位边界、语言表有序性、字形覆盖、中文宽度与笔画、ZIP 文件清单，以及确认包内不存在伪造的安装 FIR。
* 实机验收必须使用 5D4 1.3.3、满电电池和无重要数据的专用卡，并记录使用的卡槽。依次完成：无卡进入 Canon 菜单、同一派生源码树与同一工具链生成且只去掉汉化改动的英文安全对照构建、全部模块关闭的汉化构建。若缺少这一英文对照，不得把异常归因于汉化或声称汉化核心通过。每阶段检查 Canon 固件版本项、实时取景、回放、菜单、正常关机和再次启动。
* 汉化核心至少完成 10 次取电池冷启动；随后只测试 `file_man` 的只读功能。`bench` 应只能看到安全锁提示，`dual_iso` 不应提供可执行功能。不得通过拍摄、录像或属性写入来“试试看”尚未验证的地址。
* 只完成电脑端检查时，文件名和说明必须标为“静态检查通过／开发启动测试包”；完成一台或多台机身测试也只能逐项写明已经实际操作的功能、机身、卡槽和包哈希，不能据设备数量笼统推断“可用”。任何阶段都不得称为“稳定版”“全功能版”或“功能接近 5D3”，除非相应全部功能已经逐项取得可复核证据。
* 出现装卡黑屏、持续红灯、异常快门动作、Canon 固件版本项消失或只能无卡启动时，应立即关机，取出 CF、SD 卡和电池，等待后无卡启动；不得连续反复开机，也不得在故障状态下尝试改 bootflag、刷入其他机型 FIR 或执行基准测试。
* 故障卡应停止写入并保留现场，收集包名、SHA-256、源码提交、原厂固件、卡槽、模块状态、`ML/LOGS`、`ML/SETTINGS` 和完整复现步骤。若无卡仍不能正常启动，应停止所有测试并交由熟悉 5D4 启动链或专业维修的人员处理。
* 收到上述任一事故报告后立即撤回对应修订版。修复包必须使用新修订号，并明确列出故障包哈希、修复点和仍未完成的实机项目。

## 7. 官方资料索引

* 项目源码与发行说明：[magiclantern_simplified](https://github.com/reticulatedpines/magiclantern_simplified)
* 5D4 1.3.3 平台源码：[platform/5D4.133](https://github.com/reticulatedpines/magiclantern_simplified/tree/dev/platform/5D4.133)
* 5D4 平台构建定义：[Makefile](https://raw.githubusercontent.com/reticulatedpines/magiclantern_simplified/dev/platform/5D4.133/Makefile)
* 5D4 功能与内存声明：[features.h](https://raw.githubusercontent.com/reticulatedpines/magiclantern_simplified/dev/platform/5D4.133/features.h)、[internals.h](https://raw.githubusercontent.com/reticulatedpines/magiclantern_simplified/dev/platform/5D4.133/internals.h)
* 5D4 模块清单：[modules.included](https://raw.githubusercontent.com/reticulatedpines/magiclantern_simplified/dev/platform/5D4.133/modules.included)、[modules.hidden](https://raw.githubusercontent.com/reticulatedpines/magiclantern_simplified/dev/platform/5D4.133/modules.hidden)
* 5D4 地址与属性风险：[consts.h](https://raw.githubusercontent.com/reticulatedpines/magiclantern_simplified/dev/platform/5D4.133/consts.h)、[property_whitelist.h](https://raw.githubusercontent.com/reticulatedpines/magiclantern_simplified/dev/platform/5D4.133/property_whitelist.h)
* 上游发行页（当前无 5D4 正式发行）：[Releases](https://github.com/reticulatedpines/magiclantern_simplified/releases)
* 官方构建指南：[Building Magic Lantern](https://github.com/reticulatedpines/magiclantern_simplified/blob/dev/developer_guide/04_00_building_ML.md)
* 官方 ETTR Wiki：[ETTR](https://wiki.magiclantern.fm/ettr)
* 官方安装安全说明（旧版 Wiki，需结合当前包判断）：[Installation](https://wiki.magiclantern.fm/es%3Ainstall)
* 官方用户指南（旧版 Wiki）：[User guide](https://wiki.magiclantern.fm/es%3Auserguide)
* 官方论坛首页及各模块专题：本文各节链接均指向 `magiclantern.fm/forum` 原文；论坛内容可能随版本变化，引用时应保留主题号和访问日期。

本文没有把论坛中的单个用户报告升级为普遍结论，也没有把源码中存在但未列入平台清单的实验组件包装成“所有机型功能”。
