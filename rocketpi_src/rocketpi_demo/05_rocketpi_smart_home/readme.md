## 效果展示

- 输入设备编码，保存刷新即可连接

![image-20260103003344719](https://cloud.rocketpi.club/cloud/image-20260103003344719.png)

![image-20251231005713447](https://cloud.rocketpi.club/cloud/image-20251231005713447.png)

## 功能说明

- ESP01S AT：Wi-Fi + MQTT 连接
- AHT30：温湿度采集并 MQTT 发布
- LED：蓝/绿/粉三色独立控制
- 电机：L9110 正转/反转/停止
- 无源蜂鸣器：频率 1000-3000 Hz
- ST7789 LCD：未连接时显示 ESP8266/Wi-Fi/MQTT 状态，连接成功后显示温湿度与设备编码

## 硬件连接

![image-20251231013251106](https://cloud.rocketpi.club/cloud/image-20251231013251106.png)


## MQTT 主题（deviceId 自动生成）

- 发布：`rocketpi/sensors/<deviceId>/aht30`
- 订阅：
  - LED：`rocketpi/actuators/<deviceId>/led/cmd`
  - 电机：`rocketpi/actuators/<deviceId>/motor/cmd`
  - 蜂鸣器：`rocketpi/actuators/<deviceId>/buzzer/cmd`

## 控制指令

LED：

```
{"cmd":"led","id":"b","state":1}

{"cmd":"led","b":1,"g":0,"p":1}

{"cmd":"led","state":1}
```

电机（速度 0-100）：

```
{"cmd":"motor","dir":"forward","speed":60}

{"cmd":"motor","dir":"reverse","speed":60}

{"cmd":"motor","dir":"stop"}
```

蜂鸣器（频率 1000-3000 Hz）：

```
{"cmd":"buzzer","state":1,"freq":2000}

{"cmd":"buzzer","state":0}
```

说明：`cmd` 可省略，使用对应订阅主题即可。

## 运行说明

- 在 `app/app.c` 配置 `APP_WIFI_SSID`、`APP_WIFI_PASSWORD` 等宏。
- 设备启动后会显示 12 位小写字母的编码（deviceId），用于工具端输入与主题匹配。
- Web 控制台见 `tools/README.md`。

