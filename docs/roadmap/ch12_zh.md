# 12 开发者支持

## 12.1 简介

本章重点介绍了一组功能，旨在最大限度地提高
生产力：

- 提供对应用程序行为方式的深入了解。
- 突出优化机会。
- 在错误发生时捕获错误。


## 12.2 configASSERT()

在 C 语言中，宏 `assert()` 用于验证*断言*（
假设）由程序做出。断言被写成C语言
表达式，如果表达式的计算结果为 false (0)，则
断言被视为失败。例如，清单 12.1 测试了
断言指针 `pxMyPointer` 不是 NULL。


<a name="list12.1" title="Listing 12.1 Using the standard C assert() macro to check pxMyPointer is not NULL"></a>

```c
/* Test the assertion that pxMyPointer is not NULL */
assert( pxMyPointer != NULL );
```
***清单 12.1*** *使用标准 C assert() 宏来检查 pxMyPointer 是不是 NULL*

应用程序编写者指定在断言出现时要采取的操作
由于提供 `assert()` 宏的实现而失败。

FreeRTOS源代码没有调用`assert()`，因为`assert()`不是
适用于编译 FreeRTOS 的所有编译器。
相反，FreeRTOS 源代码包含大量对宏的调用
称为 `configASSERT()`，它可以由应用程序编写者定义
`FreeRTOSConfig.h`，其行为与标准 C `assert()` 完全相同。

失败的断言必须被视为致命错误。不要尝试
执行断言失败的行。

> *使用 `configASSERT()` 通过立即捕获和
> 识别许多最常见的错误来源。它是强烈的
> 建议在开发或调试时定义 `configASSERT()`
> FreeRTOS 应用程序。*

定义 `configASSERT()` 将极大地帮助运行时调试，但是
还会增加应用程序代码大小，从而减慢速度
它的执行。如果未提供 `configASSERT()` 的定义，则
将使用默认的空定义，并且所有调用
`configASSERT()` 将被 C 预处理器完全删除。


### 12.2.1 configASSERT() 定义示例

清单 12.2 中所示的 `configASSERT()` 的定义在以下情况下很有用：
应用程序正在调试器的控制下执行。它将
在断言失败的任何行上停止执行，因此该行
失败时，断言将是调试器显示的行
调试会话已暂停。


<a name="list12.2" title="Listing 12.2 A simple configASSERT() definition useful when executing under the control of a debugger"></a>

```c
/* Disable interrupts so the tick interrupt stops executing, then sit
   in a loop so execution does not move past the line that failed the
   assertion. If the hardware supports a debug break instruction, then the
   debug break instruction can be used in place of the for() loop. */

#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for(;;); }
```
***清单 12.2*** *一个简单的 configASSERT() 定义在调试器控制下执行时很有用*

清单 12.3 中所示的 `configASSERT()` 的定义在以下情况下很有用：
应用程序不在调试器的控制下执行。它
打印出或以其他方式记录失败的源代码行
断言。使用以下命令来识别断言失败的行
标准C `__FILE__`宏来获取源文件的名称，以及
标准C `__LINE__`宏获取行号
源文件。


<a name="list12.3" title="Listing 12.3 A configASSERT() definition that records the source code line that failed an assertion"></a>

```c
/* This function must be defined in a C source file, not the FreeRTOSConfig.h 
   header file. */
void vAssertCalled( const char *pcFile, uint32_t ulLine )
{
    /* Inside this function, pcFile holds the name of the source file that 
       contains the line that detected the error, and ulLine holds the line 
       number in the source file. The pcFile and ulLine values can be printed 
       out, or otherwise recorded, before the following infinite loop is 
       entered. */
    RecordErrorInformationHere( pcFile, ulLine );

    /* Disable interrupts so the tick interrupt stops executing, then sit in a 
       loop so execution does not move past the line that failed the assertion. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

/* These following two lines must be placed in FreeRTOSConfig.h. */
extern void vAssertCalled( const char *pcFile, unsigned long ulLine );
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ )
```
***清单 12.3*** *记录断言失败的源代码行的 configASSERT() 定义*


## 12.3 FreeRTOS 的 Tracealyzer

Tracealyzer for FreeRTOS 是提供的运行时诊断和优化工具
由我们的合作伙伴公司 Percepio 提供。

Tracealyzer for FreeRTOS 捕获有价值的动态行为信息，然后
在互连的图形视图中呈现捕获的信息。的
工具还能够显示多个同步视图。

捕获的信息在分析、故障排除、
或者只是优化 FreeRTOS 应用程序。

Tracealyzer for FreeRTOS 可以与传统调试器并行使用，并且
通过更高级别、基于时间的方式补充调试器的视图
观点。


<a name="fig12.1" title="Figure 12.1 FreeRTOS+Trace includes more than 20 interconnected views"></a>
<a name="fig12.2" title="Figure 12.2 FreeRTOS+Trace main trace view - one of more than 20 interconnected trace views"></a>
<a name="fig12.3" title="Figure 12.3 FreeRTOS+Trace CPU load view - one of more than 20 interconnected trace views"></a>
<a name="fig12.4" title="Figure 12.4 FreeRTOS+Trace response time view - one of more than 20 interconnected trace views"></a>
<a name="fig12.5" title="Figure 12.5 FreeRTOS+Trace user event plot view - one of more than 20 interconnected trace views"></a>
<a name="fig12.6" title="Figure 12.6 FreeRTOS+Trace kernel object history view - one of more than 20 interconnected trace views"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image82.png)   
***图 12.1*** *FreeRTOS+Trace 包括 20 多个互连视图*

![](https://cloud.rocketpi.club/cloud/image83.png)   
***图 12.2*** *FreeRTOS+Trace 主跟踪视图 - 20 多个互连跟踪视图之一*

![](https://cloud.rocketpi.club/cloud/image84.png)   
***图 12.3*** *FreeRTOS+Trace CPU 负载视图 - 20 多个互连跟踪视图之一*

![](https://cloud.rocketpi.club/cloud/image85.png)   
***图 12.4*** *FreeRTOS+Trace 响应时间视图 - 20 多个互连跟踪视图之一*

![](https://cloud.rocketpi.club/cloud/image86.png)   
***图 12.5*** *FreeRTOS+Trace 用户事件图视图 - 20 多个互连跟踪视图之一*

![](https://cloud.rocketpi.club/cloud/image87.png)   
***图 12.6*** *FreeRTOS+Trace 内核对象历史视图 - 20 多个互连跟踪视图之一*

* * *


## 12.4 调试相关Hook (Callback)函数

### 12.4.1 Malloc 失败挂钩

malloc 失败 hook (or callback) 在第 3 章“堆”中进行了描述
内存管理。

定义 malloc 失败挂钩可确保应用程序开发人员
如果尝试创建任务、队列、信号量或
事件组失败。

### 12.4.2 堆栈溢出钩子

堆栈溢出挂钩的详细信息在第 13.3 节“堆栈”中提供
溢出。

定义堆栈溢出挂钩可确保应用程序开发人员
如果任务使用的堆栈量超出堆栈空间，则通知
分配给任务。


## 12.5 查看运行时和任务状态信息

### 12.5.1 任务运行时统计

任务运行时统计信息提供有关处理量的信息
每个任务收到的时间。任务的*运行时间*是该任务的总时间
自应用程序启动以来，任务一直处于运行状态。

运行时统计数据旨在用作分析和调试
项目开发阶段的援助。他们的信息
提供仅在计数器用作运行时统计信息之前有效
时钟溢出。收集运行时统计数据会增加任务
上下文切换时间。

要获取二进制运行时统计信息，请调用
`uxTaskGetSystemState()` API 功能。获取运行时统计信息
信息作为人类可读的 ASCII 表，调用
`vTaskGetRunTimeStatistics()` 辅助函数。


### 12.5.2 运行时统计时钟

运行时统计需要测量滴答周期的分数。
因此，RTOS 滴答计数不用作运行时统计信息
时钟，而时钟则由应用程序代码提供。它是
建议制作运行时统计时钟的频率
比滴答频率快 10 到 100 倍
中断。运行时统计时钟越快，越准确
统计数据就会，而且时间值也会越快
溢出。

理想情况下，时间值将由自由运行的 32 位生成
外围定时器/计数器，无需其他即可读取其值
处理开销。如果可用的外设和时钟速度
不使该技术成为可能，然后替代，但效率较低，
技术包括：

- 配置外设以生成周期性中断
  期望运行时统计时钟频率，然后使用计数
  作为运行时统计时钟生成的中断数量。

如果周期性中断只是
  用于提供运行时统计时钟。
  但是，如果应用程序已经使用周期性中断
  合适的频率，那么添加计数就简单高效
  现有中断中生成的中断数量
  服务常规。

- 使用自由运行的当前值生成 32 位值
  16 位外设定时器作为 32 位值的最低有效位
  16位，定时器溢出的次数
  32 位值的最高有效 16 位。

通过适当且有些复杂的操作，可以
通过组合 RTOS 滴答计数生成运行时统计时钟
ARM Cortex-M SysTick 定时器的当前值。一些
FreeRTOS 下载中的演示项目演示了如何实现这一点。


### 12.5.3 配置应用程序以收集运行时统计信息

以下是收集任务运行时所需的宏的详细信息
统计数据。最初，这些宏旨在包含在
RTOS 端口层，这就是宏以“port”为前缀的原因，但它
事实证明，在 `FreeRTOSConfig.h` 中定义它们更为实用。

**用于收集运行时统计信息的宏**

- `configGENERATE_RUN_TIME_STATS`

该宏必须在 FreeRTOSConfig.h 中设置为 1。当这个宏是
  设置为 1 调度程序将调用本节中详细介绍的其他宏
  在适当的时候。

- `portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()`

必须提供该宏来初始化任何外设
  用于提供运行时统计时钟。

- `portGET_RUN_TIME_COUNTER_VALUE()` 或 `portALT_GET_RUN_TIME_COUNTER_VALUE(Time)`

必须提供这两个宏之一来返回当前
  运行时统计时钟值。这是应用程序的总时间
  自运行时统计时钟单位以来一直在运行
  应用程序首先启动。

如果使用第一个宏，则必须将其定义为计算结果
  当前时钟值。如果使用第二个宏，则必须将其定义为
  将其“时间”参数设置为当前时钟值。


### 12.5.4 uxTaskGetSystemState() API 功能

`uxTaskGetSystemState()` 提供状态信息快照
每个任务都在 FreeRTOS 调度程序的控制下。信息
以 `TaskStatus_t` 结构数组的形式提供，其中有一个索引
每个任务的数组。清单 12.5 描述了 `TaskStatus_t` 和
下面。


<a name="list12.4" title="Listing 12.4 The uxTaskGetSystemState() API function prototype"></a>

```c
UBaseType_t uxTaskGetSystemState( TaskStatus_t * const pxTaskStatusArray,
                                  const UBaseType_t uxArraySize,
                                  configRUN_TIME_COUNTER_TYPE * const pulTotalRunTime );
```
***清单 12.4*** *uxTaskGetSystemState() API 函数原型*

> 注意：为了向后兼容，`configRUN_TIME_COUNTER_TYPE` 默认为 `uint32_t`，但也可以 
> 如果 `uint32_t` 限制过多，则在 FreeRTOSConfig.h 中覆盖。


**uxTaskGetSystemState()参数及返回值**

- `pxTaskStatusArray`

指向 `TaskStatus_t` 结构数组的指针。

对于每个数组，该数组必须至少包含一个 `TaskStatus_t` 结构
  任务。任务的数量可以使用以下方式确定
  `uxTaskGetNumberOfTasks()` API 功能。

`TaskStatus_t` 结构如清单 12.5 所示，
  TaskStatus\_t 结构成员在下一个列表中描述。

- `uxArraySize`

`pxTaskStatusArray` 参数指向的数组的大小。
  大小指定为数组中索引的数量（数量
  数组中包含的 `TaskStatus_t` 结构的数量），而不是数量
  数组中的字节。

- `pulTotalRunTime`

如果 `FreeRTOSConfig.h` 中的 `configGENERATE_RUN_TIME_STATS` 设置为 1，
  然后 `*pulTotalRunTime` 由 `uxTaskGetSystemState()` 设置为总运行
  时间（由运行时统计时钟定义）
  应用程序）自目标启动以来。

`pulTotalRunTime` 可选，如果总运行量可设置为 NULL
  不需要时间。

- 返回值

填充的 `TaskStatus_t` 结构的数量
  返回 `uxTaskGetSystemState()`。

返回的值应该等于返回的数字
  `uxTaskGetNumberOfTasks()` API 函数，但如果该值为零
  传入的 `uxArraySize` 参数太小。


<a name="list12.5" title="Listing 12.5 The TaskStatus\_t structure"></a>

```c
typedef struct xTASK_STATUS
{
    TaskHandle_t xHandle;
    const char *pcTaskName;
    UBaseType_t xTaskNumber;
    eTaskState eCurrentState;
    UBaseType_t uxCurrentPriority;
    UBaseType_t uxBasePriority;
    configRUN_TIME_COUNTER_TYPE ulRunTimeCounter;
    StackType_t * pxStackBase;
    #if ( ( portSTACK_GROWTH > 0 ) || ( configRECORD_STACK_HIGH_ADDRESS == 1 ) )
        StackType_t * pxTopOfStack;
        StackType_t * pxEndOfStack;
    #endif
    uint16_t usStackHighWaterMark;
    #if ( ( configUSE_CORE_AFFINITY == 1 ) && ( configNUMBER_OF_CORES > 1 ) )
        UBaseType_t uxCoreAffinityMask;
    #endif
} TaskStatus_t;
```
***清单 12.5*** *TaskStatus\_t 结构*

**TaskStatus_t 结构成员**

- `xHandle`

结构中的信息所涉及的任务的句柄。

- `pcTaskName`

任务的人类可读文本名称。

- `xTaskNumber`

每个任务都有一个唯一的 `xTaskNumber` 值。

如果应用程序在运行时创建和删除任务，那么它是
  任务可能与之前的任务具有相同的句柄
  之前删除的。提供`xTaskNumber`以允许应用程序代码，
  和内核感知调试器，以区分仍在运行的任务
  有效，以及与有效任务具有相同句柄的已删除任务
  任务。

- `eCurrentState`

保存任务状态的枚举类型。
  `eCurrentState` 可以是以下值之一：

- `eRunning`
  - `eReady`
  - `eBlocked`
  - `eSuspended`
  - `eDeleted`

仅将任务报告为处于 `eDeleted` 状态
  通过调用删除任务的时间间隔很短
  `vTaskDelete()`，以及空闲任务释放内存的时间
  分配给已删除任务的内部数据结构和堆栈。
  在那之后，该任务将不再以任何方式存在，并且
  尝试使用其句柄无效。

- `uxCurrentPriority`

当时运行任务的优先级
  `uxTaskGetSystemState()` 被称为。 `uxCurrentPriority`只会更高
  比应用程序编写者分配给任务的优先级，如果
  任务已暂时被分配了更高的优先级
  章节中描述的优先级继承机制
  [8.3 Mutexes (and Binary Semaphores)](ch08_zh.md#83-mutexes-and-binary-semaphores)。

- `uxBasePriority`

应用程序编写者分配给任务的优先级。
  仅当 `configUSE_MUTEXES` 设置为 1 时，`uxBasePriority` 才有效
  FreeRTOSConfig.h。

- `ulRunTimeCounter`

自创建任务以来任务使用的总运行时间。的
  总运行时间以使用时钟的绝对时间形式提供
  由应用程序编写者提供，用于运行时的集合
  统计数据。 `ulRunTimeCounter` 仅在以下情况下有效
  `configGENERATE_RUN_TIME_STATS` 在 FreeRTOSConfig.h 中设置为 1。

- `pxStackBase`

指向分配给该任务的堆栈区域的基地址。

- `pxTopOfStack`

指向分配给该任务的堆栈区域的当前顶部地址。
   仅当堆栈向上增长时，字段 `pxTopOfStack` 才有效（即
   `portSTACK_GROWTH` 大于零）或 `configRECORD_STACK_HIGH_ADDRESS`
   在 FreeRTOSConfig.h 中设置为 1。

- `pxEndOfStack`

指向分配给该任务的堆栈区域的结束地址。
   仅当堆栈向上增长时，字段 `pxEndOfStack` 才有效（即
   `portSTACK_GROWTH` 大于零）或 `configRECORD_STACK_HIGH_ADDRESS`
   在 FreeRTOSConfig.h 中设置为 1。

- `usStackHighWaterMark`

任务的堆栈高水位线。这是最低金额
  自创建任务以来为该任务保留的堆栈空间。
  它表明任务已经接近溢出的程度
  堆栈；该值越接近零，任务就越接近
  溢出其堆栈。 `usStackHighWaterMark` 以字节为单位指定。

- `uxCoreAffinityMask`

一个按位值，指示可以运行任务的核心。 
   内核编号从 0 到 `configNUMBER_OF_CORES` - 1。例如，
   可以在核心 0 和核心 1 上运行的任务将具有其 `uxCoreAffinityMask`
   设置为 0x03。仅当同时满足以下条件时，字段 `uxCoreAffinityMask` 才可用
   `configUSE_CORE_AFFINITY` 设置为 1，`configNUMBER_OF_CORES` 设置为 1
   在 FreeRTOSConfig.h 中设置为大于 1。


### 12.5.5 vTaskListTasks() 辅助函数

`vTaskListTasks()` 提供与以下类似的任务状态信息
`uxTaskGetSystemState()`，但它以人类的方式呈现信息
可读的 ASCII 表，而不是二进制值数组。

`vTaskListTasks()` 是一个处理器密集型函数，并且保留了
调度程序长时间暂停。因此，建议
仅将该函数用于调试目的，而不是用于生产
实时系统。

如果 `configUSE_TRACE_FACILITY` 设置为 1 并且
`configUSE_STATS_FORMATTING_FUNCTIONS` 设置为大于 0
FreeRTOSConfig.h。


<a name="list12.6" title="Listing 12.6 The vTaskListTasks() API function prototype"></a>

```c
void vTaskListTasks( char * pcWriteBuffer, size_t uxBufferLength );
```
***清单 12.6*** *vTaskListTasks() API 函数原型*

**vTaskListTasks() 参数**

- `pcWriteBuffer`

指向写入格式化且人类可读表的字符缓冲区的指针。 
  假设该缓冲区足够大以包含生成的报告。  
  每个任务大约 40 个字节就足够了。

- `uxBufferLength`

`pcWriteBuffer` 的长度。

`vTaskListTasks()` 生成的输出示例如图 12.7 所示。
在输出中：

- 每行提供有关单个任务的信息。

- 第一列是任务名称。

- 第二列是任务的状态，其中“X”表示正在运行，“R”表示“就绪”，“B”表示
  表示已阻止，“S”表示已暂停，“D”表示任务已被
  已删除。任务只会被报告为处于已删除状态
  通过调用删除任务之间的短暂时间
  `vTaskDelete()`，以及空闲任务释放内存的时间
  被分配给已删除任务的内部数据结构并且
  堆栈。在那之后，任务将不再存在，
  并且尝试使用其句柄是无效的。

- 第三列是任务的优先级。

- 第四列是任务的堆栈高水位线。请参阅
  `usStackHighWaterMark` 的描述。

- 第五列是分配给任务的唯一编号。请参阅
  `xTaskNumber` 的描述。


<a name="fig12.7" title="Figure 12.7 Example output generated by vTaskListTasks()"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image88.png)   
***图 12.7*** *vTaskListTasks() 生成的示例输出*

* * *

> 注意：   
> `vTaskListTasks` 的旧版本是 `vTaskList`。 `vTaskList` 假设 
> `pcWriteBuffer` 的长度为 `configSTATS_BUFFER_MAX_LENGTH`。该功能仅适用于 
> 向后兼容性。新应用建议使用 `vTaskListTasks` 和 
> 明确提供 `pcWriteBuffer` 的长度。


<a name="list12.7" title="Listing 12.7 The vTaskList() API function prototype"></a>

```c
void vTaskList( signed char *pcWriteBuffer );
```
***清单 12.7*** *vTaskList() API 函数原型*

**vTaskList() 参数**

- `pcWriteBuffer`
  

指向写入格式化且人类可读表的字符缓冲区的指针。 
    缓冲区必须足够大以容纳整个表，因为不执行边界检查。


### 12.5.6 vTaskGetRunTimeStatistics() 辅助函数

`vTaskGetRunTimeStatistics()` 将收集的运行时统计数据格式化为
人类可读的 ASCII 表。

`vTaskGetRunTimeStatistics()` 是一个处理器密集型函数，并且离开
调度程序暂停很长一段时间。因此，它是
建议仅将该函数用于调试目的，而不是在
生产实时系统。

当 `configGENERATE_RUN_TIME_STATS` 设置为
1、`configUSE_STATS_FORMATTING_FUNCTIONS`设置为大于0，并且 
`configUSE_TRACE_FACILITY` 在 FreeRTOSConfig.h 中设置为 1。


<a name="list12.8" title="Listing 12.8 The vTaskGetRunTimeStatistics() API function prototype"></a>

```c
void vTaskGetRunTimeStatistics( char * pcWriteBuffer, size_t uxBufferLength );
```
***清单 12.8*** *vTaskGetRunTimeStatistics() API 函数原型*

**vTaskGetRunTimeStatistics() 参数**

- `pcWriteBuffer`

指向写入格式化且人类可读表的字符缓冲区的指针。
  假设该缓冲区足够大以包含生成的报告。  
  每个任务大约 40 个字节就足够了。

- `uxBufferLength`

`pcWriteBuffer` 的长度。

`vTaskGetRunTimeStatistics()` 生成的输出示例如下所示
图 12.8。在输出中：

- 每行提供有关单个任务的信息。

- 第一列是任务名称。

- 第二列是任务在该任务中花费的时间
  以绝对值表示的运行状态。参见描述
  `ulRunTimeCounter`。

- 第三列是任务在该任务中花费的时间
  运行状态占目标完成后总时间的百分比
  启动。显示百分比次数的总和通常为
  低于预期的 100%，因为收集了统计数据并且
  使用向下舍入到最接近的整数计算进行计算
  整数值。


<a name="fig12.8" title="Figure 12.8 Example output generated by vTaskGetRunTimeStatistics()"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image89.png)   
***图 12.8*** *vTaskGetRunTimeStatistics() 生成的示例输出*

* * *

> 注意：   
> `vTaskGetRunTimeStatistics` 的旧版本是 `vTaskGetRunTimeStats`。 
> `vTaskGetRunTimeStats` 假定 pcWriteBuffer 的长度
> `configSTATS_BUFFER_MAX_LENGTH`。该函数只是为了向后兼容。 
> 新应用推荐使用`vTaskGetRunTimeStatistics`并提供长度 
> 明确 pcWriteBuffer。


 <a name="list12.9" title="Listing 12.9 The vTaskGetRunTimeStats() API function prototype"></a>

 ```c
 void vTaskGetRunTimeStats( signed char *pcWriteBuffer );
 ```
***清单 12.9*** *vTaskGetRunTimeStats() API 功能原型*

**vTaskGetRunTimeStats() 参数**

- `pcWriteBuffer`

指向写入格式化且人类可读表的字符缓冲区的指针。的 
    缓冲区必须足够大以容纳整个表，因为不执行边界检查。


### 12.5.7 生成和显示运行时统计数据，一个工作示例

此示例使用假设的 16 位定时器来生成 32 位定时器
运行时统计时钟。计数器被配置为生成
每次 16 位值达到最大值时中断
值—有效地创建溢出中断。中断服务
例程计算溢出发生的次数。

32 位值是通过使用溢出发生次数创建的
作为 32 位值的两个最高有效字节，以及当前
16 位计数器值作为 32 位的两个最低有效字节
值。中断服务程序的伪代码如清单所示
12.10。


<a name="list12.10" title="Listing 12.10 16-bit timer overflow interrupt handler used to count timer overflows"></a>

```c
void TimerOverflowInterruptHandler( void )
{
    /* Just count the number of interrupts. */
    ulOverflowCount++;

    /* Clear the interrupt. */
    ClearTimerInterrupt();
}
```
***清单 12.10*** *16 位定时器溢出中断处理程序用于对定时器溢出进行计数*

清单 12.11 显示了添加到 FreeRTOSConfig.h 以启用
运行时统计数据的收集。


<a name="list12.11" title="Listing 12.11 Macros added to FreeRTOSConfig.h to enable the collection of run-time statistics"></a>

```c
/* Set configGENERATE_RUN_TIME_STATS to 1 to enable collection of run-time 
   statistics. When this is done, both portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
   and portGET_RUN_TIME_COUNTER_VALUE() or 
   portALT_GET_RUN_TIME_COUNTER_VALUE(x) must also be defined. */
#define configGENERATE_RUN_TIME_STATS 1

/* portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() is defined to call the function 
   that sets up the hypothetical 16-bit timer (the function's implementation 
   is not shown). */
void vSetupTimerForRunTimeStats( void );
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()  vSetupTimerForRunTimeStats()

/* portALT_GET_RUN_TIME_COUNTER_VALUE() is defined to set its parameter to the
   current run-time counter/time value. The returned time value is 32-bits 
   long, and is formed by shifting the count of 16-bit timer overflows into 
   the top two bytes of a 32-bit number, then bitwise ORing the result with 
   the current 16-bit counter value. */
#define portALT_GET_RUN_TIME_COUNTER_VALUE( ulCountValue )                  \
{                                                                           \
    extern volatile unsigned long ulOverflowCount;                          \
                                                                            \
    /* Disconnect the clock from the counter so it does not change          \
       while its value is being used. */                                    \
    PauseTimer();                                                           \
                                                                            \
    /* The number of overflows is shifted into the most significant         \
       two bytes of the returned 32-bit value. */                           \
    ulCountValue = ( ulOverflowCount << 16UL );                             \
                                                                            \
    /* The current counter value is used as the two least significant       \
       bytes of the returned 32-bit value. */                               \
    ulCountValue |= ( unsigned long ) ReadTimerCount();                     \
                                                                            \
    /* Reconnect the clock to the counter. */                               \
    ResumeTimer();                                                          \
}
```
***清单 12.11*** *添加到 FreeRTOSConfig.h 的宏以启用运行时统计信息的收集*

清单 12.12 中所示的任务每 5 秒打印出收集的运行时统计信息。


<a name="list12.12" title="Listing 12.12 The task that prints out the collected run-time statistics"></a>

```c
#define RUN_TIME_STATS_STRING_BUFFER_LENGTH       512

/* For clarity, calls to fflush() have been omitted from this code listing. */
static void prvStatsTask( void *pvParameters )
{
    TickType_t xLastExecutionTime;

    /* The buffer used to hold the formatted run-time statistics text needs to
       be quite large. It is therefore declared static to ensure it is not
       allocated on the task stack. This makes this function non re-entrant. */
    static signed char cStringBuffer[ RUN_TIME_STATS_STRING_BUFFER_LENGTH ];

    /* The task will run every 5 seconds. */
    const TickType_t xBlockPeriod = pdMS_TO_TICKS( 5000 );

    /* Initialize xLastExecutionTime to the current time. This is the only
       time this variable needs to be written to explicitly. Afterwards it is 
       updated internally within the vTaskDelayUntil() API function. */
    xLastExecutionTime = xTaskGetTickCount();

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /* Wait until it is time to run this task again. */
        xTaskDelayUntil( &xLastExecutionTime, xBlockPeriod );

        /* Generate a text table from the run-time stats. This must fit into
           the cStringBuffer array. */
        vTaskGetRunTimeStatistics( cStringBuffer, RUN_TIME_STATS_STRING_BUFFER_LENGTH );

        /* Print out column headings for the run-time stats table. */
        printf( "\nTask\t\tAbs\t\t\t%%\n" );
        printf( "-------------------------------------------------------------\n" );

        /* Print out the run-time stats themselves. The table of data contains
           multiple lines, so the vPrintMultipleLines() function is called 
           instead of calling printf() directly. vPrintMultipleLines() simply 
           calls printf() on each line individually, to ensure the line 
           buffering works as expected. */ 
        vPrintMultipleLines( cStringBuffer );
    }
}
```
***清单 12.12*** *打印收集的运行时统计数据的任务*


## 12.6 跟踪钩子宏

跟踪宏是放置在程序内关键点的宏。
FreeRTOS 源代码。默认情况下，宏是空的，所以不
生成任何代码，并且没有运行时开销。通过重写
默认空实现，应用程序编写者可以：

- 将代码插入 FreeRTOS，无需修改 FreeRTOS 源代码
  文件。

- 通过任意方式输出详细的执行顺序信息
  在目标硬件上可用。跟踪宏出现在足够多的地方
  FreeRTOS 源代码中的位置允许它们用于
  创建完整且详细的调度程序活动跟踪和分析
  日志。


### 12.6.1 可用的跟踪挂钩宏

在这里详细说明每个宏会占用太多空间。下面的列表
详细说明了被认为对应用程序最有用的宏子集
作家。

下面列表中的许多描述都引用了一个名为
`pxCurrentTCB`。 `pxCurrentTCB` 是一个 FreeRTOS 私有变量，保存
运行状态下任务的句柄，可用于任何宏
从 FreeRTOS/Source/tasks.c 源文件调用。

**最常用的跟踪挂钩宏的选择**

- `traceTASK_INCREMENT_TICK(xTickCount)`

在滴答中断期间、滴答计数递增之前调用。 `xTickCount`参数 
  将新的刻度计数值传递到宏中。

- `traceTASK_SWITCHED_OUT()`

在选择运行新任务之前调用。此时，`pxCurrentTCB`包含了句柄 
  即将离开运行状态的任务。

- `traceTASK_SWITCHED_IN()`

选择要运行的任务后调用。此时，`pxCurrentTCB`包含了 
  任务即将进入运行状态。

- `traceBLOCKING_ON_QUEUE_RECEIVE(pxQueue)`
  

在当前正在执行的任务在尝试后进入阻塞状态之前立即调用 
  从空队列中读取，或尝试“获取”空信号量或互斥体。 `pxQueue`参数 
  将目标队列或信号量的句柄传递到宏中。

- `traceBLOCKING_ON_QUEUE_SEND(pxQueue)`
  

在当前正在执行的任务在尝试后进入阻塞状态之前立即调用 
  写入已满的队列。 `pxQueue`参数将目标队列的句柄传递到 
  宏。

- `traceQUEUE_SEND(pxQueue)`
  

从 `xQueueSend()`、`xQueueSendToFront()`、`xQueueSendToBack()` 或任何信号量内部调用 
  当队列发送或信号量“give”成功时，“give”函数。 `pxQueue` 参数传递 
  将目标队列或信号量的句柄放入宏中。

- `traceQUEUE_SEND_FAILED(pxQueue)`

从 `xQueueSend()`、`xQueueSendToFront()`、`xQueueSendToBack()` 或任何信号量内部调用 
  当队列发送或信号量“give”操作失败时，“give”函数。队列发送或信号量 
  如果队列已满并且在指定的任何块时间内保持满状态，则“give”将失败。 
  `pxQueue` 参数将目标队列或信号量的句柄传递到宏中。

- `traceQUEUE_RECEIVE(pxQueue)`

当队列接收或 
  信号量“take”成功。 `pxQueue`参数传递目标队列或信号量的句柄 
  进入宏。

- `traceQUEUE_RECEIVE_FAILED(pxQueue)`

当队列或信号量时，从 `xQueueReceive()` 或任何信号量“获取”函数中调用 
  接收操作失败。队列接收或信号量“获取”操作将失败，如果队列或信号量 
  为空，并且在任何指定的区块时间内保持为空。 `pxQueue` 参数传递 
  将目标队列或信号量的句柄放入宏中。

- `traceQUEUE_SEND_FROM_ISR(pxQueue)`

发送操作成功时从 `xQueueSendFromISR()` 内部调用。 `pxQueue`参数 
  将目标队列的句柄传递到宏中。

- `traceQUEUE_SEND_FROM_ISR_FAILED(pxQueue)`

当发送操作失败时从 `xQueueSendFromISR()` 内部调用。发送操作将失败 
  如果队列已经满了。 `pxQueue`参数将目标队列的句柄传递到 
  宏。

- `traceQUEUE_RECEIVE_FROM_ISR(pxQueue)`

接收操作成功时从 `xQueueReceiveFromISR()` 内部调用。 `pxQueue` 
  参数将目标队列的句柄传递到宏中。

- `traceQUEUE_RECEIVE_FROM_ISR_FAILED(pxQueue)`

当接收操作因队列已失败而从 `xQueueReceiveFromISR()` 内部调用 
  是空的。 `pxQueue` 参数将目标队列的句柄传递到宏中。

- `traceTASK_DELAY_UNTIL( xTimeToWake )`

在调用任务进入阻塞状态之前立即从 `xTaskDelayUntil()` 中调用。

- `traceTASK_DELAY()`

在调用任务进入阻塞状态之前立即从 `vTaskDelay()` 中调用。


### 12.6.2 定义跟踪挂钩宏

每个跟踪宏都有一个默认的空定义。默认定义
可以通过在中提供新的宏定义来覆盖
FreeRTOSConfig.h。如果跟踪宏定义变得很长或很复杂，
然后它们可以在一个新的头文件中实现，该头文件就是它本身
包含在 FreeRTOSConfig.h 中。

按照软件工程最佳实践，FreeRTOS
维持严格的数据隐藏政策。跟踪宏允许用户代码
添加到 FreeRTOS 源文件中，以便数据类型对
跟踪宏与应用程序代码可见的宏不同：

- 在 FreeRTOS/Source/tasks.c 源文件中，任务句柄是
  指向描述任务的数据结构的指针（任务的
  *任务控制块*，或*TCB*）。之外的
  FreeRTOS/Source/tasks.c 源文件任务句柄是一个指向
  无效。

- 在 FreeRTOS/Source/queue.c 源文件中，队列句柄是
  指向描述队列的数据结构的指针。之外的
  FreeRTOS/Source/queue.c 源文件中队列句柄是一个指向
  无效。

> *如果通常是私有的 FreeRTOS 数据，则需要格外小心
> 结构由跟踪宏直接访问，作为私有数据
> FreeRTOS 版本之间的结构可能会发生变化。*


### 12.6.3 FreeRTOS Aware 调试器插件

提供一些 FreeRTOS 感知的插件可用于
以下 IDE。此列表可能并不详尽：

![](https://cloud.rocketpi.club/cloud/image90.png)

- Eclipse (StateViewer)

- Eclipse (ThreadSpy)

- IAR

- ARM DS-5

- Atollic TrueStudio

- Microchip MPLAB

- iSYSTEM WinIDEA

- STM32CubeIDE
