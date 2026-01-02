# 效果展示

![sd_audio](https://cloud.rocketpi.club/cloud/sd_audio.gif)

## 功能描述

- 从 SD 卡读取 16-bit  PCM，经 I2S2+DMA 输出，可播放 SD 流和内置音轨。   （拷贝整个audio目录至sd卡根目录）
- FATFS 速度测试工具，用于测量 SD 写入/读取性能。
- ST7789 SPI 屏幕初始化并显示实时频谱（Goertzel 算法，40 个频段）。

使用

- 在 SD 根目录放置 `audio/audio.bin`，格式为 16 kHz、16-bit、立体声 PCM（与 AUDIO_SAMPLE_RATE_HZ/AUDIO_BITS_PER_SAMPLE/AUDIO_NUM_CHANNELS 一致）。

- 上电后自动播放：双缓冲从 SD 读取并送入 I2S DMA；SD 出错时可回退到编译时内置音轨。

- 频谱基于左声道数据，默认刷新周期 `SPECTRUM_DRAW_INTERVAL_MS = 40ms`。

- 宏 `AUDIO_VOLUME_PERCENT`（Core/Src/main.c），范围 0–100，默认 100。
- SD 流：每次从文件读入双缓冲后即按 Q15 乘法 + 饱和缩放；
  内置音轨：先拷贝到 RAM 缓冲，再缩放后送 DMA。
- 0 = 静音，100 = 直通；修改宏后重新编译下载即可。
