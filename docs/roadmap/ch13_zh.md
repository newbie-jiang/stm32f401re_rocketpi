# 13 故障排除

## 13.1 章节简介和范围

本章重点介绍了用户遇到的最常见问题
是 FreeRTOS 的新手。首先，它重点关注三个已被证明的问题
多年来成为最常见的支持请求来源：
错误的中断优先级分配、堆栈溢出和
printf() 的不当使用。然后以 FAQ 的风格简短地说明：
涉及其他常见错误、其可能的原因及其后果
解决方案。

> *使用 `configASSERT()` 通过立即捕获和
> 识别许多最常见的错误来源。它是强烈的
> 建议在开发或调试时定义 `configASSERT()`
> FreeRTOS 应用程序。 `configASSERT()` 在第 12.2 节中进行了描述。*


## 13.2 中断优先级

> *注意：这是支持请求的首要原因，并且在大多数情况下
> 定义 `configASSERT()` 的端口将立即捕获错误！*

如果使用的FreeRTOS端口支持中断嵌套，并且服务
中断例程使用 FreeRTOS API，那么它是
*必要*中断优先级设置为等于或低于
`configMAX_SYSCALL_INTERRUPT_PRIORITY`，如第 7.8 节所述，
中断嵌套。如果不这样做将导致无效
关键部分，这反过来会导致间歇性故障。

如果在以下处理器上运行 FreeRTOS，请特别小心：

- 中断优先级默认为尽可能高
  优先级，某些 ARM Cortex 处理器就是这种情况，以及
  可能是其他人。在此类处理器上，中断的优先级
  使用 FreeRTOS API 的程序不能保持未初始化状态。

- 数字高优先级数字代表逻辑低中断
  优先级，这可能看起来违反直觉，因此会导致
  混乱。 ARM Cortex 处理器上也是如此，并且
  可能是其他人。

- 例如，在这样的处理器上，正在执行的中断
  优先级 5 本身可以被具有以下优先级的中断中断：
  优先级为 4。因此，如果 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 为
  设置为 5，任何使用 FreeRTOS API 的中断都只能
  分配的优先级数字高于或等于 5。
  在这种情况下，中断优先级 5 或 6 是有效的，但是
  中断优先级为3肯定是无效的。

  ![](https://cloud.rocketpi.club/cloud/image91.png)

- 不同的库实现期望优先级
  中断以不同的方式指定。再次强调，这是特别
  与针对 ARM Cortex 处理器的库相关，其中
  中断优先级在写入之前会进行位移位
  硬件寄存器。有些库会执行位移
  他们自己，而其他人则期望执行位移位
  在将优先级传递给库函数之前。

- 同一架构的不同实现实现了
  不同数量的中断优先级位。例如，Cortex-M
  一个制造商的处理器可以实现 3 个优先级位，而
  来自其他制造商的 Cortex-M 处理器可以实现 4
  优先级位。

- 定义中断优先级的位可以拆分
  定义抢占优先级的位和定义抢占优先级的位之间
  定义一个子优先级。确保所有位都分配给
  指定抢占优先级，以便不使用子优先级。

在某些 FreeRTOS 移植中，`configMAX_SYSCALL_INTERRUPT_PRIORITY` 具有
替代名称 `configMAX_API_CALL_INTERRUPT_PRIORITY`。


## 13.3 堆栈溢出

堆栈溢出是第二个最常见的支持请求来源。
FreeRTOS 提供了多种功能来协助捕获和调试
堆栈相关问题[^28]。

[^28]：这些功能在 FreeRTOS Windows 端口中不可用。


### 13.3.1 uxTaskGetStackHighWaterMark() API 功能

每个任务维护自己的堆栈，其总大小是指定的
当任务创建时。 `uxTaskGetStackHighWaterMark()`用于查询
任务有多接近溢出分配给的堆栈空间
它。该值称为堆栈“高水位线”。


<a name="list13.1" title="Listing 13.1 The uxTaskGetStackHighWaterMark() API function prototype"></a>

```c
UBaseType_t uxTaskGetStackHighWaterMark( TaskHandle_t xTask );
```
***清单 13.1*** *uxTaskGetStackHighWaterMark() API 函数原型*

**uxTaskGetStackHighWaterMark()参数及返回值**

- `xTask`

正在处理堆栈高水位线的任务句柄
  queried (the subject task)—请参阅 pxCreatedTask 参数
  `xTaskCreate()` API 函数获取有关获取句柄的信息
  任务。

任务可以通过传入 NULL 来查询自己的堆栈高水位线
  有效任务句柄的位置。

- 返回值

任务使用的堆栈量随着任务的增加和减少
  执行并处理中断。 `uxTaskGetStackHighWaterMark()`
  返回已分配的最小剩余堆栈空间量
  自任务开始执行以来可用。这是堆栈的数量
  当堆栈使用量达到 greatest (or deepest) 时保持未使用状态
  值。高水位线越接近零，任务就越接近
  堆栈已经溢出。

可以使用 `uxTaskGetStackHighWaterMark2()` API 代替
`uxTaskGetStackHighWaterMark()` 仅在返回类型上有所不同。


<a name="list13.2" title="Listing 13.2 The uxTaskGetStackHighWaterMark2() API function prototype"></a>

```c
configSTACK_DEPTH_TYPE uxTaskGetStackHighWaterMark2( TaskHandle_t xTask );
```
***清单 13.2*** *uxTaskGetStackHighWaterMark2() API 函数原型*

使用 `configSTACK_DEPTH_TYPE` 允许应用程序编写者控制类型
用于堆栈深度。

### 13.3.2 运行时堆栈检查 - 概述

FreeRTOS 包括三种可选的运行时堆栈检查机制。这些
由 `configCHECK_FOR_STACK_OVERFLOW` 编译时间控制
FreeRTOSConfig.h 内的配置常量。两种方法都会增加
执行上下文切换所需的时间。

堆栈溢出 hook (or stack overflow callback) 是一个函数
当内核检测到堆栈溢出时调用。使用堆栈
溢出钩子函数：

1. 将 `configCHECK_FOR_STACK_OVERFLOW` 设置为 1、2 或 3 英寸
   FreeRTOSConfig.h，如以下小节所述。

1.提供hook函数的实现，使用准确
   函数名称和原型如清单 13.3 所示。


<a name="list13.3" title="Listing 13.3 The stack overflow hook function prototype"></a>

```c
void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed char *pcTaskName );
```
***清单 13.3*** *堆栈溢出钩子函数原型*

提供堆栈溢出钩子来捕获和调试堆栈
错误更容易，但没有真正的方法可以从堆栈溢出中恢复
当它发生时。该函数的参数传递句柄和名称
堆栈溢出到钩子函数中的任务。

堆栈溢出钩子是从中断上下文中调用的。

一些微控制器在检测到异常时会生成故障异常
内存访问错误，有可能触发故障
在内核有机会调用堆栈溢出钩子函数之前。


### 13.3.3 运行时堆栈检查 - 方法 1

当 `configCHECK_FOR_STACK_OVERFLOW` 设置为 1 时，选择方法 1。

任务的整个执行上下文每次都会保存到其堆栈中
被换掉。这很可能是堆栈的时间
使用量达到顶峰。当`configCHECK_FOR_STACK_OVERFLOW`设置为1时，
内核检查堆栈指针是否保留在有效堆栈内
保存上下文后的空间。堆栈溢出钩子是
如果发现堆栈指针超出其有效范围，则调用。

方法 1 执行速度很快，但可能会错过发生的堆栈溢出
上下文切换之间。


### 13.3.4 运行时堆栈检查——方法 2

方法 2 对已经描述的检查执行附加检查
方法1.当`configCHECK_FOR_STACK_OVERFLOW`设置为2时选择。

创建任务时，其堆栈会填充已知模式。方法
2 测试任务堆栈空间的最后有效20字节以验证
该模式尚未被覆盖。堆栈溢出钩子函数
如果 20 个字节中的任何一个与预期发生了变化，则调用
价值观。

方法2执行速度不如方法1快，但也比较
速度很快，因为只测试了 20 个字节。最有可能的是，它会捕获所有堆栈
溢出；然而，有些人认为 possible (but highly improbable)
溢出将会被错过。

### 13.3.4 运行时堆栈检查——方法 3

当 `configCHECK_FOR_STACK_OVERFLOW` 设置为 3 时，选择方法 3。

此方法仅适用于选定的端口。当可用时，此方法
启用 ISR 堆栈检查。当检测到 ISR 堆栈溢出时，
断言被触发。注意这里并没有调用栈溢出钩子函数
这种情况是因为它特定于任务堆栈而不是 ISR 堆栈。

## 13.4 printf() 和 sprintf() 的使用

通过 `printf()` 进行记录是常见的错误来源，并且，
没有意识到这一点，应用程序开发人员通常会添加
进一步调用 `printf()` 来帮助调试，这样做会加剧
问题。

许多交叉编译器供应商将提供 `printf()` 实现，
适合在小型嵌入式系统中使用。即使那是
在这种情况下，实现可能不是线程安全的，可能不会
适合在中断服务程序内使用，并且取决于
输出定向的地方，需要比较长的时间来执行。

如果 `printf()` 实现是
专门为小型嵌入式系统设计的不可用，并且
而是使用通用 `printf()` 实现，如下所示：

- 只需调用 `printf()` 或 `sprintf()` 就可以大量
  增加应用程序可执行文件的大小。

- `printf()` 和 `sprintf()` 可能会调用 `malloc()`，如果
  正在使用除 heap\_3 之外的内存分配方案。参见部分
  3.2，内存分配方案示例，了解更多信息。

- `printf()` 和 `sprintf()` 可能需要大很多倍的堆栈
  比其他情况下需要的。


### 13.4.1 Printf-stdarg.c

许多 FreeRTOS 演示项目都使用名为
printf-stdarg.c，提供最小且堆栈高效的
`sprintf()` 的实施可用于代替标准
库版本。在大多数情况下，这将允许更小的堆栈
分配给调用 `sprintf()` 及相关函数的每个任务。

printf-stdarg.c 还提供了一种用于引导 `printf()` 的机制
逐字符输出到端口，虽然速度较慢，但允许堆栈
使用量进一步减少。

请注意，并非 `printf-stdarg.c` 的所有副本都包含在 FreeRTOS 中
下载工具 `snprintf()`。未实现 `snprintf()` 的副本
只需忽略缓冲区大小参数，因为它们直接映射到
`sprintf()`。

printf-stdarg.c 是开源的，但归第三方所有，并且
因此与 FreeRTOS 分开许可。许可条款是
包含在源文件的顶部。


## 13.5 其他常见错误源

### 13.5.1 症状：向演示添加简单任务会导致演示崩溃

创建任务需要从堆中获取内存。许多
演示应用程序项目将堆设计得非常大
足以创建演示任务 - 因此，创建任务后，
剩余的堆将不足以用于任何进一步的任务、队列、事件
要添加的组或信号量。

创建空闲任务，也可能创建 RTOS 守护程序任务
调用 `vTaskStartScheduler()` 时自动执行。
仅当堆不足时才会返回 `vTaskStartScheduler()`
用于创建这些任务的剩余内存。包括空循环 `[ for(;;); ]`
调用 `vTaskStartScheduler()` 后可以使此错误更容易调试。

为了能够添加更多任务，您必须增加堆大小，或删除
一些现有的演示任务。堆大小的增加总是
受可用 RAM 数量的限制。请参见第 3.2 节，示例内存
分配方案，了解更多信息。


### 13.5.2 症状：在中断内使用 API 函数会导致应用程序崩溃

不要在中断服务程序中使用 API 函数，除非
API 函数的名称以“...FromISR()”结尾。特别是，不要
除非使用中断，否则在中断内创建临界区
安全宏。请参阅第 7.2 节，从 ISR 使用 FreeRTOS API，了解
更多信息。

在支持中断嵌套的 FreeRTOS 端口中，请勿使用任何 API
已分配中断优先级的中断中的函数
高于 `configMAX_SYSCALL_INTERRUPT_PRIORITY`。请参见第 7.8 节“中断”
嵌套，了解更多信息。


### 13.5.3 症状：有时应用程序会在中断服务例程中崩溃

首先要检查的是中断是否导致堆栈
溢出。某些端口仅检查任务内的堆栈溢出，而不检查
在中断内。

中断的定义和使用方式因端口而异
编译器之间。因此，第二件事要检查的是
中断服务中使用的语法、宏和调用约定
例程与所提供的文档页面上的描述完全相同
正在使用的端口，与演示应用程序中演示的完全相同
随端口提供。

如果应用程序运行在使用数值较低的处理器上
优先级数字代表逻辑上的高优先级，然后确保
分配给每个中断的优先级都会考虑到这一点，因为它可以
似乎违反直觉。如果应用程序正在处理器上运行
将每个中断的优先级默认为最大可能
优先级，然后确保每个中断的优先级不保留在其
默认值。请参见第 7.8 节“中断嵌套”和第 13.2 节，
中断优先级，了解更多信息。


### 13.5.4 症状：尝试启动第一个任务时调度程序崩溃

确保已安装 FreeRTOS 中断处理程序。请参阅
正在使用的 FreeRTOS 端口的文档页面以获取信息，以及
为端口提供的演示应用程序作为示例。

某些处理器必须处于特权模式才能进行调度
开始了。实现这一点的最简单方法是将处理器放入
在调用 main() 之前，C 启动代码中的特权模式。


### 13.5.5 症状：中断意外被禁用，或者关键部分未正确嵌套

如果在调度程序之前调用 FreeRTOS API 函数
开始然后中断将故意被禁用，而不是
再次重新启用，直到第一个任务开始执行。这样做是为了
保护系统免受尝试使用中断导致的崩溃
FreeRTOS API 在系统初始化期间、在
调度程序已经启动，并且虽然调度程序可能处于
不一致的状态。

不要更改微控制器中断使能位或优先级标志
使用除调用 `taskENTER_CRITICAL()` 之外的任何方法和
`taskEXIT_CRITICAL()`。这些宏保留其调用嵌套的计数
深度以确保仅当调用时才再次启用中断
嵌套已完全降至零。请注意，有些图书馆
函数本身可以启用和禁用中断。


### 13.5.6 症状：应用程序在调度程序启动之前就崩溃了

可能导致上下文的中断服务例程
在调度程序被执行之前，不得允许 switch 执行
开始了。这同样适用于任何尝试的中断服务例程
发送到 FreeRTOS 对象或从 FreeRTOS 对象接收，例如队列或
信号量。上下文切换只有在调度程序完成之后才能发生
开始了。

许多 API 函数只有在调度程序完成后才能调用
开始了。最好将 API 的使用限制为创建对象
例如任务、队列和信号量，而不是使用这些
对象，直到调用 `vTaskStartScheduler()` 之后。


### 13.5.7 症状：在调度程序挂起时或从关键部分内部调用 API 函数会导致应用程序崩溃

通过调用 `vTaskSuspendAll()` 来暂停调度程序，通过调用 `xTaskResumeAll()` 来暂停调度程序 resumed
(unsuspended)。进入临界区
通过调用 `taskENTER_CRITICAL()`，并通过调用退出
`taskEXIT_CRITICAL()`。

当调度程序挂起或从
在关键部分内。

## 13.6 附加调试步骤

如果您遇到上述常见原因未涵盖的问题，
您可以尝试使用以下一些调试步骤。

- 定义`configASSERT()`，启用malloc失败检查和堆栈溢出
  检查应用程序的 FreeRTOSConfig 文件。
- 检查 FreeRTOS API 的返回值以确保它们是
  成功了。
- 检查调度程序相关配置，例如 `configUSE_TIME_SLICING`，以及
  `configUSE_PREEMPTION` 根据应用要求正确设置。
- [This page](https://www.freertos.org/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers.html)
  提供有关调试 Cortex-M 微控制器上的硬故障的详细信息。
