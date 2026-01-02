## 效果展示

tools https://ymodem.rocketpi.club/

### 发送数据

<img src="https://cloud.rocketpi.club/cloud/image-20251212070250993.png" width="910" height="600" alt="ymodem" />

![ymodem](https://cloud.rocketpi.club/cloud/ymodem.gif)

### 接收数据

<img src="https://cloud.rocketpi.club/cloud/image-20251212070412603.png" width="910" height="600" alt="ymodem" />

![ymodem_02](https://cloud.rocketpi.club/cloud/ymodem_02.gif)

## 功能说明

本工程在 `component/ymodem` 目录下提供了通用的 YMODEM 协议实现（纯协议逻辑在 `ymodem.c/.h`，平台相关的串口/存储适配放在 `ymodem_port.c`）。

- **接收**：通过 `ymodem_recv(NULL)` 将上位机发送的文件写入片内 Flash 扇区 6、7（地址 0x08040000-0x0807FFFF）。可以使用 PC 端任意 YMODEM 工具验证。  
- **发送**：`ymodem_send()` 会发送固件内置的 `rocketpi_demo.txt` 文本内容，上位机可直接保存并打开。

## 使用方式
1. 在 `Core/Src/main.c` 中通过宏开关选择模式：
   ```c
   // 定义则为发送模式，不定义则进入接收模式
   #define YMODEM_MODE_SEND
   ```
   - 发送模式：复位后自动500 ms，随后调用 `ymodem_send()`，完成后输出返回码。
   - 接收模式：复位后等待500 ms，调用 `ymodem_recv(NULL)`，Flash 内将依次写入收到的文件。
2. 编译并点火，再使用上位机串口工具进行 YMODEM 收发验证。

## 目录结构
- `component/ymodem/ymodem.c/.h`：协议核心，纯平台无关代码。
- `component/ymodem/ymodem_port.c`：STM32 平台适配，包含串口操作、Flash 存储、虚拟发送文件等。
- `Core/Src/main.c`：示例入口，通过宏开关调用发送或接收函数。

## 注意事项
- Flash 扇区抹除与写入在接收前自动完成，请确保其他代码未占用 0x08040000-0x0807FFFF 区域。
- 发送/接收只能单方向工作，YMODEM 协议不支持同时双向；若需动态切换，可在应用层添加命令后再调用对应函数。
- 需要将 `component/ymodem/ymodem_port.c` 加入工程编译。

如需移植到其他平台，只需替换 `ymodem_port.c` 中的串口、计时器、存储回调即可。

