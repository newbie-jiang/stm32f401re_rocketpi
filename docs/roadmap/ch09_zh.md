# 9 事件组

## 9.1 章节简介和范围

人们已经注意到，实时嵌入式系统必须采取
响应事件的行动。前面的章节已经描述了功能
允许将事件传递给任务的 FreeRTOS。的例子
这些功能包括信号量和队列，两者都具有
以下属性：

- 它们允许任务在阻塞状态下等待单个事件
  发生。

- 当事件发生时，他们解锁单个任务。任务就是
  解锁是正在等待的最高优先级任务
  事件。

事件组是 FreeRTOS 的另一个功能，它允许事件
传达任务。与队列和信号量不同：

- 事件组允许任务在阻塞状态下等待
  要发生的多个事件之一的组合。

- 事件组解锁所有等待相同任务的任务
  事件或事件的组合，当事件发生时。

事件组的这些独特属性使它们非常有用
同步多个任务，向多个任务广播事件，
允许任务在阻塞状态下等待一组中的任何一个
事件发生，并允许任务在阻塞状态下等待
需要完成多个动作。

事件组还提供了减少 RAM 使用的机会
应用程序，因为通常可以替换许多二进制信号量
与单个事件组。

事件组功能是可选的。包含事件组
功能，构建 FreeRTOS 源文件 event\_groups.c 作为
你的项目。


### 9.1.1 范围

本章旨在让读者更好地理解：

- 事件组的实际用途。
- 活动组相对于其他组的优点和缺点
  FreeRTOS 功能。
- 如何设置事件组中的位。
- 如何在阻塞状态下等待事件中的位被设置
  组。
- 如何使用事件组来同步一组任务。


## 9.2 事件组的特征

### 9.2.1 事件组、事件标志和事件位

事件“标志”是一个 Boolean (1 or 0) 值，用于指示事件是否
发生或未发生。事件“组”是一组事件标志。

事件标志只能是 1 或 0，允许事件标志的状态
被存储在单个位中，并且所有事件标志的状态在一个
事件组存储在单个变量中；每个事件的状态
事件组中的标志由变量中的单个位表示
型号 `EventBits_t`。因此，事件标志也称为事件
“位”。如果 `EventBits_t` 变量中的某个位设置为 1，则该事件
该位代表的情况已经发生。如果某个位设置为 0
`EventBits_t`变量，则该位代表的事件还没有
发生。

图 9.1 显示了各个事件标志如何映射到各个位
在 `EventBits_t` 类型的变量中。


<a name="fig9.1" title="Figure 9.1 Event flag to bit number mapping in a variable of type EventBits\_t"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image71.png)
***图 9.1*** *EventBits\_t* 类型变量中事件标志到位数的映射

* * *

例如，如果事件组的值为 0x92（二进制 1001 0010）
那么只有事件位 1、4 和 7 被设置，因此只有代表的事件
位 1、4 和 7 已发生。图 9.2 显示了一个变量类型
`EventBits_t` 已设置事件位 1、4 和 7，以及所有其他事件
位清零，为事件组赋予值 0x92。


<a name="fig9.2" title="Figure 9.2 An event group in which only bits 1, 4 and 7 are set, and all the other event flags are clear, making the event group's value 0x92"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image72.png)
***图 9.2*** *事件组中仅设置了位 1、4 和 7，所有其他事件标志均被清除，使得事件组的值为 0x92*

* * *

由应用程序编写者为个人赋予含义
事件组内的位。例如，应用程序编写者可能
创建一个事件组，然后：

- 定义事件组中的位 0 表示“消息已被发送”
  从网络收到”。

- 定义事件组中的位 1 表示“消息已准备好
  被发送到网络上”。

- 定义事件组中的位 2 表示“中止当前事件”
  网络连接”。


### 9.2.2 有关 EventBits\_t 数据类型的更多信息

事件组中事件位的数量取决于
`configTICK_TYPE_WIDTH_IN_BITS` 编译时配置常量
FreeRTOSConfig.h[^24]：

[^24]：`configTICK_TYPE_WIDTH_IN_BITS` 配置用于保存 RTOS 的类型
滴答计数，因此似乎与事件组功能无关。其
对 `EventBits_t` 类型的影响是 FreeRTOS 的结果
内部实现，并且需要设置 `configTICK_TYPE_WIDTH_IN_BITS`
对于 `TICK_TYPE_WIDTH_16_BITS`，仅当 FreeRTOS 在
可以更有效地处理 16 位类型的架构
32 位类型。

- 如果 `configTICK_TYPE_WIDTH_IN_BITS` 是 `TICK_TYPE_WIDTH_16_BITS`，则每个
  事件组包含 8 个可用事件位。

- 如果 `configTICK_TYPE_WIDTH_IN_BITS` 为 `TICK_TYPE_WIDTH_32_BITS`，则每个事件组包含 24 个
  可用的事件位。

- 如果 `configTICK_TYPE_WIDTH_IN_BITS` 是 `TICK_TYPE_WIDTH_64_BITS`，则每个
  事件组包含 56 个可用事件位。


### 9.2.3 多任务访问

事件组本身就是对象，任何人都可以访问
任务或 ISR 知道它们的存在。可以设置任意数量的任务
同一事件组中的位，并且任意数量的任务可以读取位
同一事件组。


### 9.2.4 使用事件组的实际示例

FreeRTOS+TCP TCP/IP堆栈的实现提供了实用的
如何使用事件组同时简化事件的示例
设计，并最大限度地减少资源使用。

TCP 套接字必须响应许多不同的事件。事件示例
包括接受事件、绑定事件、读取事件和关闭事件。的
套接字在任何给定时间可以预期的事件取决于状态
插座的。例如，如果套接字已创建，但尚未创建
绑定到一个地址，那么它可以期望收到一个绑定事件，但是
不会期望收到读取事件（如果收到，则无法读取数据）
没有地址）。

FreeRTOS+TCP 套接字的状态保存在名为
`FreeRTOS_Socket_t`。该结构包含一个事件组，该事件组具有
为套接字必须处理的每个事件定义的事件位。 FreeRTOS+TCP
API 调用该块来等待一个事件或一组事件，只需
阻止事件组。

事件组还包含一个“中止”位，允许 TCP 连接
被中止，无论套接字正在等待哪个事件
时间。


## 9.3 使用事件组进行事件管理

### 9.3.1 xEventGroupCreate() API 功能

FreeRTOS还包括`xEventGroupCreateStatic()`函数，
它静态分配创建事件组所需的内存
在编译时：必须先显式创建事件组，然后才能
被使用。

使用 `EventGroupHandle_t` 类型的变量引用事件组。
`xEventGroupCreate()` API函数用于创建事件组，
并返回 `EventGroupHandle_t` 来引用事件组
创造。


<a name="list9.1" title="Listing 9.1 The xEventGroupCreate() API function prototype"></a>

```c
EventGroupHandle_t xEventGroupCreate( void );
```
***清单 9.1*** *xEventGroupCreate() API 函数原型*


**xEventGroupCreate() 返回值**

- 返回值

如果返回NULL，则无法创建事件组
  因为 FreeRTOS 没有足够的可用堆内存
  分配事件组数据结构。第 3 章提供了更多内容
  有关堆内存管理的信息。

返回的非 NULL 值表示事件组已
  已创建成功。返回值应存储为
  创建的事件组的句柄。


### 9.3.2 xEventGroupSetBits() API 功能

`xEventGroupSetBits()` API 函数设置事件中的一个或多个位
组，通常用于通知任务该事件
由一个或多个位表示的设置已经发生。

> *注意：切勿从中断服务中调用 `xEventGroupSetBits()`
> 常规。中断安全版本 `xEventGroupSetBitsFromISR()` 应
> 在其位置使用。*


<a name="list9.2" title="Listing 9.2. The xEventGroupSetBits() API function prototype"></a>

```c
EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup,

const EventBits_t uxBitsToSet );
```
***清单 9.2*** *xEventGroupSetBits() API 函数原型*


**xEventGroupSetBits()参数及返回值**

- `xEventGroup`

正在设置位的事件组的句柄。活动内容
  组句柄将从调用中返回
  `xEventGroupCreate()` 用于创建事件组。

- `uxBitsToSet`

指定要设置的一个或多个事件位的位掩码
  活动组中1人。事件组的值按位更新
  将事件组的现有值与传入的值进行“或”运算
  `uxBitsToSet`。

例如，将 `uxBitsToSet` 设置为 0x04（二进制 0100）将导致
  在事件组中的事件位 3 中（如果尚未设置）
  设置），同时保留事件组中的所有其他事件位
  不变。

- 返回值

调用时事件组的值
  `xEventGroupSetBits()` 已返回。请注意，返回的值不会
  必须设置 `uxBitsToSet` 指定的位，因为这些位
  可能已被不同的任务再次清除。


### 9.3.3 xEventGroupSetBitsFromISR() API 功能

`xEventGroupSetBitsFromISR()` 是中断安全版本
`xEventGroupSetBits()`。

给出信号量是一种确定性操作，因为它在
提出给予信号量最多可以导致一个任务离开
阻塞状态。事件组中的位何时设置是未知的
提前有多少任务将离开阻塞状态，因此设置位
在事件组中不是确定性操作。

FreeRTOS设计和实现标准不允许
在中断服务内执行的非确定性操作
例程，或者当中断被禁用时。由于这个原因，
`xEventGroupSetBitsFromISR()` 不会直接在内部设置事件位
中断服务例程，而是将操作推迟到 RTOS
守护进程任务。


<a name="list9.3" title="Listing 9.3 The xEventGroupSetBitsFromISR() API function prototype"></a>

```c
BaseType_t xEventGroupSetBitsFromISR( EventGroupHandle_t xEventGroup,
                                      const EventBits_t uxBitsToSet,
                                      BaseType_t *pxHigherPriorityTaskWoken );
```
***清单 9.3*** *xEventGroupSetBitsFromISR() API 函数原型*


**xEventGroupSetBitsFromISR()参数及返回值**

- `xEventGroup`

正在设置位的事件组的句柄。活动内容
  组句柄将从调用中返回
  `xEventGroupCreate()` 用于创建事件组。

- `uxBitsToSet`

指定要设置的一个或多个事件位的位掩码
  活动组中1人。事件组的值按位更新
  将事件组的现有值与传入的值进行“或”运算
  `uxBitsToSet`。

例如，将 `uxBitsToSet` 设置为 0x05（二进制 0101）将导致
  事件组中的事件位 2 和事件位 0 被置位（如果它们
  尚未设置），同时保留所有其他事件位
  事件组不变。

- `pxHigherPriorityTaskWoken`

`xEventGroupSetBitsFromISR()` 不直接设置事件位
  在中断服务程序内部，而是将操作推迟到
  RTOS 守护程序任务通过在计时器命令队列上发送命令来执行。如果
  守护任务处于阻塞状态，等待数据变为
  在定时器命令队列上可用，然后写入定时器命令
  队列将导致守护任务离开阻塞状态。如果
  守护任务的优先级高于当前任务的优先级
  执行 task (the task that was interrupted)，然后在内部，
  `xEventGroupSetBitsFromISR()` 将 `*pxHigherPriorityTaskWoken` 设置为
  `pdTRUE`。

如果 `xEventGroupSetBitsFromISR()` 将此值设置为 `pdTRUE`，则
  上下文切换应该在中断退出之前执行。这个
  将确保中断直接返回到守护程序任务，如下所示
  守护任务将是最高优先级的就绪状态任务。

- 返回值

有两种可能的返回值：

- 仅当数据成功发送至
    定时器命令队列。

- 如果无法执行“设置位”命令，将返回 `pdFALSE`
    写入定时器命令队列，因为该队列已经
    满。


### 9.3.4 xEventGroupWaitBits() API 功能

`xEventGroupWaitBits()` API 函数允许任务读取值
事件组的一个，并且可以选择在阻塞状态下等待一个或
如果事件位已设置，则事件组中的更多事件位将被设置
尚未设置。


<a name="list9.4" title="Listing 9.4 The xEventGroupWaitBits() API function prototype"></a>

```c
EventBits_t xEventGroupWaitBits( EventGroupHandle_t xEventGroup,
                                 const EventBits_t uxBitsToWaitFor,
                                 const BaseType_t xClearOnExit,
                                 const BaseType_t xWaitForAllBits,
                                 TickType_t xTicksToWait );
```
***清单 9.4*** *xEventGroupWaitBits() API 函数原型*

调度程序用来确定任务是否进入的条件
阻塞状态，以及任务何时离开阻塞状态，是
称为“解锁条件”。解锁条件由一个指定
`uxBitsToWaitFor` 和 `xWaitForAllBits` 参数的组合
价值观：

- `uxBitsToWaitFor` 指定事件组中的哪些事件位
  测试

- `xWaitForAllBits` 指定是使用按位 OR 测试还是使用
  按位 AND 测试

如果满足解锁条件，任务不会进入阻塞状态
调用 `xEventGroupWaitBits()` 时。

导致任务进入的条件示例
表 6 中提供了阻塞状态或退出阻塞状态。
表6仅显示了事件的最低有效的四个二进制位
组和 uxBitsToWaitFor 值 - 这两个值的其他位是
假设为零。

<a name="tbl6" title="Table 6 The Effect of the uxBitsToWaitFor and xWaitForAllBits Parameters"></a>

* * *
|现有事件组值 | uxBitsToWaitFor值| xWaitForAllBits 值 |结果行为|
| -------------------------- | --------------------- | --------------------- | ------------------ |
| 0000 | 0000 0101| pdFALSE |由于事件组中的位 0 或位 2 均未置位，调用任务将进入阻塞状态，并且当事件组中的位 0 OR 位 2 置位时，调用任务将离开阻塞状态。 |
| 0100 | 0100 0101| pdTRUE |调用任务将进入阻塞状态，因为事件组中的位 0 和位 2 未同时置位，并且当事件组中的位 0 AND 和位 2 均置位时，调用任务将离开阻塞状态。 |
| 0100 | 0100 0110| pdFALSE |调用任务不会进入阻塞状态，因为 xWaitForAllBits 是 pdFALSE，并且 uxBitsToWaitFor 指定的两位之一已在事件组中设置。 |
| 0100 | 0100 0110| pdTRUE |调用任务将进入阻塞状态，因为 xWaitForAllBits 是 pdTRUE，并且事件组中仅设置了 uxBitsToWaitFor 指定的两位之一。当事件组中的位 1 和位 2 均被设置时，任务将离开阻塞状态。 |

***表 6*** *uxBitsToWaitFor 和 xWaitForAllBits 参数的影响*
* * *

调用任务指定要使用 `uxBitsToWaitFor` 进行测试的位
参数，并且调用任务可能需要清除这些
满足解锁条件后，位恢复为零。事件位
可以使用 `xEventGroupClearBits()` API 函数清除，但使用
手动清除事件位的功能将导致竞争条件
在应用程序代码中，如果：

- 多个任务使用同一事件组。
- 事件组中的位由不同的任务或由
  中断服务程序。

提供 `xClearOnExit` 参数是为了避免这些潜在的竞争
条件。如果 `xClearOnExit` 设置为 `pdTRUE`，则测试和
事件位的清除对于调用任务来说似乎是一个原子操作
operation (uninterruptable by other tasks or interrupts)。

**xEventGroupWaitBits()参数及返回值**

- `xEventGroup`

包含事件位的事件组的句柄
  阅读。事件组句柄将从调用中返回
  `xEventGroupCreate()` 用于创建事件组。

- `uxBitsToWaitFor`

指定要测试的一个或多个事件位的位掩码
  在活动组中。

例如，如果调用任务想要等待事件位 0 和/或
  事件位 2 在事件组中设置，然后设置 `uxBitsToWaitFor`
  到 0x05（二进制 0101）。更多示例请参阅表 6。

- `xClearOnExit`

如果满足调用任务的解锁条件，并且
  `xClearOnExit` 设置为 `pdTRUE`，则事件位由
  `uxBitsToWaitFor` 将在事件组中清零之前
  调用任务退出 `xEventGroupWaitBits()` API 函数。

如果 `xClearOnExit` 设置为 `pdFALSE`，则事件位的状态
  事件组中的 `xEventGroupWaitBits()` 未修改 API
  功能。

- `xWaitForAllBits`

`uxBitsToWaitFor` 参数指定要测试的事件位
  事件组。 `xWaitForAllBits` 指定调用任务是否应该
  当一个或多个事件位发生时，从阻塞状态中移除
  由 `uxBitsToWaitFor` 参数指定的设置，或者仅当所有
  设置 `uxBitsToWaitFor` 参数指定的事件位。

如果 `xWaitForAllBits` 设置为 `pdFALSE`，则进入
  阻塞状态等待其解除阻塞条件满足将离开
  当 `uxBitsToWaitFor` 指定的任何位变为阻塞状态时
  set (or the timeout specified by the `xTicksToWait` parameter expires)。

如果 `xWaitForAllBits` 设置为 `pdTRUE`，则进入
  阻塞状态等待其解锁条件满足只会
  当指定的所有位都离开阻塞状态时
  `uxBitsToWaitFor` 已设置（或 `xTicksToWait` 指定的超时
  参数过期）。

示例请参阅表 6。

- `xTicksToWait`

任务应保留在已阻止状态的最长时间
  状态等待满足其解锁条件。

如果 `xTicksToWait` 为 `xEventGroupWaitBits()` 将立即返回
  零，或者在`xEventGroupWaitBits()`时刻满足解锁条件
  被称为。

区块时间以滴答周期为单位指定，因此它的绝对时间
  表示取决于滴答频率。宏 `pdMS_TO_TICKS()`
  可用于将以毫秒为单位的时间转换为时间
  以刻度指定。

将 `xTicksToWait` 设置为 `portMAX_DELAY` 将导致任务等待
  indefinitely (without timing out)，前提是设置了 `INCLUDE_vTaskSuspend`
  为 1 英寸 FreeRTOSConfig.h。

- 返回值

如果 `xEventGroupWaitBits()` 由于调用任务的
  满足解锁条件，则返回值为
  调用任务的解锁条件时的事件组为 met
  (before any bits were automatically cleared if `xClearOnExit` was `pdTRUE`)。
  在这种情况下，返回值也将满足解锁条件。

如果 `xEventGroupWaitBits()` 由于指定的区块时间而返回
  `xTicksToWait`参数过期，则返回值为该值
  块时间到期时事件组的编号。在这种情况下
  返回值将不满足解锁条件。


### 9.3.5 xEventGroupGetStaticBuffer() API 功能

`xEventGroupGetStaticBuffer()` API 函数提供了检索指针的方法
到静态创建的事件组的缓冲区。它与提供的缓冲区相同
在创建事件组时。

*注意：切勿从中断服务中调用 `xEventGroupGetStaticBuffer()`
例行公事。


<a name="list9.5" title="Listing 9.5 The xEventGroupGetStaticBuffer() API function prototype"></a>

```c
BaseType_t xEventGroupGetStaticBuffer( EventGroupHandle_t xEventGroup,

StaticEventGroup_t ** ppxEventGroupBuffer );
```
***清单 9.5*** *xEventGroupGetStaticBuffer() API 功能原型*


**xEventGroupGetStaticBuffer()参数及返回值**

- `xEventGroup`

要检索缓冲区的事件组。此活动组必须
  由 `xEventGroupCreateStatic()` 创建。

- `ppxEventGroupBuffer`

用于返回指向事件组数据结构缓冲区的指针。
  它与创建时提供的缓冲区相同。

- 返回值

有两种可能的返回值：

- 如果成功检索缓冲区，将返回 `pdTRUE`。

- 如果缓冲区未成功检索，将返回 `pdFALSE`。

<a name="example9.1" title="Example 9.1 Experimenting with event groups"></a>
---
***示例 9.1*** *使用事件组进行实验*

---

此示例演示如何：

- 创建一个活动组。
- 通过中断服务例程设置事件组中的位。
- 设置任务中事件组中的位。
- 阻止事件组。

`xEventGroupWaitBits()` `xWaitForAllBits`参数的作用是
首先执行示例并将 `xWaitForAllBits` 设置为
`pdFALSE`，然后执行示例并将 `xWaitForAllBits` 设置为
`pdTRUE`。

事件位 0 和事件位 1 由任务设置。事件位 2 设置为
一个中断服务程序。这三位给出了描述性的
使用清单 9.6 中所示的 \#define 语句命名。


<a name="list9.6" title="Listing 9.6 Event bit definitions used in Example 9.1"></a>

```c
/* Definitions for the event bits in the event group. */
#define mainFIRST_TASK_BIT ( 1UL << 0UL )  /* Event bit 0, set by a task */
#define mainSECOND_TASK_BIT ( 1UL << 1UL ) /* Event bit 1, set by a task */
#define mainISR_BIT ( 1UL << 2UL )         /* Event bit 2, set by an ISR */
```
***清单 9.6*** *示例 9.1 中使用的事件位定义*


清单 9.7 显示了设置事件位 0 的任务的实现
和事件位 1。它处于一个循环中，重复设置一位，然后
其他，每次调用之间有 200 毫秒的延迟
`xEventGroupSetBits()`。在将每个位设置为之前打印出一个字符串
允许在控制台中看到执行顺序。


<a name="list9.7" title="Listing 9.7 The task that sets two bits in the event group in Example 9.1"></a>

```c
static void vEventBitSettingTask( void *pvParameters )
{
    const TickType_t xDelay200ms = pdMS_TO_TICKS( 200UL );

    for( ;; )
    {
        /* Delay for a short while before starting the next loop. */
        vTaskDelay( xDelay200ms );

        /* Print out a message to say event bit 0 is about to be set by the
           task, then set event bit 0. */
        vPrintString( "Bit setting task -\t about to set bit 0.\r\n" );
        xEventGroupSetBits( xEventGroup, mainFIRST_TASK_BIT );

        /* Delay for a short while before setting the other bit. */
        vTaskDelay( xDelay200ms );

        /* Print out a message to say event bit 1 is about to be set by the
           task, then set event bit 1. */
        vPrintString( "Bit setting task -\t about to set bit 1.\r\n" );
        xEventGroupSetBits( xEventGroup, mainSECOND_TASK_BIT );
    }
}
```
***清单 9.7*** *在示例 9.1 中设置事件组中两位的任务*


清单9.8显示了中断服务程序的实现
设置事件组中的位 2。再次打印出一个字符串
在设置该位以允许执行序列之前
控制台。然而在这种情况下，因为控制台输出不应该是
直接在中断服务程序中执行，
`xTimerPendFunctionCallFromISR()` 用于执行输出
RTOS 守护程序任务的上下文。

与前面的示例一样，中断服务例程由
强制软件中断的简单周期性任务。在这个例子中，
中断每 500 毫秒生成一次。


<a name="list9.8" title="Listing 9.8 The ISR that sets bit 2 in the event group in Example 9.1"></a>

```c
static uint32_t ulEventBitSettingISR( void )
{
    /* The string is not printed within the interrupt service routine, but is
       instead sent to the RTOS daemon task for printing. It is therefore
       declared static to ensure the compiler does not allocate the string on
       the stack of the ISR, as the ISR's stack frame will not exist when the
       string is printed from the daemon task. */
    static const char *pcString = "Bit setting ISR -\t about to set bit 2.\r\n";
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Print out a message to say bit 2 is about to be set. Messages cannot
       be printed from an ISR, so defer the actual output to the RTOS daemon
       task by pending a function call to run in the context of the RTOS
       daemon task. */
    xTimerPendFunctionCallFromISR( vPrintStringFromDaemonTask,
                                   ( void * ) pcString,
                                   0,
                                   &xHigherPriorityTaskWoken );

    /* Set bit 2 in the event group. */
    xEventGroupSetBitsFromISR( xEventGroup,
                               mainISR_BIT,
                               &xHigherPriorityTaskWoken );

    /* xTimerPendFunctionCallFromISR() and xEventGroupSetBitsFromISR() both
       write to the timer command queue, and both used the same
       xHigherPriorityTaskWoken variable. If writing to the timer command
       queue resulted in the RTOS daemon task leaving the Blocked state, and
       if the priority of the RTOS daemon task is higher than the priority of
       the currently executing task (the task this interrupt interrupted) then
       xHigherPriorityTaskWoken will have been set to pdTRUE.

       xHigherPriorityTaskWoken is used as the parameter to
       portYIELD_FROM_ISR(). If xHigherPriorityTaskWoken equals pdTRUE, then
       calling portYIELD_FROM_ISR() will request a context switch. If
       xHigherPriorityTaskWoken is still pdFALSE, then calling
       portYIELD_FROM_ISR() will have no effect.

       The implementation of portYIELD_FROM_ISR() used by the Windows port
       includes a return statement, which is why this function does not
       explicitly return a value. */

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```
***清单 9.8*** *在示例 9.1 中设置事件组中的位 2 的 ISR*


清单 9.9 显示了调用的任务的实现
`xEventGroupWaitBits()` 用于阻止事件组。该任务打印出一个
事件组中设置的每个位的字符串。

`xEventGroupWaitBits()` `xClearOnExit` 参数设置为 `pdTRUE`，因此
导致调用 `xEventGroupWaitBits()` 的一个或多个事件位
返回值在 `xEventGroupWaitBits()` 之前会自动清零
返回。


<a name="list9.9" title="Listing 9.9 The task that blocks to wait for event bits to become set in Example 9.1"></a>

```c
static void vEventBitReadingTask( void *pvParameters )
{
    EventBits_t xEventGroupValue;
    const EventBits_t xBitsToWaitFor = ( mainFIRST_TASK_BIT  |
                                         mainSECOND_TASK_BIT |
                                         mainISR_BIT );

    for( ;; )
    {
        /* Block to wait for event bits to become set within the event
           group. */
        xEventGroupValue = xEventGroupWaitBits( /* The event group to read */
                                                xEventGroup,

                                                /* Bits to test */
                                                xBitsToWaitFor,

                                                /* Clear bits on exit if the
                                                   unblock condition is met */
                                                pdTRUE,

                                                /* Don't wait for all bits. This
                                                   parameter is set to pdTRUE for the
                                                   second execution. */
                                                pdFALSE,

                                                /* Don't time out. */
                                                portMAX_DELAY );

        /* Print a message for each bit that was set. */
        if( ( xEventGroupValue & mainFIRST_TASK_BIT ) != 0 )
        {
            vPrintString( "Bit reading task -\t Event bit 0 was set\r\n" );
        }

        if( ( xEventGroupValue & mainSECOND_TASK_BIT ) != 0 )
        {
            vPrintString( "Bit reading task -\t Event bit 1 was set\r\n" );
        }

        if( ( xEventGroupValue & mainISR_BIT ) != 0 )
        {
            vPrintString( "Bit reading task -\t Event bit 2 was set\r\n" );
        }
    }
}
```
***清单 9.9*** *示例 9.1 中阻塞等待事件位设置的任务*


`main()` 函数在之前创建事件组和任务
启动调度程序。其实现请参见清单 9.10。的
从事件组读取的任务的优先级高于
写入事件组的任务的优先级，确保
每次读任务执行时，读任务都会抢占写任务
满足解锁条件。


<a name="list9.10" title="Listing 9.10 Creating the event group and tasks in Example 9.1"></a>

```c
int main( void )
{
    /* Before an event group can be used it must first be created. */
    xEventGroup = xEventGroupCreate();

    /* Create the task that sets event bits in the event group. */
    xTaskCreate( vEventBitSettingTask, "Bit Setter", 1000, NULL, 1, NULL );

    /* Create the task that waits for event bits to get set in the event
       group. */
    xTaskCreate( vEventBitReadingTask, "Bit Reader", 1000, NULL, 2, NULL );

    /* Create the task that is used to periodically generate a software
       interrupt. */
    xTaskCreate( vInterruptGenerator, "Int Gen", 1000, NULL, 3, NULL );

    /* Install the handler for the software interrupt. The syntax necessary
       to do this is dependent on the FreeRTOS port being used. The syntax
       shown here can only be used with the FreeRTOS Windows port, where such
       interrupts are only simulated. */
    vPortSetInterruptHandler( mainINTERRUPT_NUMBER, ulEventBitSettingISR );

    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();

    /* The following line should never be reached. */
    for( ;; );
    return 0;
}
```
***清单 9.10*** *创建示例 9.1 中的事件组和任务*


使用以下命令执行例 9.1 时产生的输出
显示 `xEventGroupWaitBits()` `xWaitForAllBits` 参数设置为 `pdFALSE`
如图 9.3 所示。从图 9.3 中可以看出，由于
`xWaitForAllBits` 调用 `xEventGroupWaitBits()` 中的参数已设置
到 `pdFALSE`，从事件组读取的任务离开阻塞状态
状态并在每次设置任何事件位时立即执行。


<a name="fig9.3" title="Figure 9.3 The output produced when Example 9.1 is executed with xWaitForAllBits set to pdFALSE"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image73.jpg)
***图 9.3*** *在 xWaitForAllBits 设置为 pdFALSE 的情况下执行示例 9.1 时产生的输出*

* * *

使用以下命令执行例 9.1 时产生的输出
显示 `xEventGroupWaitBits()` `xWaitForAllBits` 参数设置为 `pdTRUE`
如图 9.4 所示。从图 9.4 可以看出，由于
`xWaitForAllBits` 参数设置为 `pdTRUE`，任务从
事件组仅在所有三个事件之后才离开阻塞状态
事件位已设置。


<a name="fig9.4" title="Figure 9.4 The output produced when Example 9.1 is executed with xWaitForAllBits set to pdTRUE"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image74.jpg)
***图 9.4*** *在 xWaitForAllBits 设置为 pdTRUE 的情况下执行示例 9.1 时产生的输出*

* * *


## 9.4 使用事件组进行任务同步

有时应用程序的设计需要两个或多个任务来
彼此同步。例如，考虑一个设计，其中任务 A
接收一个事件，然后委托一些必要的处理
将事件分配给其他三个任务：任务 B、任务 C 和任务 D。如果任务 A
在任务 B、C 和 D 全部完成之前无法接收另一个事件
处理前一个事件，那么所有四个任务都需要
彼此同步。每个任务的同步点将是
该任务完成处理后，无法继续进行
直到其他每个任务都完成相同的操作。任务A只能
在所有四个任务都完成后接收另一个事件
同步点。

此类任务需求的一个不太抽象的示例
同步可以在 FreeRTOS+TCP 演示之一中找到
项目。该演示在两个任务之间共享 TCP 套接字；一
任务向套接字发送数据，另一个任务从套接字接收数据
相同的套接字[^25]。这两个任务关闭 TCP 都是不安全的
套接字，直到确定其他任务不会尝试访问该套接字
再次套接字。如果两个任务中的任何一个希望关闭套接字，
然后它必须将其意图告知其他任务，然后等待
在继续之前停止使用套接字的其他任务。场景
其中任务将数据发送到希望关闭的套接字
清单 9.10 中的伪代码演示了套接字。

[^25]：在撰写本文时，这是单一方法的唯一方法
FreeRTOS+TCP 套接字可以在任务之间共享。

清单 9.10 演示的场景很简单，因为只有
两个任务需要相互同步，但很容易
看看场景将如何变得更加复杂，并需要更多任务
如果其他任务正在执行处理，则加入同步
这取决于套接字是否打开。


<a name="list9.11" title="Listing 9.11 Pseudo code for two tasks that synchronize with each other to ensure a shared TCP socket..."></a>

```c
void SocketTxTask( void *pvParameters )
{
    xSocket_t xSocket;
    uint32_t ulTxCount = 0UL;

    for( ;; )
    {
        /* Create a new socket. This task will send to this socket, and another
           task will receive from this socket. */
        xSocket = FreeRTOS_socket( ... );

        /* Connect the socket. */
        FreeRTOS_connect( xSocket, ... );

        /* Use a queue to send the socket to the task that receives data. */
        xQueueSend( xSocketPassingQueue, &xSocket, portMAX_DELAY );

        /* Send 1000 messages to the socket before closing the socket. */
        for( ulTxCount = 0; ulTxCount < 1000; ulTxCount++ )
        {
            if( FreeRTOS_send( xSocket, ... ) < 0 )
            {
                /* Unexpected error - exit the loop, after which the socket
                   will be closed. */
                break;
            }
        }

        /* Let the Rx task know the Tx task wants to close the socket. */
        TxTaskWantsToCloseSocket();

        /* This is the Tx task's synchronization point. The Tx task waits here
           for the Rx task to reach its synchronization point. The Rx task will
           only reach its synchronization point when it is no longer using the
           socket, and the socket can be closed safely. */
        xEventGroupSync( ... );

        /* Neither task is using the socket. Shut down the connection, then
           close the socket. */
        FreeRTOS_shutdown( xSocket, ... );
        WaitForSocketToDisconnect();
        FreeRTOS_closesocket( xSocket );
    }
}
/*-----------------------------------------------------------*/

void SocketRxTask( void *pvParameters )
{
    xSocket_t xSocket;

    for( ;; )
    {
        /* Wait to receive a socket that was created and connected by the Tx
           task. */
        xQueueReceive( xSocketPassingQueue, &xSocket, portMAX_DELAY );

        /* Keep receiving from the socket until the Tx task wants to close the
           socket. */
        while( TxTaskWantsToCloseSocket() == pdFALSE )
        {
           /* Receive then process data. */
           FreeRTOS_recv( xSocket, ... );
           ProcessReceivedData();
        }

        /* This is the Rx task's synchronization point - it only reaches here
           when it is no longer using the socket, and it is therefore safe for
           the Tx task to close the socket. */
        xEventGroupSync( ... );
    }
}
```
***清单 9.11*** *两个任务的伪代码相互同步，以确保共享的 TCP 套接字在套接字关闭之前不再被任一任务使用*


事件组可用于创建同步点：

- 每个必须参与同步的任务都被分配一个
  事件组内的唯一事件位。

- 每个任务在达到同步时设置自己的事件位
  点。

- 设置了自己的事件位后，事件组上的每个任务都会阻塞
  等待代表所有其他同步的事件位
  任务也已确定。

但是，`xEventGroupSetBits()` 和 `xEventGroupWaitBits()` API
在这种情况下不能使用函数。如果使用它们，那么
设置一个位（以指示任务已达到同步
点）和位测试（以确定其他同步
任务已达到其同步点）将作为两个执行
单独的操作。要了解为什么这会成为问题，请考虑
任务 A、任务 B 和任务 C 尝试使用同步的场景
活动组：

1. 任务A和任务B已经到达同步点，所以
   它们的事件位在事件组中设置，并且它们位于
   阻塞状态等待任务 C 的事件位也被设置。

2. 任务C到达同步点，并使用
   `xEventGroupSetBits()` 设置事件组中的位。一旦
   任务 C 的位被设置，任务 A 和任务 B 离开阻塞状态，并且
   清除所有三个事件位。

3.任务C然后调用`xEventGroupWaitBits()`来等待所有三个事件
   位被设置，但到那时，所有三个事件位都已设置
   已清除，任务A和任务B已离开各自的
   同步点，因此同步失败。

要成功使用事件组创建同步点，
事件位的设置以及事件位的后续测试，
必须作为单个不间断操作来执行。的
`xEventGroupSync()` API 功能就是为此目的而提供的。


### 9.4.1 xEventGroupSync() API 功能

提供 `xEventGroupSync()` 是为了允许两个或多个任务使用一个事件
组之间相互同步。该函数允许任务设置
事件组中的一个或多个事件位，然后等待组合
事件位在同一事件组中设置为单个
不间断运行。

`xEventGroupSync()` `uxBitsToWaitFor` 参数指定调用
任务的解锁条件。 `uxBitsToWaitFor` 指定的事件位
在 `xEventGroupSync()` 返回之前将被清零，如果
`xEventGroupSync()` 由于满足解锁条件而返回。


<a name="list9.12" title="Listing 9.12 The xEventGroupSync() API function prototype"></a>

```c
EventBits_t xEventGroupSync( EventGroupHandle_t xEventGroup,
                             const EventBits_t uxBitsToSet,
                             const EventBits_t uxBitsToWaitFor,
                             TickType_t xTicksToWait );
```
***清单 9.12*** *xEventGroupSync() API 函数原型*


**xEventGroupSync()参数及返回值**

- `xEventGroup`

要在其中设置事件位的事件组的句柄，以及
  然后进行测试。事件组句柄将从
  调用 `xEventGroupCreate()` 用于创建事件组。

- `uxBitsToSet`

指定要设置的一个或多个事件位的位掩码
  活动组中1人。事件组的值按位更新
  将事件组的现有值与传入的值进行“或”运算
  `uxBitsToSet`。

例如，将 `uxBitsToSet` 设置为 0x04（二进制 0100）将导致
  事件位 2 变为 set (if it was not already set)，同时离开
  事件组中的所有其他事件位不变。

- `uxBitsToWaitFor`

指定要测试的一个或多个事件位的位掩码
  在活动组中。

例如，如果调用任务想要等待事件位 0、1
  和 2 在事件组中设置，然后将 `uxBitsToWaitFor` 设置为 0x07
  （二进制 111）。

- `xTicksToWait`

任务应保留在已阻止状态的最长时间
  状态等待满足其解锁条件。

如果 `xTicksToWait` 为零，则 `xEventGroupSync()` 将立即返回，或者
  `xEventGroupSync()` 时满足解锁条件
  叫。

区块时间以滴答周期为单位指定，因此它的绝对时间
  表示取决于滴答频率。宏 `pdMS_TO_TICKS()`
  可用于将以毫秒为单位的时间转换为时间
  以刻度指定。

将 `xTicksToWait` 设置为 `portMAX_DELAY` 将导致任务等待
  indefinitely (without timing out)，前提是设置了 `INCLUDE_vTaskSuspend`
  为 1 英寸 FreeRTOSConfig.h。

- 返回值

如果 `xEventGroupSync()` 由于调用任务解锁而返回
  满足条件，则返回值为事件的值
  满足调用任务的解锁条件时（之前
  任何位都会自动清零）。在这种情况下
  返回值也将满足调用任务的解锁条件。

如果 `xEventGroupSync()` 由于指定的区块时间而返回
  `xTicksToWait`参数过期，则返回值为
  区块时间到期时的事件组。在这种情况下
  返回值不满足调用任务的解锁
  条件。


<a name="example9.2" title="Example 9.2 Synchronizing tasks"></a>
---
***示例 9.2*** *同步任务*

---

示例 9.2 使用 `xEventGroupSync()` 同步 a 的三个实例
单一任务执行。任务参数用于传入每个
实例 任务在调用时将设置的事件位
`xEventGroupSync()`。

该任务在调用 `xEventGroupSync()` 之前打印一条消息，然后再次
对 `xEventGroupSync()` 的调用返回后。每条消息包括
时间戳。这允许观察执行顺序
产生的输出。伪随机延迟用于防止所有
任务同时到达同步点。

该任务的实现请参见清单 9.12。


<a name="list9.13" title="Listing 9.13 The implementation of the task used in Example 9.2"></a>

```c
static void vSyncingTask( void *pvParameters )
{
    const TickType_t xMaxDelay = pdMS_TO_TICKS( 4000UL );
    const TickType_t xMinDelay = pdMS_TO_TICKS( 200UL );
    TickType_t xDelayTime;
    EventBits_t uxThisTasksSyncBit;
    const EventBits_t uxAllSyncBits = ( mainFIRST_TASK_BIT  |
                                        mainSECOND_TASK_BIT |
                                        mainTHIRD_TASK_BIT );

    /* Three instances of this task are created - each task uses a different
       event bit in the synchronization. The event bit to use is passed into
       each task instance using the task parameter. Store it in the
       uxThisTasksSyncBit variable. */
    uxThisTasksSyncBit = ( EventBits_t ) pvParameters;

    for( ;; )
    {
        /* Simulate this task taking some time to perform an action by delaying
           for a pseudo random time. This prevents all three instances of this
           task reaching the synchronization point at the same time, and so
           allows the example's behavior to be observed more easily. */
        xDelayTime = ( rand() % xMaxDelay ) + xMinDelay;
        vTaskDelay( xDelayTime );

        /* Print out a message to show this task has reached its synchronization
           point. pcTaskGetTaskName() is an API function that returns the name
           assigned to the task when the task was created. */
        vPrintTwoStrings( pcTaskGetTaskName( NULL ), "reached sync point" );

        /* Wait for all the tasks to have reached their respective
           synchronization points. */
        xEventGroupSync( /* The event group used to synchronize. */
                         xEventGroup,

                         /* The bit set by this task to indicate it has reached
                            the synchronization point. */
                         uxThisTasksSyncBit,

                         /* The bits to wait for, one bit for each task taking
                            part in the synchronization. */
                         uxAllSyncBits,

                         /* Wait indefinitely for all three tasks to reach the
                            synchronization point. */
                         portMAX_DELAY );

        /* Print out a message to show this task has passed its synchronization
           point. As an indefinite delay was used the following line will only
           be executed after all the tasks reached their respective
           synchronization points. */
        vPrintTwoStrings( pcTaskGetTaskName( NULL ), "exited sync point" );
    }
}
```
***清单 9.13*** *示例 9.2 中使用的任务的实现*

`main()` 函数创建事件组，创建所有三个任务，
然后启动调度程序。其实现请参见清单 9.14。


<a name="list9.14" title="Listing 9.14 The main() function used in Example 9.2"></a>

```c
/* Definitions for the event bits in the event group. */

#define mainFIRST_TASK_BIT ( 1UL << 0UL ) /* Event bit 0, set by the 1st task */
#define mainSECOND_TASK_BIT( 1UL << 1UL ) /* Event bit 1, set by the 2nd task */
#define mainTHIRD_TASK_BIT ( 1UL << 2UL ) /* Event bit 2, set by the 3rd task */

/* Declare the event group used to synchronize the three tasks. */
EventGroupHandle_t xEventGroup;

int main( void )
{
    /* Before an event group can be used it must first be created. */
    xEventGroup = xEventGroupCreate();

    /* Create three instances of the task. Each task is given a different
       name, which is later printed out to give a visual indication of which
       task is executing. The event bit to use when the task reaches its
       synchronization point is passed into the task using the task parameter. */
    xTaskCreate( vSyncingTask, "Task 1", 1000, mainFIRST_TASK_BIT, 1, NULL );
    xTaskCreate( vSyncingTask, "Task 2", 1000, mainSECOND_TASK_BIT, 1, NULL );
    xTaskCreate( vSyncingTask, "Task 3", 1000, mainTHIRD_TASK_BIT, 1, NULL );

    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();

    /* As always, the following line should never be reached. */
    for( ;; );
    return 0;
}
```
***清单 9.14*** *示例 9.2 中使用的 main() 函数*

执行例 9.2 时产生的输出如图 9.5 所示。
可以看出，即使各个任务达到同步
指向different (pseudo random)时间，每个任务退出
同时同步点[^26]（即
最后一个任务到达同步点）。

[^26]：图9.5显示了在FreeRTOS Windows端口中运行的示例，
它不提供真正的实时行为（特别是当
使用 Windows 系统调用打印到控制台），并且将
因此显示出一些时间变化。


<a name="fig9.5" title="Figure 9.5 The output produced when Example 9.2 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image75.jpg)
***图 9.5*** *执行例 9.2 时产生的输出*
* * *

