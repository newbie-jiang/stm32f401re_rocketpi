## 功能描述

示例通过别名 `aht30` 在 I²C1 上挂载 AHT30 温湿度传感器，利用 Zephyr `sensor` 子系统反复执行 `sensor_sample_fetch()`，并分别读取 `SENSOR_CHAN_AMBIENT_TEMP` 与 `SENSOR_CHAN_HUMIDITY`。为了方便阅读，`log_sensor_value()` 会把 `sensor_value` 拆成整数+小数部分，以 `LOG_INF` 的形式每秒输出一次温度/湿度。

通过该示例可以验证 I²C 设备树、驱动 binding 以及传感器初始化是否正常，也便于后续将 `sensor_channel_get()` 的数据接入应用层。

## 编译

```
west build -p always -b rocket_pi aht30
```

## 日志

```
minicom -b 115200 -D /dev/ttyACM0
```

![image-20251207004800589](https://cloud.rocketpi.club/cloud/image-20251207004800589.png)
