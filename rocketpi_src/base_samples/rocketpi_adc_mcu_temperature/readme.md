

## 效果展示

![adc_temp](https://cloud.rocketpi.club/cloud/adc_temp.gif)

## 功能说明

该示例工程基于 STM32F4，使用 ADC1 轮询方式依次采集内部温度传感器（Channel 16）与内部基准电压 VREFINT（Channel 17），并结合工厂校准常数计算当前 MCU 芯片温度，最终通过 USART2 输出到终端。

- ADC1 配置为 12bit 分辨率、同步分频 4、单次软件触发，序列包含温度与 VREFINT 两个通道。
- 每次采样时会执行 5 组转换并取中值，结合校准常数（TS_CAL1/TS_CAL2）和实时 VDDA，得到更稳定的摄氏温度结果。
- 温度以 `MCU Temp: xx.xx C` 的格式每秒打印一次；若 ADC 读取失败，会输出 `MCU Temp read failed` 以便排查。
