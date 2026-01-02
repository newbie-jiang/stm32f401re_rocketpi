

## 效果展示

![aht30](https://cloud.rocketpi.club/cloud/aht30.gif)

## 功能说明

面向 RocketPI STM32F401RE 开发板的 **AHT30温湿度 演示工程**。主要特性：

- 驱动AHT30在串口上打印温湿度。

- 提供 `driver_aht30` 基础驱动。

- `driver_aht30_test` 直接调用测试，自主选择轮询时间。

- `soft_i2c` 提供通用GPIO bit-bang I2C主机，可复用到其它外设。

- `driver_aht30_config.h` 允许在软件/硬件I2C之间切换并集中定义GPIO与时序宏。

### 软硬件i2c切换

![image-20251213015257240](https://cloud.rocketpi.club/cloud/image-20251213015257240.png)

## 硬件连接

![image-20251213015406267](https://cloud.rocketpi.club/cloud/image-20251213015406267.png)











































