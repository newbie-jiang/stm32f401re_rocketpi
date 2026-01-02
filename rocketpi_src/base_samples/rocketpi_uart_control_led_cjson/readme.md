## 效果展示

```
{"led":["B","G","P"],"state":[1,1,1]}
```

![uart2_control](https://cloud.rocketpi.club/cloud/uart2_control.gif)

## 功能说明

- 基于 `rocketpi_uart_control_led` 的串口协议，使用 `component/cjson` 中的 cJSON 官方库完成实际的 JSON 解析。
- 在 `Core/Src/main.c` 内实现一个阻塞式 UART 控制台，按命令批量控制 RocketPi 板载的三颗 LED (`LED_B`、`LED_G`、`LED_P`)。
- 上电后串口会打印 GPIO 映射、示例命令，并在每次处理完成后回显执行结果或错误 JSON。

### 硬件资源

| LED | 端口 | 引脚 | 说明 |
| --- | ---- | ---- | ---- |
| `B` | GPIOA | PA1  | `main.h` 中定义为 `LED_B_Pin` |
| `G` | GPIOB | PB10 | `LED_G_Pin` |
| `P` | GPIOB | PB14 | `LED_P_Pin` |

LED 通过晶体管反相驱动，GPIO 输出低电平时点亮（`LED_ACTIVE_LOW=1`）。如需改为高电平点亮，可在 `main.c` 顶部将该宏改为 `0`。

### 串口配置

- 端口：`USART2` (`PA2`/`PA3`)
- 波特率：115200，8 数据位，1 停止位，无校验
- 阻塞收发：循环调用 `HAL_UART_Receive` 获取单字节，命令以 `\n` 结束

### 命令格式

命令保持与模拟实现一致，仅解析以下字段（大小写均可）：

```jsonc
{
  "led": "B" | ["B","G","P"] | "ALL",
  "state": 0 | 1 | "on" | [1,0,1]
}
```

- `led` 支持单字符串或数组，别名：`B/BLUE/LED_B`、`G/GREEN/LED_G`、`P/PINK/LED_P`，以及 `ALL`（全部 LED）。
- `state` 支持数字、布尔、字符串（`on/off/true/false/enable/disable`)，也可为数组，长度需与 `led` 数量一致；只提供一个状态时会应用到所有选定 LED。
- 每条命令会收到形如 `{"status":"ok","led":["B"],"state":[1]}` 的成功回显，解析失败则返回 `{"status":"error","msg":"..."}`

### 使用步骤

1. 打开任意串口终端（115200 8N1），连接到 RocketPi 的 `USART2`。
2. 复位开发板，终端会打印 LED 映射与示例，并出现提示符 `> `。
3. 发送示例命令：
   - 单灯点亮：`{"led":"B","state":1}`
   - 多灯控制：`{"led":["G","P"],"state":[1,0]}`
   - 全部熄灭：`{"led":"ALL","state":0}`
4. 命令以换行 `\n` 结尾即可触发解析，控制台会在下一行打印执行结果或错误原因。

### 注意事项

- 由于 cJSON 解析数组命令需要更多堆内存，需将 `Heap_Size` 调整为 `0x800`。若仍保持默认的 `0x200`，发送 `{"led":["B","G","P"],"state":[1,0,1]}` 等命令会因为 `malloc` 失败导致 `invalid json`。

### 目录提示

- `Core/Src/main.c`：UART 控制台与 LED 控制逻辑（使用 cJSON 解析）。
- `component/cjson/`：cJSON 官方源码（已加入工程编译）。
