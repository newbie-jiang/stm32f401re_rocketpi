## 效果展示

![shell](https://cloud.rocketpi.club/cloud/shell.gif)

## 功能说明

集成 microrl 命令行库，在 `USART2`（115200 8N1）上提供交互式 shell，并扩展了简单的板载功能控制。

1. **shell 接口**  
   - 通过串口工具连接 `USART2`，复位后会显示 `IRin >` 提示符。  
   - 支持 `help / version / echo` 等 microrl 默认命令。
   - 自持自动补全以及历史命令回看
   
2. **LED 控制命令**  
   - 实现 `led <blue|green|pink> <on|off|toggle>` 指令，可在 shell 中直接点亮/熄灭/翻转三色 LED（LED_B/LED_G/LED_P 引脚）。  
   - 命令支持大小写混输，错误输入会返回提示。

3. **按键回显优化**  
   - 接收中断使用环形缓冲，兼容 CR / CRLF 终端换行，保证回显与命令解析稳定。
