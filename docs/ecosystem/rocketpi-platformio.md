---
title: 让 Rocket-Pi 开发多一种选择：感谢 majorzpley 带来 PlatformIO 社区适配
description: 介绍社区作者 majorzpley 为 Rocket-Pi 整理的 PlatformIO 示例工程，提供项目入口、起步建议，并感谢作者的适配与分享。
---

# 让 Rocket-Pi 开发多一种选择：感谢 majorzpley 带来 PlatformIO 社区适配

![PlatformIO 标识](../assets/images/ecosystem/platformio.png){ width="220" height="220" style="display: block; max-width: 100%; height: auto; margin: 1.5rem auto;" }

项目官网：[PlatformIO](https://platformio.org/)。

**感谢社区作者 [majorzpley](https://github.com/majorzpley) 为 Rocket-Pi 带来的 PlatformIO 适配与分享！**

作者在 [01_rocketpi 仓库](https://github.com/majorzpley/01_rocketpi)中整理了 Rocket-Pi 相关工程，其中 [base_examples 基础示例目录](https://github.com/majorzpley/01_rocketpi/tree/main/base_examples)提供了丰富的实践入口。对于希望使用 VS Code 配合 PlatformIO 开发 Rocket-Pi 的朋友，这份社区贡献提供了可以参考和动手尝试的工程基础。

## 从点灯到外设实践，已有这些探索入口

查看作者的 `base_examples` 目录，可以找到工程模板，以及覆盖多种外设和应用主题的示例：

| 方向 | 目录中的示例 |
| --- | --- |
| 基础入门 | 工程模板、LED、按键扫描、按键中断、多功能按键、微秒延时 |
| 串口交互 | printf、回显、串口控制 LED、cJSON、YMODEM、Shell、EasyLogger |
| 传感与控制 | AHT30、AT24Cxx、蜂鸣器、舵机、电机、超声波、ADC、红外、WS2812B |
| 显示与音频 | SPI LCD、位图显示、LVGL、I2S、SD 卡音频播放与图片显示 |
| 存储与通信 | SDIO、FATFS、W25Qxx、littlefs、USB CDC、USB MSC、ESP8266 |
| 其他实践 | 待机唤醒、CRC、mbedTLS、扩展 IO 检查 |

以上为[作者仓库目录](https://github.com/majorzpley/01_rocketpi/tree/main/base_examples)中的示例主题，具体内容与后续更新请以原仓库为准。

这样的整理让我们可以从熟悉的功能入手：先观察 LED 和串口输出，再逐步阅读传感器、显示或文件系统工程，对照理解不同功能的代码组织与依赖关系。

## 以 LED 工程认识适配方式

作者的 [LED 示例](https://github.com/majorzpley/01_rocketpi/tree/main/base_examples/01_rocketpi_led)中包含 `platformio.ini`、`src`、`include` 和 `lib` 等文件与目录。其 [PlatformIO 配置](https://github.com/majorzpley/01_rocketpi/blob/main/base_examples/01_rocketpi_led/platformio.ini)采用以下设置：

- 目标板配置：`genericSTM32F401RE`。
- 开发平台与框架：`ststm32`、`stm32cube`。
- 默认烧录与调试工具：`stlink`。
- CPU 频率配置为 84 MHz，外部晶振参数为 8 MHz。

这些配置为理解工程如何匹配 Rocket-Pi 硬件提供了直接参考。作者还在 [LED 示例说明](https://github.com/majorzpley/01_rocketpi/blob/main/base_examples/01_rocketpi_led/readme.md)中记录了调试问题和配合 clangd 使用的经验，方便后来者查阅。

## 建议怎样开始

1. 下载或克隆[作者仓库](https://github.com/majorzpley/01_rocketpi)，先阅读准备运行的示例说明。
2. 在装有 PlatformIO 的 VS Code 中，打开具体示例的工程目录，例如 `base_examples/01_rocketpi_led`，确认该目录包含 `platformio.ini`。
3. 对照自己的开发板、晶振和调试器检查工程配置，先完成构建，再连接硬件尝试烧录与调试。
4. 从 LED 示例开始观察运行现象，再选择按键、串口等熟悉的例程逐步扩展。

学习过程中，可以配合本站的 [Rocket-Pi 硬件一览](../roadmap/hardware.md)和[学习路线](../roadmap/roadmap.md)，对照引脚、接线与功能原理阅读工程。

!!! note "社区适配说明"
    本文介绍的是 majorzpley 分享的社区项目。本次核对了仓库目录、LED 工程配置与说明，未逐一编译或进行板上验证。各例程的使用条件、依赖与支持情况，请以作者当前版本为准。

## 感谢作者，让经验成为大家的起点

把示例适配到另一套开发环境，需要处理工程配置、源码组织、头文件路径、烧录与调试等细节。愿意把这些工作整理并公开分享，能够为后来者提供很有价值的参考。

**感谢 majorzpley 对 Rocket-Pi 生态的支持，也感谢作者把适配成果和使用经验带给社区。** 每一份可供阅读的工程、每一条真实的调试记录，都让更多开发者有机会从已有经验出发，继续完成自己的实验与作品。

如果这份项目对你有帮助，欢迎前往[原仓库](https://github.com/majorzpley/01_rocketpi)为作者点一个 Star，或通过清晰的问题反馈、文档补充和改进贡献支持项目。分享相关成果时，也请保留作者署名和原始项目链接，让贡献者的工作被更多人看见。
