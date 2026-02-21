# 10 任务通知

## 10.1 简介

FreeRTOS 应用程序通常被构造为一系列独立的任务，这些任务相互通信以共同提供系统功能。  任务通知是一种有效的机制，允许一个任务直接通知另一个任务。

### 10.1.1 通过中间对象进行通信

本书已经描述了任务可以实现的各种方式
互相沟通。到目前为止描述的方法需要
创建通信对象。沟通示例
对象包括队列、事件组和各种不同类型的
信号量。

使用通信对象时，不会发送事件和数据
直接发送到接收任务，或接收 ISR，但改为发送
到通信对象。同样，任务和 ISR 接收事件并
来自通信对象的数据，而不是直接来自任务
或发送事件或数据的 ISR。如图 10.1 所示。


<a name="fig10.1" title="Figure 10.1 A communication object being used to send an event from one task to another"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image76.png)    
***图 10.1*** *用于将事件从一个任务发送到另一个任务的通信对象*

* * *

### 10.1.2 任务通知——直接任务通信

“任务通知”允许任务与其他任务交互，并
与 ISR 同步，无需单独通信
对象。通过使用任务通知，任务或 ISR 可以发送事件
直接进入接收任务。如图 10.2 所示。


<a name="fig10.2" title="Figure 10.2 A task notification used to send an event directly from one task to another"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image77.png)    
***图 10.2*** *用于将事件直接从一个任务发送到另一个任务的任务通知*

* * *

任务通知功能是可选的。包含任务
通知功能将 FreeRTOSConfig.h 中的 `configUSE_TASK_NOTIFICATIONS` 设置为 1。

当`configUSE_TASK_NOTIFICATIONS`设置为1时，每个任务至少有一个
“通知状态”，可以是“待处理”或“未待处理”，
以及“通知值”，它是一个 32 位无符号整数。当一个
任务收到通知后，将其通知状态设置为待处理。
当任务读取其通知值时，将设置其通知状态
至未决。  如果 `configTASK_NOTIFICATION_ARRAY_ENTRIES` 设置为一个值
\> 1 则有多个由索引标识的通知状态和值。

任务可以在阻塞状态下等待，并有一个可选的超时时间，以等待其完成任务。
通知状态变为待处理。

### 10.1.3 范围

本章讨论：

- 任务的通知状态和通知值。
- 如何以及何时可以使用任务通知来代替任务通知
  通信对象，例如信号量。
- 使用任务通知代替任务通知的优点
  通讯对象。

## 10.2 任务通知；优点和局限性

### 10.2.1 任务通知的性能优势

使用任务通知向任务发送事件或数据是
比使用队列、信号量或事件组快得多
执行等效操作。

### 10.2.2 RAM 任务通知的占用空间优势

同样，使用任务通知将事件或数据发送到任务
与使用队列、信号量或事件相比，需要的 RAM 显着减少
组来执行等效操作。这是因为每个
必须创建通信 object (queue, semaphore or event group)
在使用之前，同时启用任务通知功能
有固定的开销。  任务通知的 RAM 成本为
`configTASK_NOTIFICATION_ARRAY_ENTRIES` * 每个任务 5 字节。  的
`configTASK_NOTIFICATION_ARRAY_ENTRIES`的默认值为1
任务通知的默认大小是每个任务 5 个字节。

### 10.2.3 任务通知的限制

任务通知比通信更快并且使用更少的 RAM
对象，但任务通知并不是所有场景都可以使用。这个
部分记录了无法发送任务通知的场景
使用：

- 向 ISR 发送事件或数据

通信对象可用于从 ISR 发送事件和数据
  到任务，以及从任务到 ISR。

任务通知可用于将事件和数据从 ISR 发送到
  任务，但它们不能用于将事件或数据从任务发送到
  ISR。

- 启用多个接收任务

任何知道的任务或 ISR 都可以访问通信对象
  它的句柄（可能是队列句柄、信号量句柄或事件句柄）
  组句柄）。任意数量的任务和 ISR 都可以处理事件或数据
  发送到任何给定的通信对象。

任务通知直接发送到接收任务，因此他们可以
  仅由通知发送到的任务处理。
  然而，这在实际情况中很少是一个限制，因为虽然
  多个任务和 ISR 发送到同一个是很常见的
  通信对象，很少有多个任务和ISR
  从同一通信对象接收。

- 缓冲多个数据项

队列是一种可以保存多个数据的通信对象
  一次项目。已发送到队列但尚未发送的数据
  从队列接收的数据被缓冲在队列对象内。

任务通知通过更新接收方向任务发送数据
  任务的通知值。任务的通知值只能保存
  一次一个值。

- 广播到多个任务

事件组是一个通信对象，可用于发送事件
  一次处理多个任务的事件。

任务通知直接发送到接收任务，因此可以
  只由接收任务处理。

- 在阻塞状态下等待发送完成

如果通信对象暂时处于不再意味着的状态
  数据或事件可以写入其中（例如，当队列已满时）
  没有更多的数据可以发送到队列），然后任务尝试写入
  对象可以选择进入阻塞状态以等待其
  写操作完成。

如果任务尝试向已经存在的任务发送任务通知
  有待处理的通知，则无法发送
  等待接收任务重置其阻塞状态的任务
  通知状态。正如我们将看到的，这很少是一个限制
  使用任务通知的实际案例。

## 10.3 使用任务通知

### 10.3.1 任务通知 API 选项

任务通知是一项非常强大的功能，通常可用于
二进制信号量、计数信号量、事件组和
有时甚至要排队。这种广泛的使用场景可以
使用`xTaskNotify()` API函数发送任务来实现
通知，以及 `xTaskNotifyWait()` API 函数接收任务
通知。

然而，在大多数情况下，
`xTaskNotify()` 和 `xTaskNotifyWait()` API 功能不是必需的，并且
更简单的功能就足够了。因此，`xTaskNotifyGive()` API
函数是作为一个更简单但不太灵活的替代方案提供的
`xTaskNotify()` 和 `ulTaskNotifyTake()` API 函数作为
`xTaskNotifyWait()` 的更简单但灵活性较差的替代方案。

任务通知系统不限于单个通知事件。  的
配置参数 `configTASK_NOTIFICATION_ARRAY_ENTRIES` 默认设置为 1。
如果设置为大于 1 的值，则会在内部创建通知数组
每个任务。  这允许通过索引管理通知。  每项任务
通知 api 函数有一个索引版本。  使用非索引版本
将导致访问 notification[0] （数组中的第一个）。  `indexed`
每个 API 函数的版本由后缀 `Indexed` 标识，因此该函数
`xTaskNotify` 变为 `xTaskNotifyIndexed`。  为了简单起见，仅使用非索引
本书中将使用每个函数的版本。

任务通知 API 被实现为宏，调用
每个 API 函数类型的基础 `Generic` 版本。  简单来说 API
本书中宏将被称为函数。

#### 10.3.1.1 API 函数的完整列表 <sup>27</sup>

- `xTaskNotifyGive`
- `xTaskNotifyGiveIndexed`
- `vTaskNotifyGiveFromISR`
- `vTaskNotifyGiveIndexedFromISR`
- `vTaskNotifyTake`
- `vTaskNotifyTakeIndexed`
- `xTaskNotify`
- `xTaskNotifyIndexed`
- `xTaskNotifyWait`
- `xTaskNotifyWaitIndexed`
- `xTaskNotifyStateClear`
- `xTaskNotifyStateClearIndexed`
- `ulTaskNotifyValueClear`
- `ulTaskNotifyValueClearIndexed`
- `xTaskNotifyAndQueryIndexedFromISR`
- `xTaskNotifyAndQueryFromISR`
- `xTaskNotifyFromISR`
- `xTaskNotifyIndexedFromISR`
- `xTaskNotifyAndQuery`
- `xTaskNotifyAndQueryIndexed`

*(27)：这些函数实际上是作为宏实现的。*

>注意：`FromISR` 函数不存在用于接收通知，因为通知始终发送到任务并且中断不与任何任务关联。

### 10.3.2 xTaskNotifyGive() API 功能

`xTaskNotifyGive()` 直接向任务发送通知，并且
increments (adds one to) 接收任务的通知值。
调用`xTaskNotifyGive()`会设置接收任务的通知
如果尚未挂起，则状态为挂起。

提供 `xTaskNotifyGive()` API 函数以允许执行任务
通知被用作更轻量级和更快的替代方案
二进制或计数信号量。


<a name="list10.1" title="Listing 10.1 The xTaskNotifyGive() API function prototype"></a>


```c
BaseType_t xTaskNotifyGive( TaskHandle_t xTaskToNotify );
BaseType_t xTaskNotifyGiveIndexed( TaskHandle_t xTaskToNotify, UBaseType_t uxIndexToNotify );
```

***清单 10.1*** *xTaskNotifyGive() API 功能原型*

**xTaskNotifyGive()/xTaskNotifyGiveIndexed()参数及返回值**

- `xTaskToNotify`

通知发送到的任务的句柄 - 请参阅
  `xTaskCreate()` API 函数的 `pxCreatedTask` 参数
  有关获取任务句柄的信息。

- `uxIndexToNotify`

数组中的索引

- 返回值

`xTaskNotifyGive()` 是调用 `xTaskNotify()` 的宏。的
  由宏传递到 `xTaskNotify()` 的参数设置为
  `pdPASS` 是唯一可能的返回值。 `xTaskNotify()`描述
  本书稍后会介绍。

### 10.3.3 vTaskNotifyGiveFromISR() API 功能

`vTaskNotifyGiveFromISR()` 是 `xTaskNotifyGive()` 的一个版本，可以
在中断服务程序中使用。


<a name="list10.2" title="Listing 10.2 The vTaskNotifyGiveFromISR() API function prototype"></a>


```c
void vTaskNotifyGiveFromISR( TaskHandle_t xTaskToNotify,
                             BaseType_t *pxHigherPriorityTaskWoken );
```

***清单 10.2*** *vTaskNotifyGiveFromISR() API 函数原型*

**vTaskNotifyGiveFromISR()参数及返回值**

- `xTaskToNotify`

通知发送到的任务的句柄 - 请参阅
  `xTaskCreate()` API 函数的 `pxCreatedTask` 参数
  有关获取任务句柄的信息。

- `pxHigherPriorityTaskWoken`

如果通知发送到的任务正在等待
  阻塞状态接收通知，然后发送
  通知将导致任务离开阻塞状态。

如果调用 `vTaskNotifyGiveFromISR()` 导致任务离开
  阻塞状态，且非阻塞任务的优先级高于
  当前执行任务的优先级（之前执行的任务）
  中断），然后，在内部，`vTaskNotifyGiveFromISR()` 将设置
  `*pxHigherPriorityTaskWoken` 至 `pdTRUE`。

如果 `vTaskNotifyGiveFromISR()` 将此值设置为 `pdTRUE`，则上下文
  切换应该在中断退出之前执行。这将
  确保中断直接返回到最高优先级的Ready
  状态任务。

与所有中断安全 API 功能一样，
  使用前必须将 `pxHigherPriorityTaskWoken` 参数设置为 `pdFALSE`。

### 10.3.4 ulTaskNotifyTake() API 功能

`ulTaskNotifyTake()` 允许任务在阻塞状态下等待
通知值大于零，并且 decrements
(subtracts one from) 或清除之前任务的通知值
返回。

提供 `ulTaskNotifyTake()` API 函数以允许执行任务
通知被用作更轻量级和更快的替代方案
二进制或计数信号量。


<a name="list10.3" title="Listing 10.3 The ulTaskNotifyTake() API function prototype"></a>


```c
uint32_t ulTaskNotifyTake( BaseType_t xClearCountOnExit, TickType_t
xTicksToWait );
```

***清单 10.3*** *ulTaskNotifyTake() API 函数原型*

**ulTaskNotifyTake()参数及返回值**

- `xClearCountOnExit`

如果 `xClearCountOnExit` 设置为 `pdTRUE`，则调用任务的
  在调用之前通知值将被清除为零
  `ulTaskNotifyTake()` 返回。

如果 `xClearCountOnExit` 设置为 `pdFALSE`，并且调用任务的
  通知值大于零，则调用任务的
  通知值将在调用之前递减
  `ulTaskNotifyTake()` 返回。

- `xTicksToWait`

调用任务应在任务中保留的最长时间
  阻塞状态等待其通知值大于
  零。

区块时间以滴答周期为单位指定，因此它的绝对时间
  表示取决于滴答频率。宏 `pdMS_TO_TICKS()`
  可用于将以毫秒为单位的时间转换为时间
  以刻度指定。

将 `xTicksToWait` 设置为 `portMAX_DELAY` 将导致任务等待
  indefinitely (without timing out)，前提是设置了 `INCLUDE_vTaskSuspend`
  为 `FreeRTOSConfig.h` 中的 1。

- 返回值

返回值是调用任务的通知值
  *之前*它被清除为零或递减，如下所示
  由 `xClearCountOnExit` 参数的值指定。

如果区块时间为 specified (`xTicksToWait` was not zero)，并且
  返回值不为零，则调用任务可能是
  置于阻塞状态以等待其通知值
  大于零，并且其通知值在
  区块时间已过。

如果区块时间为 specified (`xTicksToWait` was not zero)，并且
  返回值为零，则调用任务被放入Blocked
  状态等待其通知值大于零，但是
  指定的区块时间在此之前已过期。


<a name="example10.1" title="Example 10.1 Using a task notification in place of a semaphore, method 1"></a>
---
***示例 10.1*** *使用任务通知代替信号量，方法 1*

---

例 7.1 使用二进制信号量来解除对任务的阻塞
中断服务例程——有效地将任务与
中断。此示例复制了示例 7.1 的功能，但是
使用直接任务通知代替二进制信号量。

清单10.4显示了同步任务的实现
随着中断。对 `xSemaphoreTake()` 的调用用于
示例 7.1 已替换为对 `ulTaskNotifyTake()` 的调用。

将`ulTaskNotifyTake()` `xClearCountOnExit`参数设置为`pdTRUE`，
这会导致接收任务的通知值被清除
在 `ulTaskNotifyTake()` 返回之前清零。因此有必要
处理每次调用之间已经可用的所有事件
`ulTaskNotifyTake()`。在例7.1中，因为使用了二进制信号量，
待处理事件的数量必须由硬件确定，
这并不总是实用的。在示例 10.1 中，待处理的数量
事件从 `ulTaskNotifyTake()` 返回。

调用 `ulTaskNotifyTake` 之间发生的中断事件是
锁存在任务的通知值中，并调用
如果调用任务已经存在，`ulTaskNotifyTake()`将立即返回
有待处理的通知。


<a name="list10.4" title="Listing 10.4 The implementation of the task to which the interrupt processing is deferred (the task that..."></a>


```c
/* The rate at which the periodic task generates software interrupts. */
const TickType_t xInterruptFrequency = pdMS_TO_TICKS( 500UL );

static void vHandlerTask( void *pvParameters )
{
    /* xMaxExpectedBlockTime is set to be a little longer than the maximum
       expected time between events. */
    const TickType_t xMaxExpectedBlockTime = xInterruptFrequency +
                                             pdMS_TO_TICKS( 10 );
    uint32_t ulEventsToProcess;

    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {
        /* Wait to receive a notification sent directly to this task from the
           interrupt service routine. */
        ulEventsToProcess = ulTaskNotifyTake( pdTRUE, xMaxExpectedBlockTime );
        if( ulEventsToProcess != 0 )
        {
            /* To get here at least one event must have occurred. Loop here 
               until all the pending events have been processed (in this case,
               just print out a message for each event). */
            while( ulEventsToProcess > 0 )
            {
                vPrintString( "Handler task - Processing event.\r\n" );
                ulEventsToProcess--;
            }
        }
        else
        {
            /* If this part of the function is reached then an interrupt did 
               not arrive within the expected time, and (in a real application)
               it may be necessary to perform some error recovery operations. */
        }
    }
}
```

***清单10.4*** *例10.1中中断处理为deferred (the task that synchronizes with the interrupt)的任务的实现*

用于生成软件中断的周期性任务打印一条消息
在中断产生之前，以及在中断产生之后再次
已生成。这允许观察执行顺序
产生的输出。

清单 10.5 显示了中断处理程序。这几乎没有其他作用
而不是直接向中断的任务发送通知
处理被推迟。


<a name="list10.5" title="Listing 10.5 The implementation of the interrupt service routine used in Example 10.1"></a>


```c
static uint32_t ulExampleInterruptHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken;

    /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as
       it will get set to pdTRUE inside the interrupt safe API function if a
       context switch is required. */
    xHigherPriorityTaskWoken = pdFALSE;

    /* Send a notification directly to the task to which interrupt processing 
       is being deferred. */
    vTaskNotifyGiveFromISR( /* The handle of the task to which the notification
                               is being sent. The handle was saved when the task
                               was created. */
                            xHandlerTask,

                            /* xHigherPriorityTaskWoken is used in the usual 
                               way. */
                            &xHigherPriorityTaskWoken );

    /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR(). If 
       xHigherPriorityTaskWoken was set to pdTRUE inside vTaskNotifyGiveFromISR()
       then calling portYIELD_FROM_ISR() will request a context switch. If
       xHigherPriorityTaskWoken is still pdFALSE then calling 
       portYIELD_FROM_ISR() will have no effect. The implementation of
       portYIELD_FROM_ISR() used by the Windows port includes a return statement,
       which is why this function does not explicitly return a value. */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```

***清单 10.5*** *例 10.1 中使用的中断服务程序的实现*

执行例 10.1 时产生的输出如图 10.3 所示。
正如预期的那样，它与示例 7.1 中生成的结果相同
被执行。 `vHandlerTask()` 一旦进入运行状态
产生中断，因此任务的输出分割输出
由周期性任务产生。图 10.4 提供了进一步的解释。


<a name="fig10.3" title="Figure 10.3 The output produced when Example 7.1 is executed"></a>
<a name="fig10.4" title="Figure 10.4 The sequence of execution when Example 10.1 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image78.png)    
***图 10.3*** *执行例 7.1 时产生的输出*

![](https://cloud.rocketpi.club/cloud/image79.png)    
***图10.4*** *执行例10.1时的执行顺序*

* * *

<a name="example10.2" title="Example 10.2 Using a task notification in place of a semaphore, method 2"></a>
---
***示例 10.2*** *使用任务通知代替信号量，方法 2*

---

在示例 10.1 中，`ulTaskNotifyTake()` `xClearOnExit` 参数设置为
`pdTRUE`。例 10.1 稍微修改例 10.1 以演示
当 `ulTaskNotifyTake()` `xClearOnExit` 参数改为
设置为 `pdFALSE`。

当`xClearOnExit`为`pdFALSE`时，调用`ulTaskNotifyTake()`只会
decrement (reduce by one) 调用任务的通知值，而不是
将其清除为零。因此，通知计数是
已发生的事件数与发生的事件数之间的差异
已处理的事件数。这使得结构
`vHandlerTask()` 以两种方式简化：

1. 等待处理的事件数量保存在
    通知值，因此不需要保存在本地
    变量。

2.每次调用之间只需要处理一个事件
    `ulTaskNotifyTake()`。

例 10.2 中使用的 `vHandlerTask()` 的实现如下所示
清单 10.6。


<a name="list10.6" title="Listing 10.6 The implementation of the task to which the interrupt processing is deferred (the task..."></a>


```c
static void vHandlerTask( void *pvParameters )
{
    /* xMaxExpectedBlockTime is set to be a little longer than the maximum 
       expected time between events. */
    const TickType_t xMaxExpectedBlockTime = xInterruptFrequency + 
                                             pdMS_TO_TICKS( 10 );

    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {
        /* Wait to receive a notification sent directly to this task from the
           interrupt service routine. The xClearCountOnExit parameter is now
           pdFALSE, so the task's notification value will be decremented by
           ulTaskNotifyTake(), and not cleared to zero. */
        if( ulTaskNotifyTake( pdFALSE, xMaxExpectedBlockTime ) != 0 )
        {
            /* To get here an event must have occurred. Process the event (in 
               this case just print out a message). */
            vPrintString( "Handler task - Processing event.\r\n" );
        }
        else
        {
            /* If this part of the function is reached then an interrupt did 
               not arrive within the expected time, and (in a real application)
               it may be necessary to perform some error recovery operations. */
        }
    }
}
```

***清单10.6*** *例102中中断处理为deferred (the task that synchronizes with the interrupt)的任务的实现*

出于演示目的，中断服务程序也已
修改为每个中断发送多个任务通知，并且
这样做可以模拟高频发生的多个中断。的
例 10.2 中使用的中断服务程序的实现是
如清单 10.7 所示。


<a name="list10.7" title="Listing 10.7 The implementation of the interrupt service routine used in Example 10.2"></a>


```c
static uint32_t ulExampleInterruptHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;

    /* Send a notification to the handler task multiple times. The first
       'give' will unblock the task, the following 'gives' are to demonstrate 
       that the receiving task's notification value is being used to count 
       (latch) events - allowing the task to process each event in turn. */
    vTaskNotifyGiveFromISR( xHandlerTask, &xHigherPriorityTaskWoken );
    vTaskNotifyGiveFromISR( xHandlerTask, &xHigherPriorityTaskWoken );
    vTaskNotifyGiveFromISR( xHandlerTask, &xHigherPriorityTaskWoken );

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```

***清单 10.7*** *例 10.2 中使用的中断服务程序的实现*

执行例 10.2 时产生的输出如图 10.5 所示。
可以看出，`vHandlerTask()` 每次处理所有三个事件
产生中断。


<a name="fig10.5" title="Figure 10.5 The output produced when Example 10.2 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image80.jpg)    
***图 10.5*** *执行例 10.2 时产生的输出*

* * *

### 10.3.5 xTaskNotify() 和 xTaskNotifyFromISR() API 功能

`xTaskNotify()` 是 `xTaskNotifyGive()` 的功能更强大的版本，可以
用于更新任何接收任务的通知值
以下方式：

- Increment (add one to) 接收任务的通知值，in
  这种情况 `xTaskNotify()` 相当于 `xTaskNotifyGive()`。

- 在接收任务的通知值中设置一位或多位。
  这允许任务的通知值用作打火机
  重量和更快的替代项目组。

- 在接收任务的通知中写入一个全新的号码
  值，但前提是接收任务已读取其通知
  自上次更新以来的值。这允许任务的通知
  值提供与队列提供的功能类似的功能
  长度为一。

- 在接收任务的通知中写入一个全新的号码
  值，即使接收任务尚未读取其通知
  自上次更新以来的值。这允许任务的通知
  值提供与提供的功能类似的功能
  `xQueueOverwrite()` API 功能。由此产生的行为有时是
  称为“邮箱”。

`xTaskNotify()` 比 `xTaskNotifyGive()` 更灵活、更强大，并且
由于额外的灵活性和力量，它也多一点
使用起来很复杂。

`xTaskNotifyFromISR()` 是 `xTaskNotify()` 的一个版本，可用于
一个中断服务程序，因此有一个额外的
`pxHigherPriorityTaskWoken` 参数。

调用`xTaskNotify()`总是会设置接收任务的通知
如果尚未挂起，则状态为挂起。


<a name="list10.8" title="Listing 10.8 Prototypes for the xTaskNotify() and xTaskNotifyFromISR() API functions"></a>


```c
BaseType_t xTaskNotify( TaskHandle_t xTaskToNotify,
                        uint32_t ulValue,
                        eNotifyAction eAction );

BaseType_t xTaskNotifyFromISR( TaskHandle_t xTaskToNotify,
                               uint32_t ulValue,
                               eNotifyAction eAction,
                               BaseType_t *pxHigherPriorityTaskWoken );
```

***清单 10.8*** *xTaskNotify() 和 xTaskNotifyFromISR() API 函数的原型*

**xTaskNotify()参数及返回值**

- `xTaskToNotify`

通知发送到的任务的句柄 - 请参阅
  `xTaskCreate()` API 函数的 `pxCreatedTask` 参数
  有关获取任务句柄的信息。

- `ulValue`

ulValue 的使用方式取决于 eNotifyAction 值。见下文。

- `eNotifyAction`

一个枚举类型，指定如何更新接收任务的
  通知值。见下文。

- 返回值

`xTaskNotify()` 将返回 `pdPASS` *除非*在下面提到的一种情况下。

**有效 xTaskNotify() eNotifyAction 参数值及其结果
对接收任务通知值的影响**

- `eNoAction`

接收任务的通知状态设置为pending，无需
  它的通知值正在更新。 `xTaskNotify()` `ulValue`
  不使用参数。

`eNoAction` 操作允许将任务通知用作
  二进制信号量的更快、更轻的替代方案。

- `eSetBits`

接收任务的通知值按位 OR 与
  `xTaskNotify()` `ulValue` 参数中传递的值。例如，如果
  `ulValue` 设置为 0x01，则接收任务的位 0 将被设置
  通知值。另一个例子，如果 `ulValue` 是 0x06（二进制 0110）
  那么位1和位2将被设置在接收任务的通知值中。

`eSetBits` 操作允许将任务通知用作更快的通知
  以及事件组的更轻量级替代方案。

- `eIncrement`

接收任务的通知值递增。的
  `xTaskNotify()` `ulValue` 参数未使用。

`eIncrement` 操作允许将任务通知用作
  二进制或计数信号量的更快、更轻量级的替代方案，
  相当于更简单的 `xTaskNotifyGive()` API 函数。

- `eSetValueWithoutOverwrite`

如果接收任务之前有待处理的通知
  `xTaskNotify()` 被调用，然后不采取任何操作，`xTaskNotify()` 将
  返回 `pdFAIL`。

如果接收任务之前没有待处理的通知
  `xTaskNotify()`被调用，则接收任务的通知值
  设置为 `xTaskNotify()` `ulValue` 参数中传递的值。

- `eSetValueWithOverwrite`

接收任务的通知值设置为传递的值
  `xTaskNotify()` `ulValue` 参数中，无论是否
  接收任务在 `xTaskNotify()` 之前有一个待处理的通知
  打电话或不打电话。

### 10.3.6 xTaskNotifyWait() API 功能

`xTaskNotifyWait()` 是 `ulTaskNotifyTake()` 的功能更强大的版本。它
允许任务以可选的超时时间等待调用任务的
通知状态变为待处理（如果它尚未处于待处理状态）。
`xTaskNotifyWait()` 提供了在调用中清除位的选项
进入函数和退出时任务的通知值
从函数。


<a name="list10.9" title="Listing 10.9 The xTaskNotifyWait() API function prototype"></a>


```c
BaseType_t xTaskNotifyWait( uint32_t   ulBitsToClearOnEntry,
                            uint32_t   ulBitsToClearOnExit,
                            uint32_t   *pulNotificationValue,
                            TickType_t xTicksToWait );
```

***清单 10.9*** *xTaskNotifyWait() API 功能原型*

**xTaskNotifyWait()参数及返回值**

- `ulBitsToClearOnEntry`

如果调用任务之前没有待处理的通知
  称为 `xTaskNotifyWait()`，则 `ulBitsToClearOnEntry` 中设置的任何位都将
  在进入该函数时在任务的通知值中清除。

例如，如果 `ulBitsToClearOnEntry` 为 0x01，则该位的位 0
  任务的通知值将被清除。作为另一个例子，设置
  `ulBitsToClearOnEntry` 至 0xffffffff (`ULONG_MAX`) 将清除所有位
  在任务的通知值中，有效地将值清除为 0。

- `ulBitsToClearOnExit`

如果调用任务退出 `xTaskNotifyWait()` 因为它收到了
  通知，或者因为它已经有一个待处理的通知
  调用 `xTaskNotifyWait()`，然后在 `ulBitsToClearOnExit` 中设置任何位
  任务退出前会在任务的通知值中被清除
  `xTaskNotifyWait()` 函数。

任务的通知值被清除后，这些位被清除
  保存在`*pulNotificationValue`中（参见说明
  `pulNotificationValue` 以下）。

例如，如果 `ulBitsToClearOnExit` 为 0x03，则 `ulBitsToClearOnExit` 的位 0 和位 1
  任务的通知值将在函数执行之前被清除
  退出。

将 `ulBitsToClearOnExit` 设置为 0xffffffff (`ULONG_MAX`) 将清除所有
  任务通知值中的位，有效地清除
  值为 0。

- `pulNotificationValue`

用于传递任务的通知值。复制的值
  `*pulNotificationValue` 是任务的通知值
  在由于 `ulBitsToClearOnExit` 设置而清除任何位之前。

`pulNotificationValue` 为可选参数，可设置为 NULL
  如果不需要。

- `xTicksToWait`

调用任务应在任务中保留的最长时间
  阻塞状态等待其通知状态变为挂起。

区块时间以滴答周期为单位指定，因此它的绝对时间
  表示取决于滴答频率。宏 `pdMS_TO_TICKS()`
  可用于将以毫秒为单位的时间转换为时间
  以刻度指定。

将 `xTicksToWait` 设置为 `portMAX_DELAY` 将导致任务等待
  indefinitely (without timing out)，前提是设置了 `INCLUDE_vTaskSuspend`
  为 `FreeRTOSConfig.h` 中的 1。

- 返回值

有两种可能的返回值：

- `pdTRUE`

这表示由于通知已返回而返回 `xTaskNotifyWait()`
    已收到，或者因为调用任务已有待处理的通知
    当 `xTaskNotifyWait()` 被调用时。

如果区块时间为 specified (`xTicksToWait` was not zero)，那么它是
    可能调用任务被置于阻塞状态，以
    等待其通知状态变为待处理，但其通知
    在区块时间到期之前，状态被设置为待处理。

- `pdFALSE`

这表明`xTaskNotifyWait()`返回而没有调用
    任务接收任务通知。

如果 `xTicksToWait` 不为零，则调用任务将被
    保持阻塞状态以等待其通知状态变为
    待处理，但在此之前指定的区块时间已过期。

### 10.3.7 外围设备驱动程序中使用的任务通知：UART 示例

外设驱动库提供了执行常见功能的函数
硬件接口上的操作。适用的外围设备示例
通常提供的此类库包括通用异步
接收器和 Transmitters (UARTs)、串行外设 Interface (SPI)
端口、模拟转数字 converters (ADCs) 和以太网端口。示例
此类库通常提供的功能包括
初始化外设、向外设发送数据、接收数据
来自外围设备。

部分外设操作需要较长时间才能完成。
此类操作的示例包括高精度 ADC 转换，以及
在 UART 上传输大数据包。在这些情况下
poll (repeatedly read)可实现驱动库函数
外设的状态寄存器来确定操作何时进行
完成。然而，以这种方式进行轮询几乎总是浪费，因为
它利用了 100% 的处理器时间，但没有进行生产性处理
正在执行。废物在以下地区尤其昂贵
多任务系统，其中轮询外围设备的任务可能是
阻止执行具有较低优先级的任务
要执行的生产性处理。

为了避免浪费处理时间，高效的 RTOS
意识到设备驱动程序应该是中断驱动的，并给出一个任务
启动一个漫长的操作 在阻塞状态下等待的选项
以便操作完成。这样，较低优先级的任务就可以
当执行冗长操作的任务位于
阻塞状态，没有任务使用处理时间，除非它们可以使用它
富有成效地。

RTOS 感知驱动程序库使用二进制文件是常见做法
信号量将任务置于阻塞状态。该技术是
清单 10.10 中的伪代码演示了这一点，它提供了
在 UART 上传输数据的 RTOS 感知库函数的概述
端口。在清单 10.10 中：

- `xUART` 是描述 UART 外设的结构，并保存
    状态信息。该结构的 `xTxSemaphore` 成员是
    `SemaphoreHandle_t` 类型的变量。假设信号量有
    已经被创建了。

- `xUART_Send()` 函数不包含任何互斥
    逻辑。如果多个任务要使用 `xUART_Send()`
    函数，那么应用程序编写者将必须管理相互的
    应用程序本身内的排除。例如，一个任务可能是
    在调用 `xUART_Send()` 之前需要获取互斥体。

- `xSemaphoreTake()` API函数用于放置调用任务
    UART 传输启动后进入阻塞状态。

- `xSemaphoreGiveFromISR()` API函数用于删除任务
    传输完成后从阻塞状态，
    是当UART外设的发送结束中断服务程序
    执行。


<a name="list10.10" title="Listing 10.10 Pseudo code demonstrating how a binary semaphore can be used in a driver library transmit..."></a>


```c
/* Driver library function to send data to a UART. */

BaseType_t xUART_Send( xUART *pxUARTInstance, 
                       uint8_t *pucDataSource, 
                       size_t uxLength )
{
    BaseType_t xReturn;

    /* Ensure the UART's transmit semaphore is not already available by 
       attempting to take the semaphore without a timeout. */
    xSemaphoreTake( pxUARTInstance->xTxSemaphore, 0 );

    /* Start the transmission. */
    UART_low_level_send( pxUARTInstance, pucDataSource, uxLength );

    /* Block on the semaphore to wait for the transmission to complete. If 
       the semaphore is obtained then xReturn will get set to pdPASS. If the 
       semaphore take operation times out then xReturn will get set to pdFAIL. 
       Note that, if the interrupt occurs between UART_low_level_send() being 
       called, and xSemaphoreTake() being called, then the event will be 
       latched in the binary semaphore, and the call to xSemaphoreTake() will 
       return immediately. */
    xReturn = xSemaphoreTake( pxUARTInstance->xTxSemaphore, 
                              pxUARTInstance->xTxTimeout );

    return xReturn;
}
/*-----------------------------------------------------------*/

/* The service routine for the UART's transmit end interrupt, which executes 
   after the last byte has been sent to the UART. */
void xUART_TransmitEndISR( xUART *pxUARTInstance )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Clear the interrupt. */
    UART_low_level_interrupt_clear( pxUARTInstance );

    /* Give the Tx semaphore to signal the end of the transmission. If a task 
       is Blocked waiting for the semaphore then the task will be removed from
       the Blocked state. */
    xSemaphoreGiveFromISR( pxUARTInstance->xTxSemaphore, 
                           &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```

***清单 10.10*** *演示如何在驱动程序库传输函数中使用二进制信号量的伪代码*

清单 10.10 中演示的技术是完全可行的，并且
确实是常见做法，但它有一些缺点：

- 该库使用多个信号量，这增加了其 RAM
  足迹。

- 信号量在创建之前无法使用，因此需要一个库
  使用信号量的函数只有在明确声明后才能使用
  已初始化。

- 信号量是适用于广泛范围的通用对象
  用例数量；它们包含允许任意数量的任务执行的逻辑
  在阻塞状态下等待信号量变得可用，并且
  select (in a deterministic manner) 要从
  当信号量变得可用时处于阻塞状态。执行中
  该逻辑需要有限的时间，并且处理开销是
  在所示场景中不必要的是清单 10.10，其中有
  在任何给定的情况下，不能有多个任务在等待信号量
  时间。

清单 10.11 演示了如何通过使用任务来避免这些缺点
通知代替二进制信号量。

> *注意：如果图书馆使用任务通知，则该图书馆的
> 文档必须明确说明调用库函数可以
> 更改调用任务的通知状态和通知值。*

在清单 10.11 中：

- `xUART` 结构中的 `xTxSemaphore` 成员已替换为
  `xTaskToNotify` 成员。 `xTaskToNotify` 是类型变量
  `TaskHandle_t`，用于保存正在执行的任务的句柄
  等待 UART 操作完成。

- `xTaskGetCurrentTaskHandle()` FreeRTOS API 函数用于
  获取处于运行状态的任务句柄。

- 该库不创建任何 FreeRTOS 对象，因此不会产生
  RAM 开销，并且不需要显式初始化。

- 任务通知直接发送到正在等待的任务
  UART 操作完成，因此没有不必要的逻辑
  被执行。

`xUART` 结构的 `xTaskToNotify` 成员可从以下两个地址访问：
任务和中断服务程序，要求考虑
给出了处理器如何更新其值：

- 如果 `xTaskToNotify` 通过单个内存写操作更新，则
  它可以在关键部分之外更新，正如所示
  清单 10.11。如果 `xTaskToNotify` 是 32 位，则会出现这种情况
  variable (`TaskHandle_t` was a 32-bit type)，以及处理器
  FreeRTOS 运行的是 32 位处理器。

- 如果需要多次内存写操作来更新
  `xTaskToNotify`，则 `xTaskToNotify` 只能从
  临界区——否则中断服务程序可能
  访问处于不一致状态的 `xTaskToNotify`。这个
  如果 `xTaskToNotify` 是 32 位变量，并且
  运行 FreeRTOS 的处理器是 16 位处理器，因为它
  需要两次 16 位内存写操作来更新所有
  32 位。

在 FreeRTOS 实现内部，`TaskHandle_t` 是
指针，因此 `sizeof( TaskHandle_t )` 始终等于 `sizeof( void * )`。


<a name="list10.11" title="Listing 10.11 Pseudo code demonstrating how a task notification can be used in a driver library transmit..."></a>


```c
/* Driver library function to send data to a UART. */
BaseType_t xUART_Send( xUART *pxUARTInstance, 
                       uint8_t *pucDataSource, 
                       size_t uxLength )
{
    BaseType_t xReturn;

    /* Save the handle of the task that called this function. The book text
       contains notes as to whether the following line needs to be protected 
       by a critical section or not. */
    pxUARTInstance->xTaskToNotify = xTaskGetCurrentTaskHandle();

    /* Ensure the calling task does not already have a notification pending by 
       calling ulTaskNotifyTake() with the xClearCountOnExit parameter set to 
       pdTRUE, and a block time of 0 (don't block). */
    ulTaskNotifyTake( pdTRUE, 0 );

    /* Start the transmission. */
    UART_low_level_send( pxUARTInstance, pucDataSource, uxLength );

    /* Block until notified that the transmission is complete. If the 
       notification is received then xReturn will be set to 1 because the ISR 
       will have incremented this task's notification value to 1 (pdTRUE). If 
       the operation times out then xReturn will be 0 (pdFALSE) because this 
       task's notification value will not have been changed since it was 
       cleared to 0 above. Note that, if the ISR executes between the calls to
       UART_low_level_send() and the call to ulTaskNotifyTake(), then the 
       event will be latched in the task's notification value, and the call to 
       ulTaskNotifyTake() will return immediately. */
    xReturn = ( BaseType_t ) ulTaskNotifyTake( pdTRUE, 
                                               pxUARTInstance->xTxTimeout );

    return xReturn;
}
/*-----------------------------------------------------------*/

/* The ISR that executes after the last byte has been sent to the UART. */
void xUART_TransmitEndISR( xUART *pxUARTInstance )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* This function should not execute unless there is a task waiting to be 
       notified. Test this condition with an assert. This step is not strictly
       necessary, but will aid debugging. configASSERT() is described in 
       section 12.2. */
    configASSERT( pxUARTInstance->xTaskToNotify != NULL );

    /* Clear the interrupt. */
    UART_low_level_interrupt_clear( pxUARTInstance );

    /* Send a notification directly to the task that called xUART_Send(). If 
       the task is Blocked waiting for the notification then the task will be 
       removed from the Blocked state. */
    vTaskNotifyGiveFromISR( pxUARTInstance->xTaskToNotify,
                            &xHigherPriorityTaskWoken );

    /* Now there are no tasks waiting to be notified. Set the xTaskToNotify 
       member of the xUART structure back to NULL. This step is not strictly 
       necessary but will aid debugging. */
    pxUARTInstance->xTaskToNotify = NULL;
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```

***清单 10.11*** *演示如何在驱动程序库传输函数中使用任务通知的伪代码*

任务通知还可以替换接收函数中的信号量，如
伪代码清单 10.12 展示了
RTOS 感知库函数，用于在 UART 端口上接收数据。
参见清单 10.12：

- `xUART_Receive()` 函数不包含任何互斥
  逻辑。如果多个任务要使用 `xUART_Receive()`
  函数，那么应用程序编写者将必须管理相互的
  应用程序本身内的排除。例如，一个任务可能是
  在调用 `xUART_Receive()` 之前需要获取互斥体。

- UART 的接收中断服务程序放置字符
  由 UART 接收到 RAM 缓冲区中。 `xUART_Receive()`
  函数从 RAM 缓冲区返回字符。

- `xUART_Receive()` `uxWantedBytes` 参数用于指定
  要接收的字符数。如果 RAM 缓冲区尚未
  包含请求的数字字符，则调用任务为
  置于阻塞状态以等待收到该号码的通知
  缓冲区中的字符数增加了。使用 `while()` 循环
  重复此序列，直到接收缓冲区包含
  请求的字符数，或者发生超时。

- 调用任务可能多次进入阻塞状态。的
  因此，调整区块时间以考虑到
  自调用 `xUART_Receive()` 以来已经过去的时间。的
  调整确保 `xUART_Receive()` 内花费的总时间不
  不超过`xRxTimeout`成员指定的出块时间
  `xUART`结构。使用 FreeRTOS 调整区块时间
  `vTaskSetTimeOutState()` 和 `xTaskCheckForTimeOut()` 辅助函数。


<a name="list10.12" title="Listing 10.12 Pseudo code demonstrating how a task notification can be used in a driver library receive..."></a>


```c
/* Driver library function to receive data from a UART. */

size_t xUART_Receive( xUART *pxUARTInstance, 
                      uint8_t *pucBuffer,
                      size_t uxWantedBytes )
{
    size_t uxReceived = 0;
    TickType_t xTicksToWait;
    TimeOut_t xTimeOut;

    /* Record the time at which this function was entered. */
    vTaskSetTimeOutState( &xTimeOut );

    /* xTicksToWait is the timeout value - it is initially set to the maximum 
       receive timeout for this UART instance. */
    xTicksToWait = pxUARTInstance->xRxTimeout;

    /* Save the handle of the task that called this function. The book text 
       contains notes as to whether the following line needs to be protected 
       by a critical section or not. */
    pxUARTInstance->xTaskToNotify = xTaskGetCurrentTaskHandle();

    /* Loop until the buffer contains the wanted number of bytes, or a
       timeout occurs. */
    while( UART_bytes_in_rx_buffer( pxUARTInstance ) < uxWantedBytes )
    {
        /* Look for a timeout, adjusting xTicksToWait to account for the time
           spent in this function so far. */
        if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
        {
            /* Timed out before the wanted number of bytes were available, 
               exit the loop. */
            break;
        }

        /* The receive buffer does not yet contain the required amount of 
           bytes. Wait for a maximum of xTicksToWait ticks to be notified that 
           the receive interrupt service routine has placed more data into the 
           buffer. It does not matter if the calling task already had a 
           notification pending when it called this function, if it did, it
           would just iteration around this while loop one extra time. */
        ulTaskNotifyTake( pdTRUE, xTicksToWait );
    }

    /* No tasks are waiting for receive notifications, so set xTaskToNotify
       back to NULL. The book text contains notes as to whether the following 
       line needs to be protected by a critical section or not. */
    pxUARTInstance->xTaskToNotify = NULL;

    /* Attempt to read uxWantedBytes from the receive buffer into pucBuffer. 
       The actual number of bytes read (which might be less than uxWantedBytes)
       is returned. */
    uxReceived = UART_read_from_receive_buffer( pxUARTInstance, 
                                                pucBuffer,
                                                uxWantedBytes );
    return uxReceived;
}

/*-----------------------------------------------------------*/

/* The interrupt service routine for the UART's receive interrupt */
void xUART_ReceiveISR( xUART *pxUARTInstance )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Copy received data into this UART's receive buffer and clear the
       interrupt. */
    UART_low_level_receive( pxUARTInstance );

    /* If a task is waiting to be notified of the new data then notify it now. */
    if( pxUARTInstance->xTaskToNotify != NULL )
    {
        vTaskNotifyGiveFromISR( pxUARTInstance->xTaskToNotify,
                                &xHigherPriorityTaskWoken );
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}
```

***清单 10.12*** *演示如何在驱动程序库接收函数中使用任务通知的伪代码*

### 10.3.8 外围设备驱动程序中使用的任务通知：ADC 示例

上一节演示了如何使用 `vTaskNotifyGiveFromISR()`
从中断向任务发送任务通知。
`vTaskNotifyGiveFromISR()` 是一个使用简单的函数，但它的
能力有限；它只能发送任务通知
无价值的事件，它不能发送数据。本节演示如何
使用 `xTaskNotifyFromISR()` 通过任务通知事件发送数据。
清单 10.13 中的伪代码演示了该技术，
它提供了 RTOS 感知中断服务例程的概要
适用于模拟转数字 Converter (ADC)。在清单 10.13 中：

- 假设至少每 50 启动一次 ADC 转换
  毫秒。

- `ADC_ConversionEndISR()` 是中断服务程序
  ADC的转换结束中断，也就是执行的中断
  每次有新的 ADC 值可用时。

- `vADCTask()` 执行的任务处理由
  ADC。假设任务的句柄存储在
  创建任务时的 `xADCTaskToNotify`。

- `ADC_ConversionEndISR()` 将 `xTaskNotifyFromISR()` 与 `eAction` 结合使用
  参数设置为 `eSetValueWithoutOverwrite` 发送任务
  通知`vADCTask()`任务，并写入ADC的结果
  转换为任务的通知值。

- `vADCTask()`任务使用`xTaskNotifyWait()`等待通知
  新的 ADC 值可用，并检索结果
  ADC 从其通知值转换而来。


<a name="list10.13" title="Listing 10.13 Pseudo code demonstrating how a task notification can be used to pass a value to a task"></a>


```c
/* A task that uses an ADC. */
void vADCTask( void *pvParameters )
{
    uint32_t ulADCValue;
    BaseType_t xResult;

    /* The rate at which ADC conversions are triggered. */
    const TickType_t xADCConversionFrequency = pdMS_TO_TICKS( 50 );

    for( ;; )
    {
        /* Wait for the next ADC conversion result. */
        xResult = xTaskNotifyWait(
                    /* The new ADC value will overwrite the old value, so there
                       is no need to clear any bits before waiting for the new 
                       notification value. */
                    0,
                    /* Future ADC values will overwrite the existing value, so
                       there is no need to clear any bits before exiting 
                       xTaskNotifyWait(). */
                    0,
                    /* The address of the variable into which the task's 
                       notification value (which holds the latest ADC 
                       conversion result) will be copied. */
                    &ulADCValue,
                    /* A new ADC value should be received every 
                       xADCConversionFrequency ticks. */
                    xADCConversionFrequency * 2 );

        if( xResult == pdPASS )
        {
            /* A new ADC value was received. Process it now. */
            ProcessADCResult( ulADCValue );
        }
        else
        {
            /* The call to xTaskNotifyWait() did not return within the expected
               time, something must be wrong with the input that triggers the 
               ADC conversion, or with the ADC itself. Handle the error here. */
        }
    }
}

/*-----------------------------------------------------------*/

/* The interrupt service routine that executes each time an ADC conversion 
   completes. */
void ADC_ConversionEndISR( xADC *pxADCInstance )
{
    uint32_t ulConversionResult;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult;

    /* Read the new ADC value and clear the interrupt. */
    ulConversionResult = ADC_low_level_read( pxADCInstance );

    /* Send a notification, and the ADC conversion result, directly to
       vADCTask(). */
    xResult = xTaskNotifyFromISR( xADCTaskToNotify, /* xTaskToNotify parameter */
                                  ulConversionResult, /* ulValue parameter */
                                  eSetValueWithoutOverwrite, /* eAction parameter. */
                                  &xHigherPriorityTaskWoken );

    /* If the call to xTaskNotifyFromISR() returns pdFAIL then the task is not
       keeping up with the rate at which ADC values are being generated. 
       configASSERT() is described in section 11.2. */
    configASSERT( xResult == pdPASS );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```

***清单 10.13*** *演示如何使用任务通知将值传递给任务的伪代码*

### 10.3.9 直接在应用程序中使用的任务通知

本节通过演示增强了任务通知的力量
它们在假设应用中的使用，包括以下内容
功能：

- 该应用程序通过慢速互联网连接进行通信
  向远程数据服务器发送数据以及从远程数据服务器请求数据。从这里
  上，远程数据服务器被称为*云服务器*。

- 向云服务器请求数据后，请求任务
  必须在阻塞状态下等待请求的数据
  收到。

- 发送数据到云服务器后，发送任务必须等待
  处于阻塞状态以确认云服务器
  正确接收到数据。

软件设计原理图如图 10.6 所示。在图 10.6 中：

- 处理多个互联网连接的复杂性
  云服务器封装在单个 FreeRTOS 任务中。任务
  充当 FreeRTOS 应用程序中的代理服务器，并且
  称为*服务器任务*。

- 应用程序任务通过调用从云服务器读取数据
  `CloudRead()`。 `CloudRead()`与云服务器不通信
  直接，而是将读取请求发送到a上的服务器任务
  队列，并从服务器任务接收请求的数据
  任务通知。

- 应用程序任务通过调用将数据写入云服务器
  `CloudWrite()`。 `CloudWrite()` 不与云通信
  直接向服务器发送写请求，而不是直接向服务器发送写请求
  队列上的任务，并接收写操作的结果
  服务器任务作为任务通知。

`CloudRead()` 发送到服务器任务的结构和
`CloudWrite()` 函数如清单 10.14 所示。


<a name="fig10.6" title="Figure 10.6 The communication paths from the application tasks to the cloud server, and back again"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image81.png)    
***图10.6*** *从应用程序任务到云服务器并再次返回的通信路径*
* * *


<a name="list10.14" title="Listing 10.14 The structure and data type sent on a queue to the server task"></a>


```c
typedef enum CloudOperations
{
    eRead, /* Send data to the cloud server. */
    eWrite /* Receive data from the cloud server. */
} Operation_t;

typedef struct CloudCommand
{
    Operation_t eOperation; /* The operation to perform (read or write). */
    uint32_t ulDataID; /* Identifies the data being read or written. */
    uint32_t ulDataValue; /* Only used when writing data to the cloud server. */
    TaskHandle_t xTaskToNotify;/* The handle of the task performing the operation. */
} CloudCommand_t;
```

***清单 10.14*** *通过队列发送到服务器任务的结构和数据类型*

`CloudRead()` 的伪代码如清单 10.15 所示。该函数发送
它向服务器任务请求，然后调用 `xTaskNotifyWait()` 等待
阻塞状态，直到通知请求的数据已被接收
可用。

显示了显示服务器任务如何管理读取请求的伪代码
清单 10.16 中。当从云服务器接收到数据后，
服务器任务解锁应用程序任务，并将收到的
通过使用 `eAction` 调用 `xTaskNotify()` 将数据传输到应用程序任务
参数设置为 `eSetValueWithOverwrite`。

清单 10.16 显示了一个简化的场景，因为它假设 `GetCloudData()`
不必等待从云服务器获取值。


<a name="list10.15" title="Listing 10.15 The Implementation of the Cloud Read API Function"></a>


```c
/* ulDataID identifies the data to read. pulValue holds the address of the 
   variable into which the data received from the cloud server is to be written. */
BaseType_t CloudRead( uint32_t ulDataID, uint32_t *pulValue )
{
    CloudCommand_t xRequest;
    BaseType_t xReturn;

    /* Set the CloudCommand_t structure members to be correct for this read
       request. */
    xRequest.eOperation = eRead; /* This is a request to read data. */
    xRequest.ulDataID = ulDataID; /* A code that identifies the data to read. */
    xRequest.xTaskToNotify = xTaskGetCurrentTaskHandle(); /* Handle of the
                                                             calling task. */

    /* Ensure there are no notifications already pending by reading the
       notification value with a block time of 0, then send the structure to 
       the server task. */
    xTaskNotifyWait( 0, 0, NULL, 0 );
    xQueueSend( xServerTaskQueue, &xRequest, portMAX_DELAY );

    /* Wait for a notification from the server task. The server task writes
       the value received from the cloud server directly into this task's 
       notification value, so there is no need to clear any bits in the 
       notification value on entry to or exit from the xTaskNotifyWait() 
       function. The received value is written to *pulValue, so pulValue is
       passed as the address to which the notification value is written. */
    xReturn = xTaskNotifyWait( 0, /* No bits cleared on entry */
                               0, /* No bits to clear on exit */
                               pulValue, /* Notification value into *pulValue */
                               pdMS_TO_TICKS( 250 ) ); /* Wait 250ms maximum */

    /* If xReturn is pdPASS, then the value was obtained. If xReturn is pdFAIL,
       then the request timed out. */
    return xReturn;
}
```

***清单10.15*** *云读API功能的实现*


<a name="list10.16" title="Listing 10.16 The Server Task Processing a Read Request"></a>


```c
void ServerTask( void *pvParameters )
{
    CloudCommand_t xCommand;
    uint32_t ulReceivedValue;

    for( ;; )
    {
        /* Wait for the next CloudCommand_t structure to be received from a task */
        xQueueReceive( xServerTaskQueue, &xCommand, portMAX_DELAY );

        switch( xCommand.eOperation ) /* Was it a read or write request? */
        {
            case eRead:

                /* Obtain the requested data item from the remote cloud server */
                ulReceivedValue = GetCloudData( xCommand.ulDataID );

                /* Call xTaskNotify() to send both a notification and the value
                   received from the cloud server to the task that made the 
                   request. The handle of the task is obtained from the 
                   CloudCommand_t structure. */
                xTaskNotify( xCommand.xTaskToNotify, /* The task's handle is in
                                                        the structure */
                             ulReceivedValue, /* Cloud data sent as notification 
                                                 value */
                             eSetValueWithOverwrite );
                break;

                /* Other switch cases go here. */
        }
    }
}
```

***清单 10.16*** *服务器任务处理读取请求*

`CloudWrite()` 的伪代码如清单 10.17 所示。为了
演示中，`CloudWrite()` 返回一个按位状态代码，其中每个
状态码中的位被赋予了唯一的含义。四种状态示例
位由清单 10.17 顶部的 \#define 语句显示。

该任务清除四个状态位，将其请求发送到服务器
任务，然后调用 `xTaskNotifyWait()` 在 Blocked 状态等待
状态通知。


<a name="list10.17" title="Listing 10.17 The Implementation of the Cloud Write API Function"></a>


```c
/* Status bits used by the cloud write operation. */
#define SEND_SUCCESSFUL_BIT ( 0x01 << 0 )
#define OPERATION_TIMED_OUT_BIT ( 0x01 << 1 )
#define NO_INTERNET_CONNECTION_BIT ( 0x01 << 2 )
#define CANNOT_LOCATE_CLOUD_SERVER_BIT ( 0x01 << 3 )

/* A mask that has the four status bits set. */
#define CLOUD_WRITE_STATUS_BIT_MASK ( SEND_SUCCESSFUL_BIT |
                                      OPERATION_TIMED_OUT_BIT |
                                      NO_INTERNET_CONNECTION_BIT |
                                      CANNOT_LOCATE_CLOUD_SERVER_BIT )

uint32_t CloudWrite( uint32_t ulDataID, uint32_t ulDataValue )
{
    CloudCommand_t xRequest;
    uint32_t ulNotificationValue;

    /* Set the CloudCommand_t structure members to be correct for this
       write request. */
    xRequest.eOperation = eWrite; /* This is a request to write data */
    xRequest.ulDataID = ulDataID; /* A code that identifies the data being
                                     written */
    xRequest.ulDataValue = ulDataValue; /* Value of the data written to the
                                           cloud server. */
    xRequest.xTaskToNotify = xTaskGetCurrentTaskHandle(); /* Handle of the 
                                                             calling task. */

    /* Clear the three status bits relevant to the write operation by calling
       xTaskNotifyWait() with the ulBitsToClearOnExit parameter set to
       CLOUD_WRITE_STATUS_BIT_MASK, and a block time of 0. The current
       notification value is not required, so the pulNotificationValue 
       parameter is set to NULL. */
    xTaskNotifyWait( 0, CLOUD_WRITE_STATUS_BIT_MASK, NULL, 0 );

    /* Send the request to the server task. */
    xQueueSend( xServerTaskQueue, &xRequest, portMAX_DELAY );

    /* Wait for a notification from the server task. The server task writes
       a bitwise status code into this task's notification value, which is 
       written to ulNotificationValue. */
    xTaskNotifyWait( 0, /* No bits cleared on entry. */
                     CLOUD_WRITE_STATUS_BIT_MASK, /* Clear relevant bits to 0 on exit. */
                     &ulNotificationValue, /* Notified value. */
                     pdMS_TO_TICKS( 250 ) ); /* Wait a maximum of 250ms. */

    /* Return the status code to the calling task. */
    return ( ulNotificationValue & CLOUD_WRITE_STATUS_BIT_MASK );
}
```

***程序清单10.17*** *云写API函数的实现*

演示服务器任务如何管理写入请求的伪代码是
如清单 10.18 所示。当数据发送到云端服务器后，
服务器任务解锁应用程序任务，并按位发送
通过使用 `xTaskNotify()` 调用应用程序任务的状态代码
`eAction` 参数设置为 `eSetBits`。仅由定义的位
`CLOUD_WRITE_STATUS_BIT_MASK` 常数可以在接收中改变
任务的通知值，因此接收任务可以使用其他位
其用于其他目的的通知值。

清单 10.18 显示了一个简化的场景，因为它假设 `SetCloudData()`
不必等待从远程云获取确认
服务器。


<a name="list10.18" title="Listing 10.18 The Server Task Processing a Send Request"></a>


```c
void ServerTask( void *pvParameters )
{
    CloudCommand_t xCommand;
    uint32_t ulBitwiseStatusCode;

    for( ;; )
    {
        /* Wait for the next message. */
        xQueueReceive( xServerTaskQueue, &xCommand, portMAX_DELAY );

        /* Was it a read or write request? */
        switch( xCommand.eOperation )
        {
            case eWrite:

            /* Send the data to the remote cloud server. SetCloudData() returns
               a bitwise status code that only uses the bits defined by the
               CLOUD_WRITE_STATUS_BIT_MASK definition (shown in Listing 10.17). */
            ulBitwiseStatusCode = SetCloudData( xCommand.ulDataID,
                                                xCommand.ulDataValue );

            /* Send a notification to the task that made the write request. 
               The eSetBits action is used so any status bits set in 
               ulBitwiseStatusCode will be set in the notification value of 
               the task being notified. All the other bits remain unchanged. 
               The handle of the task is obtained from the CloudCommand_t
               structure. */
            xTaskNotify( xCommand.xTaskToNotify, /* The task's handle is in 
                                                    the structure. */
                         ulBitwiseStatusCode,    /* Cloud data sent as 
                                                    notification value. */
                         eSetBits );
            break;

            /* Other switch cases go here. */
        }
    }
}
```

***清单 10.18*** *服务器任务处理发送请求*
