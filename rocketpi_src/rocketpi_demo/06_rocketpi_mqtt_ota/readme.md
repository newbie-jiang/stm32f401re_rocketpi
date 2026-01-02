# RocketPi MQTT OTA 设备端
本工程实现 MQTT OTA 分包升级，仅保留固件升级功能，适配 ESP8266 AT。设备端主动发起升级，接收固件写入 Flash 并校验，校验通过后跳转执行新固件。

## 功能说明

- ESP8266 AT：Wi-Fi + MQTT 连接
- 设备主动发起 OTA 检查（`check`）
- 分包接收（Base64）、CRC32 校验、Flash 写入（`FLASH_TYPEPROGRAM_BYTE`）
- 传输完成后 SHA256 校验，不通过则返回失败原因
- 校验通过后反初始化外设并跳转到新固件
- 实时 ACK/进度上报
- 串口日志详细，便于调试

## Flash 分区（当前配置）

默认写入 STM32F4 的 Sector 6-7（共 256 KB）：

- Sector 6：`0x08040000 - 0x0805FFFF`
- Sector 7：`0x08060000 - 0x0807FFFF`

宏定义位置：`app/app.c`

```
#define OTA_FLASH_BASE         0x08040000UL
#define OTA_FLASH_SIZE         0x00040000UL
#define OTA_FLASH_SECTOR_FIRST FLASH_SECTOR_6
#define OTA_FLASH_SECTOR_LAST  FLASH_SECTOR_7
```

注意：
- 应用固件必须以 `OTA_FLASH_BASE` 作为链接起始地址
- 固件大小不得超过 `OTA_FLASH_SIZE`

## MQTT 主题

根主题为 `APP_MQTT_BASE_TOPIC`，设备 ID 为 `APP_DEVICE_ID`：
- 设备 -> 服务端：`<base>/ota/<deviceId>/req`
- 服务端 -> 设备：`<base>/ota/<deviceId>/cmd`
- 服务端 -> 设备：`<base>/ota/<deviceId>/data`
- 设备 -> 服务端：`<base>/ota/<deviceId>/ack`
- 设备 -> 服务端：`<base>/ota/<deviceId>/state`

## OTA 流程（设备主动）

1) 设备连接 MQTT 后发送 `check`
2) 服务端返回 `offer`
3) 设备发送 `start`
4) 服务端开始分包 `chunk`
5) 设备校验 + 写入 Flash + 回 `ack`
6) 完成后发送 `result`
7) 校验通过则跳转到新固件

### 载荷示例

设备检查：
```json
{ "type": "check", "model": "demo-mcu", "version": "1.0.0", "cap": { "chunkSize": 1024 } }
```

服务端下发：
```json
{ "type": "offer", "status": "update", "version": "1.1.0", "size": 231424, "sha256": "...", "chunkSize": 1024 }
```

分包：
```json
{ "type": "chunk", "index": 0, "offset": 0, "size": 1024, "total": 226, "data": "<base64>", "crc32": "..." }
```

ACK：
```json
{ "type": "ack", "next": 1, "status": "ok" }
```

结果：
```json
{ "type": "result", "status": "ok" }
```

## 跳转说明

下载完成后会进行：
1) SHA256 校验（与 `offer` 的 `sha256` 比对）
2) 向量表有效性检查（SP 在 SRAM 范围、Reset 在应用区间）
3) 反初始化外设，设置 `VTOR`、`MSP` 并跳转

当前反初始化包含：ESP8266 AT、UART2/UART6、TIM3/TIM4 PWM、I2C1、SPI1、DMA 中断、GPIO A/B/C/H。

若镜像无效会返回：
```json
{ "type": "result", "status": "fail", "reason": "invalid_image" }
```

## 使用说明

1) 修改宏（`app/app.c`）：
   - Wi-Fi：`APP_WIFI_SSID` / `APP_WIFI_PASSWORD`
   - MQTT：`APP_MQTT_BROKER` / `APP_MQTT_PORT`
   - 设备信息：`APP_DEVICE_ID` / `APP_DEVICE_MODEL` / `APP_DEVICE_VERSION`
   - 根主题：`APP_MQTT_BASE_TOPIC`

2) 编译并烧录固件
3) 启动 OTA 工具（PC 端）
   - 后端：`tools/backend` 运行 `npm start`
   - 前端：浏览器打开 `http://服务器IP:8080/`
   - 在“MQTT OTA”面板上传 `.bin` 固件

设备会自动发起 `check`，随后进入升级流程。

## 默认参数

- MQTT QoS：`0`
- 分包大小：`1024` 字节
- ACK 超时：`5000` ms（服务端）
- Flash 写入方式：`FLASH_TYPEPROGRAM_BYTE`
- 完整性校验：`SHA256`

## 串口日志

升级时会打印关键日志，例如：

- `[OTA] check sent`
- `[OTA] offer size=...`
- `[OTA] flash erased ...`
- `[OTA] chunk assembled ...`
- `[OTA] progress ...`
- `[OTA] download complete ...`
- `[OTA] image verified, jumping to app`
- `[OTA] invalid image sp=... reset=...`

若出现 `chunk invalid json` 或 `payload truncated`，一般是 ESP8266 AT 行被拆分，当前代码已支持分片拼接，可结合日志排查。
