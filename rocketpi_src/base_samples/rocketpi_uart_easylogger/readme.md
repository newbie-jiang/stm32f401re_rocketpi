## 功能展示

https://github.com/armink/EasyLogger

![easylogger](https://cloud.rocketpi.club/cloud/easylogger.gif)

## 功能描述

集成 [EasyLogger](https://github.com/armink/EasyLogger) 并通过 `USART2` + DMA 输出日志。

- `component/EasyLogger` 下保留了 EasyLogger v2.2.99 的完整源码与配置。
- 自定义移植层 `component/EasyLogger/easylogger/port/elog_port.c`：
  - 使用 `HAL_UART_Transmit_DMA` 在 `USART2` 上进行非阻塞输出。
  - DMA 启动失败时自动退回阻塞式 `HAL_UART_Transmit`，保证日志一定能发出去。
  - 提供简单的临界区锁 + HAL Tick 时间戳。
  - 实现 `HAL_UART_TxCpltCallback` / `HAL_UART_ErrorCallback`，在 DMA 结束或出错时释放发送状态。
- `elog_cfg.h` 中默认使用 CRLF (`"\r\n"`) 作为换行符，串口调试助手显示更友好。
- `Core/Src/main.c` 展示了完整流程：
  - 初始化 EasyLogger、分别配置各级别的输出格式并调用 `elog_start()`。
  - `elog_demo_all_levels()` 函数逐级打印 raw/assert/error/warn/info/debug/verbose 示例。
  - 主循环每 1 秒输出一次心跳日志，证明 DMA 发送的实时性。

常用文件：

- `component/EasyLogger/easylogger/inc/elog_cfg.h`：编译期配置（缓冲区、颜色、换行等）。
- `component/EasyLogger/easylogger/port/elog_port.c`：与硬件相关的适配层（串口、锁、时间）。
