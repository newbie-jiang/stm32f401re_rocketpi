# 6 软件定时器管理

## 6.1 章节简介和范围

软件定时器用于安排函数的执行时间
设定未来的时间，或以固定频率定期进行。的
由软件定时器执行的函数称为软件定时器的
回调函数。

软件定时器由以下器件实现并受其控制：
FreeRTOS 内核。它们不需要硬件支持，并且不
与硬件定时器或硬件计数器相关。

请注意，符合 FreeRTOS 使用创新理念
设计确保最大效率，软件定时器不使用任何
处理时间，除非软件定时器回调函数实际上是
执行。

软件定时器功能是可选的。包括软件定时器
功能：

1. 构建 FreeRTOS 源文件 FreeRTOS/Source/timers.c 作为
   你的项目。

2. 在应用程序的 FreeRTOSConfig.h 头文件中定义下面详述的常量：

- `configUSE_TIMERS`

将 FreeRTOSConfig.h 中的 `configUSE_TIMERS` 设置为 1。

- `configTIMER_TASK_PRIORITY`

设置定时器服务任务的优先级在 0 and ( `configMAX_PRIORITIES` - 1 ) 之间。

- `configTIMER_QUEUE_LENGTH`

设置定时器命令队列在任一时刻可以容纳的未处理命令的最大数量。

- `configTIMER_TASK_STACK_DEPTH`

设置分配给定时器服务任务的stack (in words, not bytes)的大小。

### 6.1.1 范围

本章内容包括：

- 软件定时器与定时器相比的特点
  任务的特征。
- RTOS 守护程序任务。
- 定时器命令队列。
- 一次性软件定时器和周期性软件定时器之间的区别
  软件定时器。
- 如何创建、启动、重置和更改软件计时器的周期。


## 6.2 软件定时器回调函数

软件定时器回调函数以 C 函数的形式实现。的
它们唯一特别的是它们的原型，它必须返回
void，并将软件计时器的句柄作为其唯一参数。的
回调函数原型如清单 6.1 所示。


<a name="list" title="Listing 6.1 The software timer callback function prototype"></a>

```c
void ATimerCallback( TimerHandle_t xTimer );
```
***清单 6.1*** *软件定时器回调函数原型*

软件定时器回调函数从头到尾执行，然后退出
以正常方式。它们应该保持较短，并且不得进入
封锁状态。

> *注意：正如我们将看到的，软件定时器回调函数在
> FreeRTOS 运行时自动创建的任务上下文
> 调度程序已启动。因此，软件定时器至关重要
> 回调函数永远不会调用 FreeRTOS API 函数，这会导致
> 调用任务进入阻塞状态。调用函数就可以了
> 例如 `xQueueReceive()`，但前提是函数为 `xTicksToWait`
> parameter (which specifies the function's block time) 设置为 0。
> 不能调用 `vTaskDelay()` 等函数，就像调用 `vTaskDelay()` 一样
> 始终将调用任务置于阻塞状态。*


## 6.3 软件定时器的属性和状态

### 6.3.1 软件定时器的周期

软件定时器的“周期”是软件定时器被执行之间的时间
启动，软件定时器的回调函数执行。

### 6.3.2 一次性和自动重新加载计时器

软件定时器有两种类型：

1. 一次性定时器

一旦启动，一次性定时器将执行其回调函数
   仅一次。一次性计时器可以手动重新启动，但不会
   自行重启。

1.自动重新加载计时器

一旦启动，自动重新加载计时器每次都会重新启动
   过期，导致其回调函数定期执行。

图 6.1 显示了一次性定时器和一次性定时器之间的行为差异
自动重新加载计时器。垂直虚线标记了时间
发生滴答中断。


<a name="fig6.1" title="Figure 6.1 The difference in behavior between one-shot and auto-reload software timers"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image38.png)
***图 6.1*** *一次性和自动重新加载软件计时器之间的行为差​​异*

* * *

参见图6.1：

- 定时器1

定时器 1 是一个单次定时器，周期为 6 个滴答声。它是
  从时间 t1 开始，因此其回调函数在 6 个时钟周期后执行，
  在时间t7。由于定时器 1 是一次性定时器，因此它的回调函数
  不再执行。

- 定时器2

定时器 2 是一个自动重载定时器，周期为 5 个滴答声。它是
  从时间 t1 开始，因此其回调函数每 5 个周期执行一次
  时间 t1 之后。在图 6.1 中，时间为 t6、t11 和 t16。


### 6.3.3 软件定时器状态

软件定时器可以处于以下两种状态之一：

- 休眠

存在休眠软件定时器，并且可以通过其句柄引用，
  但没有运行，所以它的回调函数不会执行。

- 跑步

正在运行的软件计时器将在经过一段时间后执行其回调函数
  自软件定时器进入以来已经过去了等于其周期的时间
  运行状态，或自上次重置软件计时器以来。

图 6.2 和图 6.3 显示了之间可能的转换
自动重新加载计时器和一次性计时器的休眠和运行状态
分别。两个图之间的主要区别在于状态
定时器到期后输入；自动重新加载计时器执行其
然后回调函数重新进入Running状态，一次性定时器
执行其回调函数然后进入休眠状态。


<a name="fig6.2" title="Figure 6.2 Auto-reload software timer states and transitions"></a>
<a name="fig6.3" title="Figure 6.3 One-shot software timer states and transitions"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image39.png)
***图 6.2*** *自动重新加载软件定时器状态和转换*

![](https://cloud.rocketpi.club/cloud/image40.png)
***图 6.3*** *一次性软件定时器状态和转换*

* * *

`xTimerDelete()` API 函数删除定时器。定时器可以删除
随时。函数原型如清单 6.2 所示。


<a name="list6.2" title="Listing 6.2 The xTimerDelete() API function prototype"></a>

```c
BaseType_t xTimerDelete( TimerHandle_t xTimer, TickType_t xTicksToWait );
```
***清单 6.2*** *xTimerDelete() API 函数原型*


**xTimerDelete()参数及返回值**

- `xTimer`

正在删除的计时器的句柄。

- `xTicksToWait`

指定调用任务应保留的时间（以刻度为单位）
  处于Blocked状态等待删除命令成功
  发送到计时器命令队列，如果队列已满，则
  xTimerDelete() 被称为。  如果 xTimerDelete() 则忽略 xTicksToWait
  在调度程序启动之前调用。

- 返回值

有两种可能的返回值：

- `pdPASS`

如果命令成功发送到，将返回 `pdPASS`
     定时器命令队列。

- `pdFAIL`

如果删除命令无法发送到，将返回 `pdFAIL`
    即使在 xBlockTime 滴答过去之后，计时器命令队列仍然存在。


## 6.4 软件定时器的上下文

### 6.4.1 RTOS Daemon (Timer Service) 任务

所有软件定时器回调函数都在相同的上下文中执行
RTOS daemon (or 'timer service') 任务[^10]。

[^10]：该任务过去被称为“定时服务任务”，因为
原本只是用来执行软件定时器回调
功能。现在相同的任务也用于其他目的，所以它
以更通用的名称“RTOS 守护程序任务”而闻名。

守护进程任务是创建的标准 FreeRTOS 任务
当调度程序启动时自动执行。它的优先级和堆栈大小
由 `configTIMER_TASK_PRIORITY` 设置和
`configTIMER_TASK_STACK_DEPTH` 编译时配置常量
分别。这两个常量均在 FreeRTOSConfig.h 中定义。

软件定时器回调函数不得调用 FreeRTOS API 函数
这将导致调用任务进入阻塞状态，至于
这样做将导致守护任务进入阻塞状态。


### 6.4.2 定时器命令队列

软件定时器 API 函数将命令从调用任务发送到
称为“计时器命令队列”的队列上的守护程序任务。这是显示的
如图 6.4 所示。命令的示例包括“启动计时器”、“停止计时器”
计时器”和“重置计时器”。

定时器命令队列是创建的标准FreeRTOS队列
当调度程序启动时自动执行。定时器的长度
命令队列由 `configTIMER_QUEUE_LENGTH` 编译时设置
FreeRTOSConfig.h 中的配置常量。


<a name="fig6.4" title="Figure 6.4 The timer command queue being used by a software timer API function to communicate with the RTOS daemon task"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image41.png)
***图 6.4*** *软件计时器 API 功能使用计时器命令队列与 RTOS 守护程序任务进行通信*

* * *


### 6.4.3 守护进程任务调度

守护进程任务的调度与任何其他 FreeRTOS 任务一样；只会
处理命令，或执行定时器回调函数，当它是
能够运行的最高优先级任务。图6.5和图6.6
演示 `configTIMER_TASK_PRIORITY` 设置如何影响
执行模式。

图 6.5 显示了守护进程优先级时的执行模式
任务的优先级低于调用 `xTimerStart()` API 的任务的优先级
功能。


<a name="fig6.5" title="Figure 6.5 The execution pattern when the priority of a task calling xTimerStart() is above the priority of the daemon task"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image42.png)
***图6.5*** *调用xTimerStart()的任务优先级高于守护任务优先级时的执行模式*

* * *

参见图6.5，其中任务1的优先级高于
守护任务的优先级，守护任务的优先级为
高于空闲任务的优先级：

1.在时间t1

任务1处于运行状态，守护任务处于
    封锁状态。

如果命令发送到，守护进程任务将离开阻塞状态
    计时器命令队列，在这种情况下它将处理命令，
    或者如果软件定时器到期，在这种情况下它将执行
    软件定时器的回调函数。

1.在时间t2

任务 1 调用 `xTimerStart()`。

`xTimerStart()`向定时器命令队列发送命令，导致
   守护进程任务离开阻塞状态。任务 1 的优先级
   比守护任务的优先级高，所以守护任务
   不抢占任务 1。

任务1仍处于Running状态，守护任务已离开
   Blocked 状态并进入 Ready 状态。

1.在时间t3

任务 1 完成 `xTimerStart()` API 函数的执行。任务1
   从函数开始到函数结束执行了`xTimerStart()`
   该功能，无需离开运行状态。

1.在时间t4

任务 1 调用 API 函数，导致其进入阻塞状态
   状态。守护进程任务现在是Ready中优先级最高的任务
   状态，因此调度程序选择守护任务作为要进入的任务
   运行状态。然后守护进程开始处理
   命令由任务 1 发送到定时器命令队列。

> *注：软件定时器启动的时间将
   > 过期时间是从“启动计时器”命令发出的时间开始计算的
   > 发送到定时器命令队列——不是从时间开始计算
   > 守护进程任务从计时器接收到“启动计时器”命令
   > 命令队列。*

1.在时间t5

守护进程任务已完成处理发送给它的命令
   任务1，并尝试从定时器命令接收更多数据
   队列。定时器命令队列为空，因此守护进程任务
   重新进入阻塞状态。守护进程任务将离开阻塞状态
   如果命令被发送到定时器命令队列，或者如果
   软件定时器到期。

空闲任务现在是就绪状态中优先级最高的任务，
   所以调度器选择Idle任务作为进入任务
   运行状态。

图 6.6 显示了与图 6.5 类似的场景，但是这个
守护任务的优先级高于本任务的优先级的时间
调用 `xTimerStart()`。


<a name="fig6.6" title="Figure 6.6 The execution pattern when the priority of a task calling xTimerStart() is below the priority of the daemon task"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image43.png)
***图6.6*** *调用xTimerStart()的任务优先级低于守护任务优先级时的执行模式*

* * *

参见图6.6，其中守护任务的优先级为
任务1的优先级高于任务1的优先级，任务1的优先级为
高于空闲任务的优先级：

1.在时间t1

和之前一样，任务1处于运行状态，守护任务处于
   阻塞状态。

1.在时间t2

任务 1 调用 `xTimerStart()`。

`xTimerStart()`向定时器命令队列发送命令，导致
   守护进程任务离开阻塞状态。的优先级
   守护进程任务的优先级高于任务1，因此调度程序
   选择守护任务作为进入运行状态的任务。

任务 1 在完成之前被守护任务抢占
   执行 `xTimerStart()` 函数，现在处于就绪状态。

守护任务开始处理发送给定时器的命令
   任务 1 的命令队列。

1.在时间t3

守护进程任务已完成处理发送给它的命令
   任务1，并尝试从定时器命令接收更多数据
   队列。定时器命令队列为空，因此守护进程任务
   重新进入阻塞状态。

任务 1 现在是就绪状态中优先级最高的任务，因此
   调度器选择Task 1作为进入Running状态的任务。

1.在时间t4

任务 1 在完成之前被守护任务抢占
   执行 `xTimerStart()` 函数，并且仅执行 exits (returns from)
   `xTimerStart()` 重新进入运行状态后。

1.在时间t5

任务 1 调用 API 函数，导致其进入阻塞状态
   状态。空闲任务现在是就绪任务中优先级最高的任务
   状态，所以调度器选择Idle任务作为进入的任务
   运行状态。

在图 6.5 所示的场景中，任务 1 发送一个
命令到定时器命令队列，守护进程任务接收和
处理命令。在图6.6所示的场景中，守护进程
任务之前已接收并处理了任务 1 发送给它的命令
任务 1 从发送命令的函数返回。

发送到定时器命令队列的命令包含时间戳。时间
时间戳用于计算命令之间经过的任何时间
由应用程序任务发送，并且正在处理相同的命令
通过守护进程任务。例如，如果“启动计时器”命令发送到
启动一个周期为 10 个刻度的计时器，时间戳用于
确保正在启动的计时器在命令执行后 10 个周期内到期
已发送，不是守护程序任务处理命令后 10 个时钟周期。


## 6.5 创建并启动软件定时器

### 6.5.1 xTimerCreate() API 功能

FreeRTOS 还包含 `xTimerCreateStatic()` 函数，该函数
在编译时静态分配创建计时器所需的内存
time：软件定时器必须在使用之前显式创建。

软件定时器由 `TimerHandle_t` 类型的变量引用。
`xTimerCreate()` 用于创建软件定时器并返回
`TimerHandle_t` 来引用它创建的软件计时器。软件
定时器是在休眠状态下创建的。

软件定时器可以在调度程序运行之前创建，或者从
调度程序启动后的任务。

[Section 2.5: Data Types and Coding Style Guide](ch02_zh.md#25-data-types-and-coding-style-guide) 描述了所使用的数据类型和命名约定。


<a name="list6.3" title="Listing 6.3 The xTimerCreate() API function prototype"></a>

```c
TimerHandle_t xTimerCreate( const char * const pcTimerName,
                            const TickType_t xTimerPeriodInTicks,
                            const BaseType_t xAutoReload,
                            void * const pvTimerID,
                            TimerCallbackFunction_t pxCallbackFunction );
```
***清单 6.3*** *xTimerCreate() API 函数原型*

**xTimerCreate()参数及返回值**

- `pcTimerName`

计时器的描述性名称。 FreeRTOS 不使用此功能
  无论如何。它纯粹是作为调试辅助工具而包含在内。识别定时器
  通过人类可读的名称比尝试识别它要简单得多
  通过其手柄。

- `xTimerPeriodInTicks`

计时器的周期以刻度为单位指定。 `pdMS_TO_TICKS()` 宏可以
  用于将以毫秒为单位的时间转换为时间
  以刻度指定。不能为 0。

- `xAutoReload`

将 `xAutoReload` 设置为 `pdTRUE` 以创建自动重新加载计时器。套装
  `xAutoReload` 至 `pdFALSE` 创建一次性定时器。

- `pvTimerID`

每个软件定时器都有一个 ID 值。 ID 是一个空指针，
  应用程序编写者可以将其用于任何目的。 ID 是
  当同一个回调函数被多个用户使用时特别有用
  一个软件定时器，因为它可用于提供定时器特定的存储。
  本节中的示例演示了定时器 ID 的使用
  章。

`pvTimerID` 为正在创建的任务的 ID 设置初始值。

- `pxCallbackFunction`

软件定时器回调函数只是 C 函数，
  符合清单 6.1 所示的原型。 `pxCallbackFunction`
  参数是指向函数的指针（实际上，只是函数
  name) 用作软件定时器的回调函数
  创建的。

- 返回值

如果返回NULL，则无法创建软件定时器
  因为 FreeRTOS 没有足够的可用堆内存
  分配必要的数据结构。

如果返回非 NULL 值，则表明软件定时器已
  已创建成功。返回值是该对象的句柄
  创建的计时器。

第 3 章提供了有关堆内存管理的更多信息。


### 6.5.2 xTimerStart() API 功能

`xTimerStart()` 用于启动处于休眠状态的软件定时器
状态，或 reset (re-start) 处于运行状态的软件定时器
状态。 `xTimerStop()` 用于停止位于
运行状态。停止软件定时器与转换相同
定时器进入休眠状态。

`xTimerStart()` 可以在调度程序启动之前调用，但是当
完成后，软件定时器将不会真正启动，直到时间
调度程序启动的时间。

> *注意：切勿从中断服务程序中调用 `xTimerStart()`。的
> 应使用中断安全版本 `xTimerStartFromISR()`
> 地点。*


<a name="list6.4" title="Listing 6.4 The xTimerStart() API function prototype"></a>

```c
BaseType_t xTimerStart( TimerHandle_t xTimer, TickType_t xTicksToWait );
```
***清单 6.4*** *xTimerStart() API 函数原型*


**xTimerStart()参数及返回值**

- `xTimer`

正在启动或重置的软件定时器的句柄。手柄
  将从调用用于创建的 `xTimerCreate()` 返回
  软件定时器。

- `xTicksToWait`

`xTimerStart()` 使用定时器命令队列发送“启动一个
  向守护程序任务发送“timer”命令。 `xTicksToWait` 指定最大
  调用任务应保持阻塞状态的时间
  等待计时器命令队列上的空间变得可用，应该
  队列已经满了。

如果 `xTicksToWait` 为零并且 `xTimerStart()` 将立即返回
  定时器命令队列已满。

区块时间以滴答周期为单位指定，因此它的绝对时间
  表示取决于滴答频率。宏 `pdMS_TO_TICKS()`
  可用于将以毫秒为单位的时间转换为时间
  以刻度指定。

如果 `FreeRTOSConfig.h` 中的 `INCLUDE_vTaskSuspend` 设置为 1，则设置
  `xTicksToWait` 到 `portMAX_DELAY` 将导致调用任务剩余
  在Blocked状态indefinitely (without a timeout)等待空间
  在计时器命令队列中变得可用。

如果在调度程序启动之前调用 `xTimerStart()`，则
  `xTicksToWait` 的值被忽略，并且 `xTimerStart()` 的行为就像
  `xTicksToWait` 已设置为零。

- 返回值

有两种可能的返回值：

- `pdPASS`

仅当“启动计时器”命令被执行时，才会返回 `pdPASS`
    成功发送到定时器命令队列。

如果守护任务的优先级高于任务的优先级
    称为 `xTimerStart()`，那么调度程序将确保启动
    命令在 `xTimerStart()` 返回之前处理。这是因为
    守护进程任务将抢占调用 `xTimerStart()` 的任务
    定时器命令队列中有数据。

如果区块时间为 specified (`xTicksToWait` was not zero)，那么它是
    可能调用任务被置于阻塞状态以等待
    定时器命令队列中函数之前的可用空间
    返回，但数据已成功写入定时器命令队列
    在区块时间到期之前。

- `pdFAIL`

如果无法执行“启动计时器”命令，将返回 `pdFAIL`
    写入定时器命令队列，因为该队列已经
    满。

如果区块时间为 specified (`xTicksToWait` was not zero) 则
    调用任务将被置于阻塞状态等待
    守护进程任务在定时器命令队列中腾出空间，但是
    指定的区块时间在此之前已过期。


<a name="example6.1" title="Example 6.1 Creating one-shot and auto-reload timers"></a>
---
***示例 6.1*** *创建一次性和自动重新加载计时器*

---

此示例创建并启动一次性计时器和自动重新加载
计时器——如清单 6.5 所示。


<a name="list6.5" title="Listing 6.5 Creating and starting the timers used in Example 6.1"></a>

```c
/* The periods assigned to the one-shot and auto-reload timers are 3.333
   second and half a second respectively. */
#define mainONE_SHOT_TIMER_PERIOD pdMS_TO_TICKS( 3333 )
#define mainAUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 500 )

int main( void )
{
    TimerHandle_t xAutoReloadTimer, xOneShotTimer;
    BaseType_t xTimer1Started, xTimer2Started;

    /* Create the one shot timer, storing the handle to the created timer in
       xOneShotTimer. */
    xOneShotTimer = xTimerCreate(
        /* Text name for the software timer - not used by FreeRTOS. */
                                  "OneShot",
        /* The software timer's period in ticks. */
                                   mainONE_SHOT_TIMER_PERIOD,
        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                                   pdFALSE,
        /* This example does not use the timer id. */
                                   0,
        /* Callback function to be used by the software timer being created. */
                                   prvOneShotTimerCallback );

    /* Create the auto-reload timer, storing the handle to the created timer
       in xAutoReloadTimer. */
    xAutoReloadTimer = xTimerCreate(
        /* Text name for the software timer - not used by FreeRTOS. */
                                     "AutoReload",
        /* The software timer's period in ticks. */
                                     mainAUTO_RELOAD_TIMER_PERIOD,
        /* Setting uxAutoRealod to pdTRUE creates an auto-reload timer. */
                                     pdTRUE,
        /* This example does not use the timer id. */
                                     0,
        /* Callback function to be used by the software timer being created. */
                                     prvAutoReloadTimerCallback );

    /* Check the software timers were created. */
    if( ( xOneShotTimer != NULL ) && ( xAutoReloadTimer != NULL ) )
    {
        /* Start the software timers, using a block time of 0 (no block time).
           The scheduler has not been started yet so any block time specified
           here would be ignored anyway. */
        xTimer1Started = xTimerStart( xOneShotTimer, 0 );
        xTimer2Started = xTimerStart( xAutoReloadTimer, 0 );

        /* The implementation of xTimerStart() uses the timer command queue,
           and xTimerStart() will fail if the timer command queue gets full.
           The timer service task does not get created until the scheduler is
           started, so all commands sent to the command queue will stay in the
           queue until after the scheduler has been started. Check both calls
           to xTimerStart() passed. */
        if( ( xTimer1Started == pdPASS ) && ( xTimer2Started == pdPASS ) )
        {
            /* Start the scheduler. */
            vTaskStartScheduler();
        }
    }

    /* As always, this line should not be reached. */
    for( ;; );
}
```
***清单 6.5*** *创建并启动示例 6.1 中使用的计时器*


计时器的回调函数每次只打印一条消息
叫。一次性定时器回调函数的实现是
如清单 6.6 所示。自动重载定时器的实现
回调函数如清单 6.7 所示。


<a name="list6.5" title="Listing 6.6 The callback function used by the one-shot timer in Example 6.1"></a>

```c
static void prvOneShotTimerCallback( TimerHandle_t xTimer )
{
    TickType_t xTimeNow;

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* Output a string to show the time at which the callback was executed. */
    vPrintStringAndNumber( "One-shot timer callback executing", xTimeNow );

    /* File scope variable. */
    ulCallCount++;
}
```
***清单 6.6*** *示例 6.1 中的一次性定时器使用的回调函数*


<a name="list6.7" title="Listing 6.7 The callback function used by the auto-reload timer in Example 6.1"></a>

```c
static void prvAutoReloadTimerCallback( TimerHandle_t xTimer )
{
    TickType_t xTimeNow;

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* Output a string to show the time at which the callback was executed. */
    vPrintStringAndNumber( "Auto-reload timer callback executing", xTimeNow);

    ulCallCount++;
}
```
***清单 6.7*** *示例 6.1 中自动重载计时器使用的回调函数*

执行该示例会产生如图 6.7 所示的输出。图6.7
显示自动重载计时器的回调函数以固定的执行
500 个刻度周期（`mainAUTO_RELOAD_TIMER_PERIOD` 设置为 500）
代码清单 6.5)，一次性定时器的回调函数只执行
一次，当刻度计数为 3333 时（`mainONE_SHOT_TIMER_PERIOD` 设置为
清单 6.5 中的 3333）。


<a name="fig6.7" title="Figure 6.7 The output produced when Example 6.1 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image44.jpg)
***图 6.7*** *执行例 6.1 时产生的输出*

* * *


## 6.6 定时器 ID

每个软件定时器都有一个ID，这是一个可以被使用的标签值
出于任何目的的应用程序编写者。 ID 存储在 void 中
pointer (`void *`)，因此可以直接存储一个整数值，指向任意
其他对象，或用作函数指针。

当软件定时器为 ID 时，初始值被分配
创建后，可以使用 `vTimerSetTimerID()` 更新 ID
API函数，并使用`pvTimerGetTimerID()` API函数进行查询。

与其他软件定时器 API 功能不同，`vTimerSetTimerID()` 和
`pvTimerGetTimerID()` 直接访问软件定时器——它们不发送
发送到定时器命令队列的命令。


### 6.6.1 vTimerSetTimerID() API 功能


<a name="list6.8" title="Listing 6.8 The vTimerSetTimerID() API function prototype"></a>

```c
void vTimerSetTimerID( const TimerHandle_t xTimer, void *pvNewID );
```
***清单 6.8*** *vTimerSetTimerID() API 函数原型*


**vTimerSetTimerID() 参数**

- `xTimer`

软件定时器的句柄正在更新为新的 ID 值。
  句柄将从对 `xTimerCreate()` 的调用中返回
  创建软件定时器。

- `pvNewID`

软件定时器的 ID 将设置的值。


### 6.6.2 pvTimerGetTimerID() API 功能


<a name="list6.9" title="Listing 6.9 The pvTimerGetTimerID() API function prototype"></a>

```c
void *pvTimerGetTimerID( const TimerHandle_t xTimer );
```
***清单 6.9*** *pvTimerGetTimerID() API 函数原型*


**pvTimerGetTimerID()参数及返回值**

- `xTimer`

正在查询的软件定时器的句柄。手柄上会有
  已从对用于创建的 `xTimerCreate()` 的调用返回
  软件定时器。

- 返回值

被查询的软件定时器的ID。


<a name="example6.2" title="Example 6.2 Using the callback function parameter and the software timer ID"></a>
---
***示例 6.2*** *使用回调函数参数和软件定时器 ID*

---

同一回调函数可以分配给多个软件
计时器。完成后，回调函数参数用于
确定哪个软件定时器到期。

例 6.1 使用了两个独立的回调函数；一个回调函数
由一次性定时器使用，并使用另一个回调函数
通过自动重新加载计时器。例 6.2 创建了类似的功能
由示例 6.1 创建，但分配了一个回调函数
两个软件定时器。

例 6.2 使用的 `main()` 函数与 `main()` 几乎相同
例 6.1 中使用的函数。唯一的区别是软件所在的位置
定时器被创建。这种差异如清单 6.10 所示，其中
`prvTimerCallback()` 用作两个定时器的回调函数。


<a name="list6.10" title="Listing 6.10 Creating the timers used in Example 6.2"></a>

```c
/* Create the one shot timer software timer, storing the handle in
   xOneShotTimer. */
xOneShotTimer = xTimerCreate( "OneShot",
                              mainONE_SHOT_TIMER_PERIOD,
                              pdFALSE,
                              /* The timer's ID is initialized to NULL. */
                              NULL,
                              /* prvTimerCallback() is used by both timers. */
                              prvTimerCallback );

/* Create the auto-reload software timer, storing the handle in
   xAutoReloadTimer */
xAutoReloadTimer = xTimerCreate( "AutoReload",
                                 mainAUTO_RELOAD_TIMER_PERIOD,
                                 pdTRUE,
                                 /* The timer's ID is initialized to NULL. */
                                 NULL,
                                 /* prvTimerCallback() is used by both timers. */
                                 prvTimerCallback );
```
***清单 6.10*** *创建示例 6.2 中使用的计时器*

当任一定时器到期时，`prvTimerCallback()` 将执行。的
`prvTimerCallback()` 的实现使用函数的参数来
确定是否由于一次性计时器到期而调用它，或者
因为自动重新加载计时器已过期。

`prvTimerCallback()` 还演示了如何使用软件定时器 ID 作为
定时器特定存储；每个软件定时器都会记录数字
它在其自己的 ID 中已过期的次数，并且自动重新加载计时器使用
计数在第五次执行时自行停止。

`prvTimerCallback()` 的实现如清单 6.9 所示。


<a name="list6.11" title="Listing 6.11 The timer callback function used in Example 6.2"></a>

```c
static void prvTimerCallback( TimerHandle_t xTimer )
{
    TickType_t xTimeNow;
    uint32_t ulExecutionCount;

    /* A count of the number of times this software timer has expired is
       stored in the timer's ID. Obtain the ID, increment it, then save it as
       the new ID value. The ID is a void pointer, so is cast to a uint32_t. */
    ulExecutionCount = ( uint32_t ) pvTimerGetTimerID( xTimer );
    ulExecutionCount++;
    vTimerSetTimerID( xTimer, ( void * ) ulExecutionCount );

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* The handle of the one-shot timer was stored in xOneShotTimer when the
       timer was created. Compare the handle passed into this function with
       xOneShotTimer to determine if it was the one-shot or auto-reload timer
       that expired, then output a string to show the time at which the
       callback was executed. */
    if( xTimer == xOneShotTimer )
    {
        vPrintStringAndNumber( "One-shot timer callback executing", xTimeNow );
    }
    else
    {
        /* xTimer did not equal xOneShotTimer, so it must have been the
           auto-reload timer that expired. */
        vPrintStringAndNumber( "Auto-reload timer callback executing", xTimeNow);

        if( ulExecutionCount == 5 )
        {
            /* Stop the auto-reload timer after it has executed 5 times. This
               callback function executes in the context of the RTOS daemon
               task so must not call any functions that might place the daemon
               task into the Blocked state. Therefore a block time of 0 is
               used. */
            xTimerStop( xTimer, 0 );
        }
    }
}
```
***清单 6.11*** *示例 6.2 中使用的定时器回调函数*


例 6.2 产生的输出如图 6.8 所示。可见
自动重新加载计时器仅执行五次。


<a name="fig6.8" title="Figure 6.8 The output produced when Example 6.2 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image45.jpg)
***图 6.8*** *执行例 6.2 时产生的输出*

* * *


## 6.7 改变定时器的周期

每个官方 FreeRTOS 端口都提供一个或多个示例
项目。大多数示例项目都是自检的，并且使用 LED
提供项目状态的视觉反馈；如果自检有
如果自检始终通过，则 LED 会缓慢切换
失败，然后 LED 快速切换。

一些示例项目在任务中执行自检，并使用
`vTaskDelay()` 功能用于控制 LED 的切换速率。
其他示例项目在软件计时器中执行自检
回调函数，并使用定时器的周期来控制速率
LED 进行切换。


### 6.7.1 xTimerChangePeriod() API 功能

使用 `xTimerChangePeriod()` 函数更改软件定时器的周期。

如果使用 `xTimerChangePeriod()` 来改变定时器的周期，
已经在运行，那么计时器将使用新的周期值
重新计算其到期时间。重新计算的到期时间是相对于
当 `xTimerChangePeriod()` 被调用时，与计时器被调用的时间无关
最初开始。

如果使用 `xTimerChangePeriod()` 来改变定时器的周期，
在休眠state (a timer that is not running)，那么定时器将
计算到期时间，并转换到运行状态（计时器
将开始运行）。

> *注意：切勿从中断服务中调用 `xTimerChangePeriod()`
> 常规。中断安全版本 `xTimerChangePeriodFromISR()` 应
> 在其位置使用。*


<a name="list6.12" title="Listing 6.12 The xTimerChangePeriod() API function prototype"></a>

```c
BaseType_t xTimerChangePeriod( TimerHandle_t xTimer,
                               TickType_t xNewPeriod,
                               TickType_t xTicksToWait );
```
***清单 6.12*** *xTimerChangePeriod() API 函数原型*


**xTimerChangePeriod()参数及返回值**

- `xTimer`

软件定时器的句柄正在更新为新的周期
  值。句柄将从调用中返回
  `xTimerCreate()` 用于创建软件定时器。

- `xTimerPeriodInTicks`

软件计时器的新周期，以刻度为单位指定。的
  `pdMS_TO_TICKS()` 宏可用于转换指定的时间
  以毫秒为单位指定的时间。

- `xTicksToWait`

`xTimerChangePeriod()` 使用定时器命令队列发送
  向守护程序任务发出“更改周期”命令。 `xTicksToWait` 指定
  调用任务应保留在阻塞状态的最长时间
  状态等待计时器命令队列上的空间变得可用，
  如果队列已经满了。

如果 `xTicksToWait` 为零，`xTimerChangePeriod()` 将立即返回
  并且定时器命令队列已满。

宏 `pdMS_TO_TICKS()` 可用于转换指定的时间
  以毫秒为单位指定的时间。

如果 FreeRTOSConfig.h 中的 `INCLUDE_vTaskSuspend` 设置为 1，则设置
  `xTicksToWait` 到 `portMAX_DELAY` 将导致调用任务剩余
  在Blocked状态indefinitely (without a timeout)等待空间
  在计时器命令队列中变得可用。

如果在调度程序之前调用 `xTimerChangePeriod()`
  启动后，`xTicksToWait` 的值将被忽略，并且
  `xTimerChangePeriod()` 的行为就像 `xTicksToWait` 已设置为零一样。

- 返回值

有两种可能的返回值：

- `pdPASS`

仅当数据成功发送到 `pdPASS` 时才会返回
    定时器命令队列。

如果区块时间为 specified (`xTicksToWait` was not zero)，那么它是
    可能调用任务被置于阻塞状态以等待
    定时器命令队列中函数之前的可用空间
    返回，但数据已成功写入定时器命令队列
    在区块时间到期之前。

- `pdFAIL`

如果无法执行“更改周期”命令，将返回 `pdFAIL`
    写入定时器命令队列，因为该队列已经
    满。

如果区块时间为 specified (`xTicksToWait` was not zero) 则
    调用任务将被置于阻塞状态等待
    守护进程任务在队列中腾出空间，但指定阻塞时间
    在此之前已过期。

清单 6.13 显示了包含自检功能的 FreeRTOS 示例如何
软件定时器回调函数中的功能使用
`xTimerChangePeriod()` 增加 LED 切换的速率
自检失败。执行自检的软件定时器是
称为“检查计时器”。


<a name="list6.13" title="Listing 6.13 Using xTimerChangePeriod()"></a>

```c
/* The check timer is created with a period of 3000 milliseconds, resulting
   in the LED toggling every 3 seconds. If the self-checking functionality
   detects an unexpected state, then the check timer's period is changed to
   just 200 milliseconds, resulting in a much faster toggle rate. */
const TickType_t xHealthyTimerPeriod = pdMS_TO_TICKS( 3000 );
const TickType_t xErrorTimerPeriod = pdMS_TO_TICKS( 200 );

/* The callback function used by the check timer. */
static void prvCheckTimerCallbackFunction( TimerHandle_t xTimer )
{
    static BaseType_t xErrorDetected = pdFALSE;

    if( xErrorDetected == pdFALSE )
    {
        /* No errors have yet been detected. Run the self-checking function
           again. The function asks each task created by the example to report
           its own status, and also checks that all the tasks are actually
           still running (and so able to report their status correctly). */
        if( CheckTasksAreRunningWithoutError() == pdFAIL )
        {
            /* One or more tasks reported an unexpected status. An error might
               have occurred. Reduce the check timer's period to increase the
               rate at which this callback function executes, and in so doing
               also increase the rate at which the LED is toggled. This
               callback function is executing in the context of the RTOS daemon
               task, so a block time of 0 is used to ensure the Daemon task
               never enters the Blocked state. */
            xTimerChangePeriod(
                  xTimer,            /* The timer being updated */
                  xErrorTimerPeriod, /* The new period for the timer */
                  0 );               /* Do not block when sending this command */

            /* Latch that an error has already been detected. */
            xErrorDetected = pdTRUE;
        }
    }

    /* Toggle the LED. The rate at which the LED toggles will depend on how
       often this function is called, which is determined by the period of the
       check timer. The timer's period will have been reduced from 3000ms to
       just 200ms if CheckTasksAreRunningWithoutError() has ever returned
       pdFAIL. */
    ToggleLED();
}
```
***清单 6.13*** *使用 xTimerChangePeriod()*

## 6.8 重置软件定时器

重置软件定时器是指重新启动定时器；计时器的
到期时间将根据计时器重置时间重新计算，
而不是计时器最初启动时。这被证明
通过图 6.9，显示了一个周期为 6 的定时器正在启动，
然后重置两次，最终过期并执行回调
功能。


<a name="fig6.9" title="Figure 6.9 Starting and resetting a software timer that has a period of 6 ticks"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image46.png)
***图 6.9*** *启动和重置周期为 6 个滴答的软件定时器*

* * *

参见图6.9：

- 定时器 1 在时间 t1 启动。它的周期为 6，所以时间为
  它将执行其回调函数最初是计算出来的
  为 t7，即启动后 6 个时钟周期。

- 定时器 1 在到达时间 t7 之前重置，即在它到期之前
  并执行其回调函数。定时器 1 在时间 t5 复位，因此
  它将执行回调函数的时间是
  重新计算为t11，即重置后的6个tick。

- 定时器 1 在时间 t11 之前再次重置，因此在它之前再次重置
  过期并执行其回调函数。定时器1在某个时间复位
  t9，所以执行回调函数的时间是
  重新计算为 t15，即上次重置后的 6 个刻度。

- 定时器 1 不再复位，因此在时间 t15 时到期，其
  相应地执行回调函数。


### 6.8.1 xTimerReset() API 功能

使用 `xTimerReset()` API 函数重置定时器。

`xTimerReset()` 还可用于启动处于休眠状态的定时器。

> *注意：切勿从中断服务程序中调用 `xTimerReset()`。的
> 应使用中断安全版本 `xTimerResetFromISR()`
> 地点。*


<a name="list6.14" title="Listing 6.14 The xTimerReset() API function prototype"></a>

```c
BaseType_t xTimerReset( TimerHandle_t xTimer, TickType_t xTicksToWait );
```
***清单 6.14*** *xTimerReset() API 功能原型*


**xTimerReset()参数及返回值**

- `xTimer`

正在重置或启动的软件定时器的句柄。手柄
  将从调用用于创建的 `xTimerCreate()` 返回
  软件定时器。

- `xTicksToWait`

`xTimerReset()` 使用定时器命令队列发送
  守护进程任务的“重置”命令。 `xTicksToWait` 指定最大
  调用任务应保持阻塞状态的时间
  等待计时器命令队列上有空间可用，如果
  队列已经满了。

如果 `xTicksToWait` 为零并且 `xTimerReset()` 将立即返回
  定时器命令队列已满。

如果 `FreeRTOSConfig.h` 中的 `INCLUDE_vTaskSuspend` 设置为 1，则设置
  `xTicksToWait` 到 `portMAX_DELAY` 将导致调用任务剩余
  在Blocked状态indefinitely (without a timeout)等待空间
  在计时器命令队列中变得可用。

- 返回值

有两种可能的返回值：

- `pdPASS`

仅当数据成功发送到 `pdPASS` 时才会返回
    定时器命令队列。

如果区块时间为 specified (`xTicksToWait` was not zero)，那么它是
    可能调用任务被置于阻塞状态以等待
    定时器命令队列中函数之前的可用空间
    返回，但数据已成功写入定时器命令队列
    在区块时间到期之前。

`pdFAIL`

如果无法写入“reset”命令，将返回 `pdFAIL`
    到计时器命令队列，因为队列已满。

如果区块时间为 specified (`xTicksToWait` was not zero) 则
    调用任务将被置于阻塞状态等待
    守护进程任务在队列中腾出空间，但指定阻塞时间
    在此之前已过期。


<a name="example6.3" title="Example 6.3 Resetting a software timer"></a>
---
***示例 6.3*** *重置软件定时器*

---

此示例模拟手机上背光的行为。背光：

- 按下某个键时打开。

- 如果在一定时间内按下更多按键，则保持开启状态。

- 如果在一定时间内没有按键，则自动关闭。

一次性软件定时器用于实现这一行为：

- 当按下按键时，\[模拟\]背光打开，并且
  在软件定时器的回调函数中关闭。

- 每次按下按键时软件计时器都会重置。

- 必须按下某个键以防止发生故障的时间段
  因此，背光关闭的时间等于
  软件定时器；如果软件定时器没有通过按键重置
  在定时器到期之前，然后是定时器的回调函数
  执行，并且背光关闭。

`xSimulatedBacklightOn` 变量保存背光状态。
`xSimulatedBacklightOn` 设置为 `pdTRUE` 表示背光打开，
`pdFALSE` 表示背光关闭。

软件定时器回调函数如清单6.15所示。


<a name="list6.15" title="Listing 6.15 The callback function for the one-shot timer used in Example 6.3"></a>

```c
static void prvBacklightTimerCallback( TimerHandle_t xTimer )
{
    TickType_t xTimeNow = xTaskGetTickCount();

    /* The backlight timer expired, turn the backlight off. */
    xSimulatedBacklightOn = pdFALSE;

    /* Print the time at which the backlight was turned off. */
    vPrintStringAndNumber(
            "Timer expired, turning backlight OFF at time\t\t", xTimeNow );
}
```
***清单 6.15*** *示例 6.3 中使用的一次性定时器的回调函数*


例 6.3 创建了一个任务来轮询键盘[^11]。任务已显示
清单 6.16 中，但由于下一段所述的原因，
清单 6.16 并不代表最佳设计。

[^11]：打印到 Windows 控制台，并从
Windows控制台，两者都会导致Windows系统的执行
来电。 Windows 系统调用，包括使用 Windows 控制台，
磁盘或 TCP/IP 堆栈可能会对
FreeRTOS Windows 端口，通常应避免。*

使用 FreeRTOS 允许您的应用程序是事件驱动的。事件驱动
设计非常有效地利用处理时间，因为处理时间是
仅在事件发生时使用，并且不会浪费处理时间
轮询尚未发生的事件。例6.3无法制作
事件驱动，因为处理键盘中断不切实际
使用 FreeRTOS Windows 端口时，轮询效率要低得多
必须使用技术来代替。如果清单 6.16 是一个中断服务
例程，则 `xTimerResetFromISR()` 将用于代替
`xTimerReset()`。


<a name="list6.16" title="Listing 6.16 The task used to reset the software timer in Example 6.3"></a>

```c
static void vKeyHitTask( void *pvParameters )
{
    const TickType_t xShortDelay = pdMS_TO_TICKS( 50 );
    TickType_t xTimeNow;

    vPrintString( "Press a key to turn the backlight on.\r\n" );

    /* Ideally an application would be event driven, and use an interrupt to
       process key presses. It is not practical to use keyboard interrupts
       when using the FreeRTOS Windows port, so this task is used to poll for
       a key press. */
    for( ;; )
    {
        /* Has a key been pressed? */
        if( _kbhit() != 0 )
        {
            /* A key has been pressed. Record the time. */
            xTimeNow = xTaskGetTickCount();

            if( xSimulatedBacklightOn == pdFALSE )
            {

                /* The backlight was off, so turn it on and print the time at
                   which it was turned on. */
                xSimulatedBacklightOn = pdTRUE;
                vPrintStringAndNumber(
                    "Key pressed, turning backlight ON at time\t\t",
                    xTimeNow );
            }
            else
            {
                /* The backlight was already on, so print a message to say the
                   timer is about to be reset and the time at which it was
                   reset. */
                vPrintStringAndNumber(
                    "Key pressed, resetting software timer at time\t\t",
                    xTimeNow );
            }

            /* Reset the software timer. If the backlight was previously off,
               then this call will start the timer. If the backlight was
               previously on, then this call will restart the timer. A real
               application may read key presses in an interrupt. If this
               function was an interrupt service routine then
               xTimerResetFromISR() must be used instead of xTimerReset(). */
            xTimerReset( xBacklightTimer, xShortDelay );

            /* Read and discard the key that was pressed – it is not required
               by this simple example. */
            ( void ) _getch();
        }
    }
}
```
***清单 6.16*** *示例 6.3 中用于重置软件定时器的任务*

执行例 6.3 时产生的输出如图 6.10 所示。
参考图6.10：

- 第一次按键发生在刻度数为 812 时。
  背光灯打开的时间，单次定时器
  开始了。

- 当滴答计数为 1813、3114 时发生进一步的按键操作，
  4015 和 5016。所有这些按键都会导致计时器
  在计时器到期之前重置。

- 当滴答计数为 10016 时，计时器到期。此时
  背光灯已关闭。


<a name="fig6.10" title="Figure 6.10 The output produced when Example 6.3 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image47.jpg)
***图 6.10*** *执行例 6.3 时产生的输出*

* * *

从图 6.10 可以看出，定时器的周期为 5000 个刻度；
最后一次按键后，背光恰好关闭 5000 次
按下，所以在计时器上次重置后 5000 个滴答声。
