# RocketPi（STM32）可编译的 Zephyr Samples 汇总（含源码路径）

- support  zephyr v4.3.99

开发板详情 https://www.rocketpi.club

https://github.com/zephyrproject-rtos/zephyr/tree/main

## 基础（basic）

### 1) blinky

- 作用：最经典入门例程，周期性翻转 LED（GPIO 输出）。
- 主要覆盖：GPIO、板级 LED alias、最小工程结构。
- 源码地址：`samples/basic/blinky/`

### 2) hello_world

- 作用：最简单的打印示例，输出 "Hello World"。
- 主要覆盖：console/UART、printk/console 配置、最小运行验证。
- 源码地址：`samples/hello_world/`

### 3) button

- 作用：按键输入示例，按下/释放事件（通常 GPIO 输入 + 中断或轮询）。
- 主要覆盖：GPIO 输入、按键 debouncing（视 sample 实现）、中断回调。
- 源码地址：`samples/basic/button/`

### 4) hash_map

- 作用：演示 Zephyr 的 hashmap（或相关数据结构/容器）使用方式。
- 主要覆盖：内核/基础库数据结构 API、内存分配。
- 源码地址：`samples/basic/hash_map/`

### 5) minimal

- 作用：极简工程骨架（最小依赖），常用于验证 toolchain / 链接脚本 / 启动流程。
- 主要覆盖：最小构建、最小启动、基础打印（视配置）。
- 源码地址：`samples/basic/minimal/`

### 6) sys_heap

- 作用：演示 Zephyr 系统堆（sys_heap）/内存管理机制的用法。
- 主要覆盖：堆分配、碎片化、基本性能验证（偏内部机制）。
- 源码地址：`samples/basic/sys_heap/`

### 7) threads

- 作用：线程创建、调度、优先级、睡眠/延时等基础多线程演示。
- 主要覆盖：kernel thread、同步原语（部分）、时钟。
- 源码地址：`samples/basic/threads/`

---

## 传感器（sensor）

### 8) die_temp_polling

- 作用：轮询方式读取芯片内部温度（die temp），通常是 STM32 内部温度传感器驱动示例。
- 主要覆盖：sensor 子系统、stm32_temp/stm32_vref/stm32_vbat 等内部通道。
- 源码地址：`samples/sensor/die_temp_polling/`

### 9) sensor_shell

- 作用：在 Zephyr Shell 中查询/读取传感器数据（交互式）。
- 主要覆盖：sensor 子系统 + shell 命令扩展/后端。
- 源码地址：`samples/sensor/sensor_shell/`

---

## 驱动（drivers）

### 10) spi_flash

- 作用：SPI NOR Flash 读写/擦除示例（外部闪存/板载 flash）。
- 主要覆盖：SPI、flash API、JEDEC SPI NOR 驱动。
- 源码地址：`samples/drivers/spi_flash/`

### 11) crc

- 作用：CRC 驱动/CRC API 示例（软件实现）。
- 主要覆盖：drivers/crc、校验流程、性能/正确性。
- 源码地址：`samples/drivers/crc/`

### 12) uart/echo_bot

- 作用：串口回显机器人（收到什么回什么），常用于串口连通性与吞吐验证。
- 主要覆盖：UART、console 或专用 UART 设备、收发。
- 源码地址：`samples/drivers/uart/echo_bot/`

### 13) uart/async_api

- 作用：UART 异步 API 示例（回调、DMA/中断驱动、收发事件）。
- 主要覆盖：CONFIG_UART_ASYNC_API、驱动能力验证、缓冲管理。
- 源码地址：`samples/drivers/uart/async_api/`

---

## 子系统（subsys）

### 14) input/input_dump

- 作用：打印 input 子系统事件（键盘/按键/编码器/触摸等输入设备事件）。
- 主要覆盖：input subsystem、事件上报链路、设备绑定。
- 源码地址：`samples/subsys/input/input_dump/`

### 15) fs/fs_sample

- 作用：文件系统示例（挂载、创建文件、读写、列目录等）。
- 主要覆盖：FS API、littlefs/fatfs（取决于配置/板子存储）。
- 源码地址：`samples/subsys/fs/fs_sample/`

### 16) usb/cdc_acm

- 作用：USB CDC-ACM（虚拟串口）示例。
- 主要覆盖：USB device、CDC ACM class、枚举与收发。
- 源码地址：`samples/subsys/usb/cdc_acm/`

### 17) bindesc/hello_bindesc

- 作用：演示 binary descriptor（bindesc）写入/声明（用于在固件中嵌入元数据）。
- 主要覆盖：bindesc 子系统、构建时嵌入信息。
- 源码地址：`samples/subsys/bindesc/hello_bindesc/`

### 18) bindesc/read_bindesc

- 作用：读取/解析 bindesc 中的元数据（与 hello_bindesc 配合理解）。
- 主要覆盖：bindesc 读取 API、输出展示。
- 源码地址：`samples/subsys/bindesc/read_bindesc/`

### 19) console/echo

- 作用：控制台回显（console 输入输出）示例。
- 主要覆盖：console backend、UART console 输入处理。
- 源码地址：`samples/subsys/console/echo/`

### 20) console/getchar

- 作用：演示从控制台读取单个字符（getchar 风格）。
- 主要覆盖：console 输入 API、阻塞读取等行为。
- 源码地址：`samples/subsys/console/getchar/`

### 21) console/getline

- 作用：演示从控制台读取一行（getline 风格）。
- 主要覆盖：行缓冲、编辑/回车处理（视实现）。
- 源码地址：`samples/subsys/console/getline/`

### 22) crc

- 作用：子系统级 CRC 示例（对校验 API 或 CRC 工具的演示）。
- 备注：与 drivers/crc 的侧重点不同，更多是“子系统/库的使用方式”。
- 源码地址：`samples/subsys/crc/`

### 23) debug/debugmon

- 作用：Debug Monitor 相关示例（ARM Cortex-M 的 DebugMonitor 异常/调试辅助）。
- 主要覆盖：异常/调试机制、调试事件处理。
- 源码地址：`samples/subsys/debug/debugmon/`

### 24) portability/cmsis_rtos_v2/philosophers

- 作用：CMSIS-RTOS v2 兼容层示例：哲学家就餐问题（同步互斥经典案例）。
- 主要覆盖：cmsis_rtos_v2 API、mutex/semaphore、线程调度。
- 源码地址：`samples/subsys/portability/cmsis_rtos_v2/philosophers/`

### 25) portability/cmsis_rtos_v2/timer_synchronization

- 作用：CMSIS-RTOS v2 兼容层示例：定时器与同步（timer + sync）。
- 主要覆盖：CMSIS timers、回调、同步原语。
- 源码地址：`samples/subsys/portability/cmsis_rtos_v2/timer_synchronization/`

### 26) rtio/producer_consumer

- 作用：RTIO（Real-Time I/O）示例：生产者/消费者模型。
- 主要覆盖：rtio 提交/完成队列、异步 I/O 组织方式。
- 源码地址：`samples/subsys/rtio/producer_consumer/`

### 27) rtio/sensor_batch_processing

- 作用：RTIO + 传感器批处理示例（批量取样/处理流水线）。
- 主要覆盖：rtio 与 sensor 的结合、批处理/延迟容忍。
- 源码地址：`samples/subsys/rtio/sensor_batch_processing/`

### 28) shell/shell_module

- 作用：演示如何以“模块方式”扩展 Zephyr Shell 命令（自定义命令组）。
- 主要覆盖：shell 注册机制、命令参数解析。
- 源码地址：`samples/subsys/shell/shell_module/`

### 29) shell/devmem_load

- 作用：演示 shell 下的 devmem/内存读写或加载类操作（用于调试）。
- 主要覆盖：shell + 内存访问工具（注意权限/安全）。
- 源码地址：`samples/subsys/shell/devmem_load/`

### 30) smf/hsm_psicc2

- 作用：SMF（State Machine Framework）示例：层级状态机（HSM）案例（psicc2 例子）。
- 主要覆盖：状态机建模、事件驱动、层级状态转换。
- 源码地址：`samples/subsys/smf/hsm_psicc2/`

### 31) smf/smf_calculator

- 作用：SMF 示例：计算器状态机（输入解析/状态切换演示）。
- 主要覆盖：SMF 基础用法、事件/状态表组织。
- 源码地址：`samples/subsys/smf/smf_calculator/`

---

## C++（cpp）

### 32) cpp/hello_world

- 作用：C++ Hello World（验证 C++ 工具链、new/delete、静态初始化等）。
- 主要覆盖：C++ runtime、libstdc++ 链接（视配置）。
- 源码地址：`samples/cpp/hello_world/`

### 33) cpp/cpp_synchronization

- 作用：C++ 同步示例（可能使用 C++ 线程/互斥封装或 Zephyr 同步原语的 C++ 用法）。
- 主要覆盖：同步、互斥、线程安全（C++ 视角）。
- 源码地址：`samples/cpp/cpp_synchronization/`

---

## 数据结构（data_structures）

### 34) min-heap

- 作用：最小堆（min-heap）数据结构示例/测试。
- 主要覆盖：容器操作、算法正确性。
- 源码地址：`samples/data_structures/min-heap/`

---

## 模块（modules）

### 35) cmsis_dsp/moving_average

- 作用：CMSIS-DSP 示例：移动平均滤波。
- 主要覆盖：DSP 库集成、基本数值处理与性能。
- 源码地址：`samples/modules/cmsis_dsp/moving_average/`

### 36) tflite-micro/hello_world

- 作用：TFLite Micro 入门示例（通常是简单正弦/微模型推理）。
- 主要覆盖：TFLM 集成、模型加载、推理流程、内存规划。
- 源码地址：`samples/modules/tflite-micro/hello_world/`



## 支持官方示例 36个

- 编译命令如下，在zephyr目录下编译

```
west build -p always -b rocket_pi samples/basic/blinky

west build -p always -b rocket_pi samples/hello_world

west build -p always -b rocket_pi samples/basic/button

west build -p always -b rocket_pi samples/sensor/die_temp_polling
/* 测试w25qxx */
west build -p always -b rocket_pi samples/drivers/spi_flash

west build -p always -b rocket_pi samples/subsys/input/input_dump
/* 测试tf卡 */
west build -p always -b rocket_pi samples/subsys/fs/fs_sample

west build -p always -b rocket_pi samples/subsys/usb/cdc_acm
/* 测试hcsr04 */
west build -p always -b rocket_pi samples/sensor/sensor_shell

west build -p always -b rocket_pi samples/basic/hash_map

west build -p always -b rocket_pi samples/basic/minimal

west build -p always -b rocket_pi samples/basic/sys_heap

west build -p always -b rocket_pi samples/basic/threads

west build -p always -b rocket_pi samples/cpp/hello_world

west build -p always -b rocket_pi samples/cpp/cpp_synchronization

west build -p always -b rocket_pi samples/data_structures/min-heap

west build -p always -b rocket_pi samples/drivers/crc

west build -p always -b rocket_pi samples/drivers/uart/echo_bot
 
west build -p always -b rocket_pi samples/drivers/uart/async_api

west build -p always -b rocket_pi samples/modules/cmsis_dsp/moving_average

west build -p always -b rocket_pi samples/modules/tflite-micro/hello_world
 
west build -p always -b rocket_pi samples/subsys/bindesc/hello_bindesc
 
west build -p always -b rocket_pi samples/subsys/bindesc/read_bindesc
 
west build -p always -b rocket_pi samples/subsys/console/echo
 
west build -p always -b rocket_pi samples/subsys/console/getchar
  
west build -p always -b rocket_pi samples/subsys/console/getline
  
west build -p always -b rocket_pi samples/subsys/crc

west build -p always -b rocket_pi samples/subsys/debug/debugmon

west build -p always -b rocket_pi samples/subsys/portability/cmsis_rtos_v2/philosophers

west build -p always -b rocket_pi samples/subsys/portability/cmsis_rtos_v2/timer_synchronization

west build -p always -b rocket_pi samples/subsys/rtio/producer_consumer

west build -p always -b rocket_pi samples/subsys/rtio/sensor_batch_processing

west build -p always -b rocket_pi samples/subsys/shell/shell_module

west build -p always -b rocket_pi samples/subsys/shell/devmem_load

west build -p always -b rocket_pi samples/subsys/smf/hsm_psicc2

west build -p always -b rocket_pi samples/subsys/smf/smf_calculator

```

## 适配驱动示例  14 个 

- 详细功能查看rocketpi_example目录下的md文档

- 需在rocketpi_example目录下编译

```
west build -p always -b rocket_pi display

west build -p always -b rocket_pi aht30

west build -p always -b rocket_pi hcsr04

west build -p always -b rocket_pi buzzer

west build -p always -b rocket_pi sg90

west build -p always -b rocket_pi ws2812b

west build -p always -b rocket_pi max98357

west build -p always -b rocket_pi --sysbuild with_mcuboot

west build -p always -b rocket_pi irda_nec

west build -p always -b rocket_pi l9110h

west build -p always -b rocket_pi adc_joystick

west build -p always -b rocket_pi esp8266

west build -p always -b rocket_pi radar

west build -p always -b rocket_pi at24c02
```



