# FH8626V300 RT-Thread SDK 开发指南

## 项目概述

- **芯片平台**: FH8626V300
- **操作系统**: RT-Thread
- **主要用途**: 视频相关开发（UVC、ISP、VPSS等）

## 项目结构

```
rt-thread/
├── app/media_demo/           # 应用程序
│   ├── sample_common/        # 公共模块 (DSP/ISP/UVC)
│   ├── sample_demo/          # Demo示例
│   │   └── uvc_rtt/         # UVC示例
│   ├── components/           # 组件 (USB/UVC)
│   │   └── libusb_rtt/      # USB库
│   └── startup/              # 启动入口
├── lib/fh8626v300/          # SDK库文件
│   └── inc/                 # 头文件
├── drivers/                  # 驱动
└── docs_tools/              # 开发文档
```

## 视频数据流

```
Sensor → VICAP → ISP → VPSS → [UVC / Encoder / MJPEG]
              ↓
           (硬件绑定)
```

- **VICAP**: 视频输入捕获
- **ISP**: 图像信号处理（曝光、白平衡、增益等）
- **VPSS**: 视频处理子系统（裁剪、缩放、旋转）
- **UVC**: USB Video Class输出

## 关键源文件路径

| 功能 | 文件路径 |
|------|----------|
| UVC回调实现 | `app/media_demo/components/libusb_rtt/usb_video/src/uvc_callback.c` |
| UVC配置头文件 | `lib/inc/bsp/uvc_init.h` |
| UVC功能配置 | `app/media_demo/sample_demo/uvc_rtt/include/uvc_feature_config.h` |
| VPSS配置 | `app/media_demo/sample_common/FH8626V300/dsp/vpu/src/vpu_config.c` |
| VPSS通道配置 | `app/media_demo/sample_common/FH8626V300/dsp/vpu/src/vpu_chn_config.c` |
| USB Video类 | `app/media_demo/components/libusb_rtt/usb_video/src/usb_video.c` |
| USB控制 | `drivers/usb/device/class/libuvc_ctrl.c` |

## 代码规范

### 功能开关规范 (强制要求)

**每个新功能都必须用单独的宏控制开关**，确保关闭功能时不影响之前的功能。

格式要求：

```c
// 1. 功能配置头文件: uvc_feature_config.h
#define UVC_ENABLE_XXX  1  // 开启
#define UVC_ENABLE_XXX  0  // 关闭

// 2. 代码中使用 - 条件编译
#if UVC_ENABLE_XXX
    // 功能代码
#endif

// 3. 回调函数注册时也需要判断
#if UVC_ENABLE_XXX
    .SetGammaCfg = Uvc_SetGammaCfg,
#endif
```

**规范要点：**
- 每个功能必须有独立的宏开关
- 宏定义统一放在 `uvc_feature_config.h` 中
- 功能代码使用 `#if xxx` 包裹
- 回调函数注册时也要条件编译
- 关闭功能后代码必须能正常编译运行

### 已实现的宏控制功能

| 宏名称 | 功能 |
|--------|------|
| `UVC_ENABLE_CROSSHAIR` | 中心十字线 |
| `UVC_ENABLE_TOSD` | 时间戳OSD显示 |
| `UVC_ENABLE_LOGO` | LOGO OSD显示 |
| `UVC_ENABLE_GBOX` | GBOX矩形框 |
| `UVC_ENABLE_SD_RECORD` | SD卡录像 |
| `UVC_ENABLE_GAMMA` | Gamma控制 |
| `UVC_ENABLE_BACKLIGHT` | 逆光补偿 |
| `UVC_ENABLE_HUE` | 色调控制 |
| `UVC_ENABLE_POWER_LINE` | 电力线频率 |
| `UVC_ENABLE_AE_PRIORITY` | 自动曝光优先级 |
| `UVC_ENABLE_FOCUS` | 聚焦控制 |
| `UVC_ENABLE_ZOOM` | 缩放控制 |
| `UVC_ENABLE_MIRROR_FLIP` | 镜像翻转 |

## 系统初始化流程

```
sample_common_dsp_system_init()
├── FH_SYS_Init()              # 系统初始化
├── API_ISP_Open()            # 打开ISP模块
└── VB_init()                 # 视频缓冲池初始化

sample_common_dsp_init()
├── sample_common_vpu_init()    # VPU组初始化
├── sample_common_vpu_chn_init() # VPU通道初始化
├── sample_common_enc_init()    # 编码器初始化
└── sample_common_vpu_bind()   # VPU绑定

sample_common_isp_init()
├── isp_model_init()           # ISP模块初始化
└── 创建ISP线程

sample_common_uvc_start()
├── uvc_init()                 # UVC初始化
└── 创建UVC视频流线程
```

## 支持的视频格式

- **NV12**: YUV420格式
- **YUY2**: YUV422格式
- **MJPEG**: JPEG压缩格式
- **H264/H265**: 视频编码格式
- **IR**: 红外格式

## UVC控制功能

在 `uvc_init.h` 中定义了两类控制：

**Processing Unit Controls** (处理单元):
- 亮度(Brightness)、对比度(Contrast)、饱和度(Saturation)
- 清晰度(Sharpness)、Gamma、逆光(Backlight)
- 白平衡(AWB)、电力线频率(Power Line)

**Camera Terminal Controls** (摄像头终端):
- 曝光模式(AE Mode)、曝光时间(Exposure)
- 聚焦(Focus)、缩放(Zoom)、云台(Pan/Tilt)、滚动(Roll)

## VPSS模块关键API

| API | 功能 |
|-----|------|
| `FH_VPSS_CreateGrp()` | 创建VPSS组 |
| `FH_VPSS_SetViAttr()` | 设置视频输入属性（含裁剪区域） |
| `FH_VPSS_Enable()` | 使能VPSS组 |
| `FH_VPSS_CreateChn()` | 创建VPSS通道 |
| `FH_VPSS_SetChnAttr()` | 设置通道属性 |
| `FH_VPSS_SetChnCrop()` | 设置通道裁剪 |
| `FH_VPSS_SetGlbCrop()` | 设置全局裁剪 |
| `FH_VPSS_SetScalerCoeff()` | 设置缩放滤波器系数 |
| `FH_VPSS_SetVORotate()` | 设置旋转 |

## 用户参数保存

- 参数ID定义在 `uvc_init.h`
- 参数保存在Flash中，地址: `ISP_USER_PARAM_ADDR`
- 相关函数: `uvc_para_read()`, `uvc_para_write()`

## 常用开发命令

```bash
# 编译项目
./build.sh

# 清理并重新编译
./rebuild.sh
```

## SDK API 文档

### 1. 音视频API (FH8626V300芯片)

详细的API函数说明请参考: [api_detailed_notes.md](docs_tools/software/board/api_detailed_notes.md)

该文档基于 `音视频函数开发参考手册_V2.0.pdf` (941页)，包含:

- **SYS模块**: FH_SYS_Init(), FH_SYS_Bind(), FH_SYS_GetChipID()
- **VICAP模块**: FH_VICAP_CreateVi(), FH_VICAP_SetViAttr(), FH_VICAP_Start()
- **VPSS模块**: FH_VPSS_CreateGrp(), FH_VPSS_SetChnAttr(), FH_VPSS_GetChnFrame()
- **VENC模块**: FH_VENC_CreateChn(), FH_VENC_SendFrame(), FH_VENC_GetStream()
- **Audio模块**: FH_AC_AI_Enable(), FH_AC_AO_SendFrame(), FH_AAC_Enc_Encode()
- **NNA模块**: FH_NNA_Model_Init(), FH_NNA_DetectProcess()
- **WiFi模块**: wifi_init(), wifi_connect(), wifi_start_ap()
- **ISP模块**: API_ISP_Init(), API_ISP_LoadIspParam()
- **VB模块**: VB_init(), VB_CreatePool()

### 2. RT-Thread操作系统

RT-Thread开发知识请参考: [rtthread_dev_notes.md](docs_tools/software/board/rtthread_dev_notes.md)

### 3. UVC协议

UVC协议知识请参考: [uvc_protocol_notes.md](docs_tools/software/board/uvc_protocol_notes.md)

该文档基于 `RTos UVC协议及代码介绍.pdf` (63页)，包含:

- **USB传输类型**: 控制/等时/中断/批量传输
- **UVC协议**: VC/VS接口、描述符结构
- **UVC控制**: SET_CUR/GET_CUR、Processing Unit、Camera Terminal
- **视频格式**: YUY2/NV12/MJPEG/H264/H265
- **Payload**: 12字节包头、PTS/SCR、帧结束标志
- **Probe/Commit**: 分辨率协商机制

### 4. Demo代码流程 (仅供理解项目结构)

- **VLC Demo**: `app/media_demo/vlcview/` - 系统初始化→VPU→MJPEG→DMC→ISP→Demo
- **UVC Demo**: `app/media_demo/sample_demo/uvc_rtt/` - UVC初始化→视频流处理线程
- **BSP Demo**: `app/bsp_demo/` - GPIO/UART/PWM/I2C/SADC/RTC/SDCard
- **音频Demo**: `app/audio_demo/` - AI/AO音频

该文档基于 `RT-Thread SDK开发指南.pdf` (162页)，包含:

- **内核基础**: 线程管理、线程状态、线程调度
- **同步与通信**: 消息队列、信号量、互斥锁、事件
- **内存管理**: 内存堆、内存池
- **组件**: FinSH控制台、DFS文件系统、LWIP网络协议栈
- **设备驱动**: UART、GPIO、I2C、SPI
- **软件包**: Env工具、常用软件包

## 已知问题/注意事项

1. **通道裁剪限制**: VPSS通道不支持动态裁剪，调用 `FH_VPSS_SetChnCrop` 返回错误 `0x80094007`
2. **Zoom功能**: 电子放大需要在UVC视频流启动前配置好
3. **参数保存**: 用户参数保存地址为 `ISP_USER_PARAM_ADDR`

## 开发时常用操作

- 搜索函数定义: 使用Grep工具搜索函数名
- 查找文件: 使用Glob工具按模式查找
- 读取头文件: 先查看 `lib/inc/bsp/uvc_init.h` 了解控制接口
- 修改功能开关: 编辑 `uvc_feature_config.h`
