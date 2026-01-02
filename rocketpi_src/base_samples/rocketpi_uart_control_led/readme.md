## 效果展示

```
{"led":["B","G","P"],"state":[1,1,1]}
```

![uart2_control](https://cloud.rocketpi.club/cloud/uart2_control.gif)



## 功能说明

- RocketPi UART 控制 LED

通过 `USART2` 创建一个简易的串口控制台，使用接近 cJSON 的命令格式控制 3 颗板载 LED (`LED_B`, `LED_G`, `LED_P`)。程序在上电后会输出 GPIO 映射以及示例命令，并在串口终端中提示输入。

### 硬件资源

| LED | 端口 | 引脚 | 备注 |
| --- | ---- | ---- | ---- |
| `B` | GPIOA | PA1  | 在 `main.h` 中定义为 `LED_B_Pin` |
| `G` | GPIOB | PB10 | `LED_G_Pin` |
| `P` | GPIOB | PB14 | `LED_P_Pin` |

LED 通过晶体管反相驱动，GPIO 输出为低电平时亮起（`LED_ACTIVE_LOW`）。如需改为高电平点亮，可在 `Core/Src/main.c` 顶部将 `LED_ACTIVE_LOW` 宏改为 `0`。

### 串口配置

- 端口：`USART2` (`PA2` / `PA3`)
- 波特率：115200，8 数据位，1 停止位，无校验
- 控制台自动回显解析结果

### 命令格式

命令使用 JSON 风格文本，但仅解析以下字段：

```jsonc
{
  "led": "B" | ["B","G","P"] | "ALL",
  "state": 0 | 1 | [0,1,0]
}
```

- `led` 可为单个字符串或字符串数组，支持别名：`B/BLUE/LED_B`、`G/GREEN/LED_G`、`P/PINK/LED_P`，以及 `ALL`（一次性匹配全部灯）。
- `state` 可为单个数值/字符串或数组（长度需与 `led` 数量一致），`0/off/false` 表示熄灭，`1/on/true` 表示点亮。
- 发送多个 LED 但只提供一个 `state` 时，会将同一个状态应用到所有 LED。

每条正确命令都会得到类似的反馈：

```json
{"status":"ok","led":["B","G"],"state":[1,0]}
```

解析失败则返回：

```json
{"status":"error","msg":"missing led field"}
```

### 使用命令示例

1. 打开串口终端（115200 8N1），复位开发板，终端会显示 GPIO 配置和提示符。
2. 发送单灯命令：
   ```
   {"led":"B","state":1}
   ```
3. 发送多灯命令：
   ```
   {"led":["G","P"],"state":[1,0]}
   ```
4. 一次性控制全部 LED：
   ```
   {"led":"ALL","state":0}
   ```

每条命令会在回车 (`\n`) 后立即解析，解析完成会打印下一条命令的提示符 `> `。无需真正链接 cJSON 库，即可通过轻量级的解析逻辑模拟 cJSON 的语法风格。
