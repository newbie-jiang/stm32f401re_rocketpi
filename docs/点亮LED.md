# 点亮 LED

点亮一颗 LED 灯是绝大多数单片机入门者迈出的第一步。这个实验虽然简单，却能帮助你串起“硬件连接 → 外设配置 → 下载调试”的完整流程，也能验证开发环境是否搭建成功。本教程将以 STM32（Rocket-Pi 套件所使用的主控）为例，详细讲解如何完成这一实验。

> 如果你使用的开发板与本文稍有差异，只需在“查找 LED 引脚”这一步确认自己的 GPIO 号，并在后续步骤中替换即可。

## 实验目标

- 认清 LED 的结构、工作原理与常见接法。
- 查阅原理图确定板载 LED 的连接方式。
- 使用 STM32CubeMX 配置 GPIO，并生成 Keil 工程。
- 在 `main.c` 中编写点亮与闪烁 LED 的代码，编译、下载并验证。
- 掌握常见调试技巧，能快速排查 LED 不亮的原因。

## 需要准备的工具

- Rocket-Pi 开发板或任意带有板载 LED 的 STM32 开发板。
- USB 数据线（可传输数据，非充电线）。
- ST-LINK 调试器（如果开发板未内置）。
- 电脑（Windows 10/11），确保已安装：
  - STM32CubeMX
  - Keil MDK（或你熟悉的 IDE，如 STM32CubeIDE）
  - ST-LINK 驱动与 STM32CubeProgrammer（下载工具）
  - 串口调试助手（用于输出调试信息，可选）
- 一份开发板原理图（PDF 或纸质均可）。

## 步骤一：理解 LED 的工作原理

1. **结构认识**  
   发光二极管（Light Emitting Diode）具有两个引脚：长脚为正极（阳极）、短脚为负极（阴极），封装内部存在一个 PN 结，只允许电流沿单一方向流动。

2. **电气特性**  
   - 正向导通电压（`V_f`）：常见红色 LED 约为 1.8~2.2 V，绿色约 2.0~2.4 V，蓝色/白色约 2.8~3.3 V。  
   - 推荐工作电流（`I_f`）：一般为 5~20 mA，取决于亮度需求。

3. **限流电阻的必要性**  
   单片机 GPIO 输出 3.3 V 或 5 V，如果不串联电阻，LED 会因为电流过大而烧毁。限流电阻计算公式：

   \[
   R = \frac{V_{\text{GPIO}} - V_f}{I_f}
   \]

   例如：GPIO 输出 3.3 V、选择红色 LED（`V_f ≈ 2.0 V`）、期望电流 10 mA，则 `R ≈ (3.3-2.0)/0.01 = 130 Ω`。可以选用 150 Ω 的标准阻值。

4. **极性判断技巧**  
   - 透明封装可观察内部电极，较大的电极片对应负极。  
   - PCB 上通常会标注 “＋”“－” 或使用三角箭头符号指向负极。  
   - 封装侧面的缺口、平边通常对应负极。

## 步骤二：确认开发板上的 LED 接法

1. **查看丝印或说明书**  
   Rocket-Pi 板常在 LED 旁标注 `LED1`、`D1` 等字样。顺着丝印可以找到与之相连的 GPIO 编号，例如 `PA5` 或 `PC13`。  
   如果板上没有标注，请打开随包附带的原理图。

2. **阅读原理图**  
   找到 LED 所在的原理图页面，确认：
   - LED 的阳极是否接到了 GPIO 引脚。  
   - LED 的阴极是否接到了 VCC 或 GND。  
   - 是否串联了限流电阻（通常在 LED 一侧标有 `Rxx`）。  
   - 与 LED 同一路的 GPIO 是否还有其他用途（避免冲突）。

3. **记录关键信息**  
   - GPIO 端口：如 `GPIOA`。  
   - GPIO 引脚号：如 `PA5`。  
   - LED 极性：GPIO 输出高电平是否点亮，还是输出低电平点亮（这取决于 LED 是共阳还是共阴接法）。

> **实例（请以你手上的原理图为准）**：Rocket-Pi 某版本中，板载 LED（LED1）阳极接 `PA5`，阴极通过 330 Ω 电阻接地，因此当 `PA5` 输出高电平时，LED 点亮。

## 步骤三：在 STM32CubeMX 中配置 GPIO

1. **创建工程**
   - 打开 STM32CubeMX，点击 `File -> New Project`。  
   - 在 `Board Selector` 搜索 Rocket-Pi 对应的芯片型号（如 STM32F407VG）。若无现成板卡，可在 `MCU Selector` 中直接选择芯片。

2. **基础设置**
   - 在 `Project Manager -> Project` 中填写项目名称（例如 `LED_Blink`），选择生成工具链为 `MDK-ARM V5`。  
   - 在 `Code Generator` 选项卡勾选 `Generate peripheral initialization as a pair of '.c/.h' files per peripheral`，方便后期查看。

3. **引脚配置**
   - 回到 `Pinout & Configuration`，在左侧 `Categories -> GPIO` 中点击需要的引脚，例如 `PA5`。  
   - 将其模式设置为 `GPIO_Output`。  
   - 在右侧的 `GPIO Mode and Configuration` 面板中确认：
     - `GPIO mode`：`Output Push Pull`（默认）。  
     - `GPIO Pull-up/Pull-down`：根据电路设置，常用 `No pull-up and no pull-down`。  
     - `Maximum output speed`：`Low` 或 `Medium`，点亮 LED 不需要高速。  
     - `User Label`：可填 `LED1`，方便阅读。

4. **时钟与系统设置**
   - 对于点亮 LED 实验，不强制修改时钟，但建议在 `Clock Configuration` 中确认 HCLK、PCLK 等默认值是否符合需求（通常默认即可）。  
   - 在 `System Core -> SYS` 中，将 `Debug` 设置为 `Serial Wire`，以便使用 ST-LINK。

5. **生成代码**
   - 点击工具栏右上角的 `Generate Code`。  
   - 首次生成时，CubeMX 会提示是否打开生成的工程，选择 `Open Project`。

## 步骤四：在 Keil 中编写点亮代码

1. **打开工程**
   - Keil 自动启动后，确认左侧项目树中存在 `Src/main.c`。

2. **修改 `main.c`**
   - CubeMX 默认会在 `main()` 中调用 `MX_GPIO_Init()`，该函数已完成 GPIO 输出配置。  
   - 在 `while (1)` 循环中编写点亮或闪烁代码，例如：

```c
/* USER CODE BEGIN WHILE */
while (1)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  // 输出高电平，点亮 LED
  HAL_Delay(500);                                      // 延时 500 ms
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // 输出低电平，熄灭 LED
  HAL_Delay(500);
}
/* USER CODE END WHILE */
```

   - 如果你的 LED 是低电平点亮（常见于 LED 正极接 VCC，负极接 GPIO），则需要将 `GPIO_PIN_SET` 与 `GPIO_PIN_RESET` 的顺序对调。

3. **编译与下载**
   - 点击 `Project -> Build Target`（或工具栏上齿轮图标）编译工程，确保无错误。  
   - 连接 ST-LINK，选择 `Flash -> Download`，将程序烧录到开发板。  
   - 如果使用外部调试器，请在 `Options for Target -> Debug` 中选择 `ST-Link Debugger`。

4. **观察现象**
   - 下载完成后，开发板上的 LED 应以 0.5 秒的间隔闪烁。  
   - 若未点亮，请参考下方的排错指南。

## 步骤五：常见问题排查

- **LED 完全不亮**  
  - 确认电源已供电（板上电源指示灯亮）。  
  - 检查 GPIO 引脚是否设置为 `Output` 模式，是否生成了新的代码并重新下载。  
  - 使用万用表测量 GPIO 电压，判断程序是否输出正确电平。  
  - 若 LED 为低电平点亮，确保代码逻辑已调整。  
  - 确认限流电阻存在且焊接良好。

- **LED 常亮或常灭，不闪烁**  
  - 检查 `HAL_Delay()` 是否生效（需要 `SysTick` 中断支持，CubeMX 默认开启）。  
  - 确认 `while (1)` 中没有被其他代码阻塞。  
  - 若使用 RTOS，需要确认 LED 控制任务是否正常调度。

- **编译报错找不到 HAL 函数**  
  - 确保未删除 `MX_GPIO_Init()` 的调用。  
  - 在 Keil 的 `Manage Run-Time Environment` 中检查 `HAL` 相关组件是否勾选。

- **下载失败**  
  - ST-LINK 驱动是否正确安装。  
  - 复位引脚、BOOT 引脚是否被外接电路占用。  
  - 尝试使用 STM32CubeProgrammer 连接芯片，检查是否能识别。

## 进阶扩展练习

1. **自定义闪烁节奏**：让 LED 以摩尔斯电码闪烁，练习编写数组和定时逻辑。  
2. **按键控制 LED**：结合另一个 GPIO 输入，实现“按下按钮亮、松开灭”。  
3. **使用定时器中断**：替换 `HAL_Delay()`，在定时器回调函数中切换 LED 状态。  
4. **多灯控制**：同时配置多个 LED，引入状态机或结构体管理。  
5. **低功耗实验**：让 MCU 进入睡眠模式，使用外部事件点亮 LED，观察功耗变化。

## 小结与下一步

- 你已经完成了从硬件认识、引脚配置到代码编写、调试验证的完整流程。  
- 建议将 CubeMX 配置、Keil 工程和实验记录提交到版本管理工具（如 Git），以便日后回顾。  
- 下一步可以尝试“串口输出”“按键输入”等实验，为更复杂的项目（例如传感器采集、无线通信）打好基础。

点亮第一颗 LED 是一个仪式，它意味着你已经与单片机建立了成功的沟通。带着这份信心继续探索，嵌入式世界将向你敞开。
