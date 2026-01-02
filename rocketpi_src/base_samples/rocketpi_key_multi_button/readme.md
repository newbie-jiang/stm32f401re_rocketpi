

## 效果展示

![button](https://cloud.rocketpi.club/cloud/button.gif)



## 功能说明
- 通过 HAL SysTick (1ms) 驱动 `MultiButton` 库，在滴答中断内每 `TICKS_INTERVAL=5ms` 调用 `button_ticks()`，实现稳定的消抖与事件机。
- 用户按键接在 `PA0`（`KEY_Pin`），默认为上拉到高电平，按下为高电平有效，可在 `Core/Src/main.c` 中的 `USER_BUTTON_ACTIVE_LEVEL` 修改电平极性。
- 根据 `MultiButton` 事件回调控制三色 LED：
  - 单击：切换 `LED_B`。
  - 双击：切换 `LED_G`。
  - 长按开始：切换 `LED_P`。

### 关键文件

- `Core/Src/main.c`：初始化 `Button` 句柄、绑定事件回调并在回调里操作 LED。
- `Core/Src/stm32f4xx_it.c`：在 `SysTick_Handler` 中进行 5ms 分频并调用 `button_ticks()`。
- `component/MultiButton/`：第三方多按键库源码。 https://github.com/0x1abin/MultiButton
