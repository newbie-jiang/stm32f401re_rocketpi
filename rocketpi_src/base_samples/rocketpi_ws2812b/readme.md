## 效果展示

![ws2812b_led](https://cloud.rocketpi.club/cloud/ws2812b_led.gif)



## 功能说明

面向 RocketPI STM32F401RE 开发板的 WS2812B 灯带演示工程。主要特性：

- 使用 TIM3 + DMA 驱动 WS2812B，一次刷新整条灯带。
- 提供 `driver_ws2812b` 基础驱动，可自行设置任意 RGB 值。
- `driver_ws2812b_test` 内置多种灯效算法（呼吸、彩虹、追逐等），并支持按毫秒控制运行时长。

## 硬件连接

- WS2812B 数据线：PB1（TIM3_CH4，推挽，Very High）。
- 电源：灯带接 5V   （红色5V  白色GND 绿色Data）

![aab215845ed1d93abce98a5f29d36ac8](https://cloud.rocketpi.club/cloud/aab215845ed1d93abce98a5f29d36ac8.jpg)

![image-20251213223221538](https://cloud.rocketpi.club/cloud/image-20251213223221538.png)

## 程序经逻辑分析仪验证通过 

![image-20251115221818493](https://cloud.rocketpi.club/cloud/image-20251115221818493.png)
