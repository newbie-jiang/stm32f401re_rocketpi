## 功能描述

示例针对 MG58F18 微波雷达模块编写了完整的串口协议驱动，设备树里把 USART1 暴露为 `radaruart`，并提供 `zephyr,user` 的 `radar_out_gpios` 读取模块的 OUT 引脚。`mg58f18_radar_test_run()` 启动后会先初始化串口、按协议轮流读取/写回距离门限、延迟、阻塞时间、光敏开关、供电模式、触发方式、功率步进等寄存器，并将结果格式化打印，便于确认与模组的通信是否可靠。

主循环调用 `mg58f18_radar_test_poll()`，既会解析串口上报帧并打印，也会通过 GPIO 读取 OUT 脚的电平变化，输出 “HIGH -> LOW” 之类的状态，帮助调试触发逻辑。整体流程展示了如何在 Rocket Pi 上复用 UART + GPIO 组合，对雷达进行参数调校并实时监控触发状态。

## 编译

```shell
west build -p always -b rocket_pi radar
```

## 日志

```shell
=== MG58F18 radar test ===
[RADAR][init] ok
*** Booting Zephyr OS build v4.2.0-5860-ge8b08d32e572 ***
MG58F18 radar sample start
Distance threshold: 2100
[RADAR][set_distance_threshold] ok
[RADAR] cmd=0x01 data2=0x00 data3=0x08 data4=0x34 checksum=0x3D
        distance threshold: 2100
Output delay: 1000 ms
[RADAR][set_delay_ms] ok
[RADAR] cmd=0x02 data2=0x00 data3=0x7D data4=0x00 checksum=0x7F
        delay: 1000 ms
Block time: 1000 ms
[RADAR][set_block_time_ms] ok
[RADAR] cmd=0x04 data2=0x00 data3=0x7D data4=0x00 checksum=0x79
        block time: 1000 ms
Light sensor: OFF
[RADAR][set_light_sensor_enabled] ok
[RADAR] cmd=0x03 data2=0x00 data3=0x00 data4=0x00 checksum=0x03
        light sensor: OFF
Active level: HIGH
[RADAR][set_active_level] ok
[RADAR] cmd=0x05 data2=0x00 data3=0x00 data4=0x01 checksum=0x04
        active level: HIGH
Power mode: ULTRA LOW
[RADAR][set_power_mode] ok
[RADAR] cmd=0x06 data2=0x00 data3=0x00 data4=0x00 checksum=0x06
        power mode: ULTRA LOW (50/60uA)
Trigger state: IDLE
[RADAR] cmd=0x87 data2=0x00 data3=0x00 data4=0x00 checksum=0x87
        trigger state: IDLE
Environment: NIGHT
[RADAR] cmd=0x88 data2=0x00 data3=0x00 data4=0x01 checksum=0x89
        light environment: NIGHT
Firmware version: V2.1
[RADAR] cmd=0x89 data2=0x00 data3=0x00 data4=0x21 checksum=0xA8
        firmware version: V2.1
Trigger mode: CONTINUOUS
[RADAR][set_trigger_mode] ok
[RADAR] cmd=0x0A data2=0x00 data3=0x00 data4=0x00 checksum=0x0A
        trigger mode: CONTINUOUS
TX power step: 5
[RADAR][set_power_step] ok
[RADAR] cmd=0x0B data2=0x00 data3=0x00 data4=0x05 checksum=0x0E
        TX power step: 5
Light threshold: 0x12
[RADAR][set_light_threshold] ok
[RADAR] cmd=0x0C data2=0x00 data3=0x00 data4=0x12 checksum=0x1E
        light threshold: 0x12
PWM enabled: NO
[RADAR][set_pwm_enabled] ok
[RADAR] cmd=0x0D data2=0x00 data3=0x00 data4=0x00 checksum=0x0D
        PWM state: OFF
PWM duty raw: 1050
[RADAR][set_pwm_duty_raw] ok
[RADAR] cmd=0x0E data2=0x00 data3=0x04 data4=0x1A checksum=0x10
        PWM duty raw: 1050
Power pulse width: 0x30
[RADAR][set_power_pulse_width] ok
[RADAR] cmd=0x0F data2=0x00 data3=0x00 data4=0x30 checksum=0x3F
        pulse width: 0x30
Sensing mode: MOTION
[RADAR][set_sensing_mode] ok
[RADAR] cmd=0x10 data2=0x00 data3=0x00 data4=0x00 checksum=0x10
        sensing mode: MOTION DETECTION
[RADAR][save_settings] ok
=== MG58F18 radar smoke test done ===
[RADAR] cmd=0x20 data2=0x00 data3=0x00 data4=0x01 checksum=0x21
        save settings ack
[RADAR][IO] initial state: HIGH
[RADAR][IO] state changed: HIGH -> LOW
[RADAR][IO] state changed: LOW -> HIGH
[RADAR][IO] state changed: HIGH -> LOW
[RADAR][IO] state changed: LOW -> HIGH

```
