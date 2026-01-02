

## 效果展示

![uart_echo](https://cloud.rocketpi.club/cloud/uart_echo.gif)

## 功能说明

- 使用 USART2 以 115200-8-N-1 配置，通过 DMA 空闲中断持续接收数据并立即回显发送。
- 上电后自动启动 DMA 接收，并在串口输出提示 `RocketPi UART echo ready.` 告知用户示例状态。
- 任意字符（含换行、中文等）都会按照接收顺序原样返回，可用来验证串口连线或上位机发送功能。
- 当出现串口错误（溢出、噪声等）时自动重新启动 DMA，确保回显功能持续可用。
