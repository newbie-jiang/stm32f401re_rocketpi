## 功能描述

示例使用 Zephyr 自带的 HC-SR04 传感器驱动：程序根据 `hcsr04` alias（或 `hcsr04_0` node）获取设备，循环执行 `sensor_sample_fetch()` + `sensor_channel_get(SENSOR_CHAN_DISTANCE)`，再将 `sensor_value_to_double()` 转换为厘米打印，同时统计成功/失败次数与连续失败次数，方便定位供电或连线问题。测量周期固定为 200 ms。

文档还展示了两种调试方式：①构建 `samples/sensor/sensor_shell` 后在 shell 里直接 `sensor get hcsr04`；②运行本示例连续输出距离值。二者结合即可验证触发/回波 GPIO、定时测距以及 Zephyr sensor shell 的协同工作。

## 两种方式驱动

- zephyr自带sensor_shell 查询

```
/* 测试hcsr04 */
west build -p always -b rocket_pi samples/sensor/sensor_shell
```

shell查询

```
uart:~$ sensor get hcsr04
channel type=26(distance) index=0 shift=0 num_samples=1 value=82418102943ns (0.560999)   0.56m
```

- 直接输出


```
west build -p always -b rocket_pi hcsr04
```

```
[OK 2117] distance = 201.60 cm  ok=2117 fail=1
[OK 2118] distance = 201.60 cm  ok=2118 fail=1
[OK 2119] distance = 202.00 cm  ok=2119 fail=1
[OK 2120] distance = 202.00 cm  ok=2120 fail=1
[OK 2121] distance = 201.60 cm  ok=2121 fail=1
[OK 2122] distance = 202.00 cm  ok=2122 fail=1
[OK 2123] distance = 202.00 cm  ok=2123 fail=1
[OK 2124] distance = 202.00 cm  ok=2124 fail=1
[OK 2125] distance = 201.60 cm  ok=2125 fail=1
[OK 2126] distance = 201.50 cm  ok=2126 fail=1
[OK 2127] distance = 201.50 cm  ok=2127 fail=1
[OK 2128] distance = 202.00 cm  ok=2128 fail=1
[OK 2129] distance = 202.00 cm  ok=2129 fail=1
```
