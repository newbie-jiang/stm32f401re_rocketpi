## 效果展示

![radar_mg58F18](https://cloud.rocketpi.club/cloud/radar_mg58F18.gif)

```c
=== MG58F18 radar test ===
[RADAR][init] ok
Distance threshold: 100
[RADAR][set_distance_threshold] ok
[RADAR] cmd=0x01 data2=0x00 data3=0x00 data4=0x64 checksum=0x65
        distance threshold: 100
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
Trigger state: TRIGGERED
[RADAR] cmd=0x87 data2=0x00 data3=0x00 data4=0x01 checksum=0x86
        trigger state: TRIGGERED
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
[RADAR][IO] state changed: HIGH -> LOW
[RADAR][IO] state changed: LOW -> HIGH
```



## 功能说明

- 连接MG58F18(微波形雷达模块) ，读取当前模块的配置参数并打印，同时打印当前感应状态 HIGH时触发，

## 硬件连接

![image-20251213212303137](https://cloud.rocketpi.club/cloud/image-20251213212303137.png)

![image-20251213211616687](https://cloud.rocketpi.club/cloud/image-20251213211616687.png)

