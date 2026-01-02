

## 效果展示

<audio controls>
  <source src="https://cloud.rocketpi.club/cloud/max98357_audio.m4a" type="audio/mp4">
  Your browser does not support the audio element.
</audio>

## 功能描述

- 把 `edge-tts` 语音转成 I2S 需要的 PCM 数据：
- 生成语音头文件直接放在工程中即可使用 audio.h
- tts_web_v2 工具使用，参考tts_web_v2目录下的readme.md文件

![image-20251217225239313](https://cloud.rocketpi.club/cloud/image-20251217225239313.png)

## 硬件连接

![image-20251217225729787](https://cloud.rocketpi.club/cloud/image-20251217225729787.png)



## MAX98357 io口介绍

以 **MAX98357A** 模块为例（典型 3W I2S D 类功放，单声道）：

- **BCLK**
   Bit Clock，比特时钟。
  - 由 **I2S 主机（MCU）输出**
  - 在 STM32 里一般叫 `I2S_SCK` 或 `CK`
  - 每一位数据来一个时钟
- **DIN**
   Data IN，串行音频数据输入。
  - 接 MCU 的 **I2S 发送数据脚**，通常叫 `I2S_SD` / `SDO` / `SD_TX`
  - 音频样本就是从这里进芯片
- **LRC**
   Left/Right Clock，也叫 **LRCK / WS / Word Select**。
  - 一般一半时间代表左声道，一半代表右声道
  - 由 MCU 的 `I2S_WS` / `LRCK` 输出
- **GAIN**
   增益 / 声道选择脚（上电时采样）。
  - 拉到 **不同电平/电阻组合** 可以选择不同输出增益（比如 3dB、6dB、9dB…）
  - 还可以指定芯片取左/右/左右混合哪个声道
  - 若是现成模块，通常板上已经接好默认增益，你可以先**悬空或接板上默认焊盘**使用；要精细调增益再按数据手册来接电阻
- **SD**
   Shutdown，使能脚。
  - **高电平：芯片工作**
  - 低电平：关断（静音+省电）
  - 很多简单用法直接用一个 10k 上拉到 3.3V/5V，让功放一直“开机”；
  - 也可以接 MCU GPIO，用来做“静音/省电控制”

下面的这个模块只需要控制  BCLK  DIN LRC,GAIN和SD悬空

![image-20251122002611792](https://cloud.rocketpi.club/cloud/image-20251122002611792.png)

```
MCU I2S_SCK → BCLK

MCU I2S_WS/LRCK → LRC

MCU I2S_SD(发送) → DIN
```

**PB13 – I2S2_CK** → 接 MAX98357 的 **BCLK**

**PB12 – I2S2_WS** → 接 MAX98357 的 **LRC（LRCK/WS）**

**PB15 – I2S2_SD** → 接 MAX98357 的 **DIN**





