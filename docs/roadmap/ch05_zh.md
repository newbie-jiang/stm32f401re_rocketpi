# 5 队列管理

## 5.1 简介

“队列”提供任务到任务、任务到中断和
中断到任务的通信机制。


### 5.1.1 范围

本章内容包括：

- 如何创建队列。
- 队列如何管理其包含的数据。
- 如何将数据发送到队列。
- 如何从队列接收数据。
- 阻塞队列意味着什么。
- 如何阻塞多个队列。
- 如何覆盖队列中的数据。
- 如何清除队列。
- 写入和读取队列时任务优先级的影响。

本章仅介绍任务到任务的通信。第 7 章涵盖
任务到中断和中断到任务通信。


## 5.2 队列的特性

### 5.2.1 数据存储

队列可以容纳有限数量的固定大小的数据项[^8]。的
队列可以容纳的最大项目数称为其“长度”。两者都
每个数据项的长度和大小在队列创建时设置
创建的。

[^8]：FreeRTOS 消息缓冲区，在 TBD 章节中描述，提供
比保存可变长度的队列更轻量级的替代方案
消息。

队列通常用作先进先出 Out (FIFO) 缓冲区，其中
数据写入队列的end (tail)并从队列中删除
队列的 front (head)。图 5.1 演示了正在写入的数据
并从用作 FIFO 的队列中读取。也是可以的
写入队列的前面，并覆盖已经存在的数据
在队列的前面。

<a name="fig5.1" title="Figure 5.1 An example sequence of writes to, and reads from a queue"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image31.png)   
***图 5.1*** *写入和读取队列的示例序列*

* * *

队列行为有两种实现方式：

1.按副本排队

通过复制排队意味着发送到队列的数据被复制字节
    字节进入队列。

1. 通过引用排队

按引用排队意味着队列仅保存指向数据的指针
    发送到队列，而不是数据本身。

FreeRTOS使用复制队列的方法，因为它既更
比按引用排队更强大且更易于使用，因为：

- 按复制排队并不会阻止队列也被用于
  按引用排队。例如，当数据大小为
  排队使得将数据复制到队列中是不切实际的，然后
  可以将指向数据的指针复制到队列中。

- 堆栈变量可以直接发送到队列，即使
  变量在声明它的函数之后将不存在
  已退出。

- 数据可以发送到队列，而无需首先分配缓冲区
  保存数据——然后将数据复制到分配的缓冲区中
  并对缓冲区的引用进行排队。

- 发送任务可以立即重用变量或缓冲区
  被发送到队列中。

- 发送任务和接收任务完整
  解耦；应用程序设计者不需要关心
  哪个任务“拥有”数据，或者哪个任务是
  负责发布数据。

- RTOS 完全负责分配内存
  用于存储数据。

- 内存保护系统限制对 RAM 的访问，在这种情况下
  仅当发送和
  接收任务都可以访问引用的数据。按副本排队
  允许数据跨越内存保护边界。


### 5.2.2 多任务访问

队列本身就是对象，可以被任何任务访问
或 ISR 知道它们的存在。任意数量的任务可以写入
同一个队列，并且任意数量的任务可以从同一个队列中读取。在
实践中，一个队列有多个写入者是很常见的，但是
队列有多个读取器的情况要少得多。


### 5.2.3 队列读取阻塞

当任务尝试从队列中读取时，它可以选择指定一个
‘阻塞’时间。这是任务保持阻塞状态的时间
等待队列中的数据可用，如果队列
已经是空的了。处于阻塞状态等待数据的任务
从队列中变为可用的状态会自动移至就绪状态
当另一个任务或中断将数据放入队列时的状态。的
任务也会自动从阻塞状态转移到
如果在数据变为之前指定的块时间到期，则处于就绪状态
可用。

队列可以有多个读取器，因此单个队列是可能的
有多个任务被阻塞等待数据。当这是
在这种情况下，当数据可用时，只有一项任务被解锁。的
未被阻塞的任务始终是最高优先级的任务
等待数据。如果两个或多个阻塞任务具有相同的优先级，则
被解除阻塞的任务是等待时间最长的任务。


### 5.2.4 队列写入阻塞

就像从队列读取时一样，任务可以选择指定一个块
写入队列时的时间。在这种情况下，出块时间是
任务处于阻塞状态等待的最长时间
如果队列已经可用，则队列上有可用空间
满。

队列可以有多个写入者，因此队列可能已满
有多个任务被阻止等待完成发送
操作。在这种情况下，当空间不足时，只有一个任务被解锁
队列中的内容变为可用。被解锁的任务始终是
正在等待空间的最高优先级任务。如果有两个或多个阻塞任务
具有相同优先级，那么被解除阻塞的任务就是已经被阻塞的任务
等待时间最长的。


### 5.2.5 多个队列上的阻塞

队列可以分组，允许任务进入阻塞状态
状态以等待数据在任何队列上变得可用
设置。第 5.6 节, 从多个队列接收，演示了队列
套。


### 5.2.6 创建队列：静态分配和动态分配的队列

队列由句柄引用，句柄是类型变量
`QueueHandle_t`。队列在使用之前必须显式创建。

两个 API 函数创建队列：`xQueueCreate()`、`xQueueCreateStatic()`。

每个队列需要两个 RAM 块，第一个保存其数据
结构，第二个用于保存排队数据。 `xQueueCreate()` 分配
从 heap (dynamically) 中获取所需的 RAM。 `xQueueCreateStatic()`用途
预先分配的 RAM 作为参数传递到函数中。


## 5.3 使用队列

### 5.3.1 xQueueCreate() API 功能

清单 5.1 显示了 `xQueueCreate()` 函数原型。
`xQueueCreateStatic()` 有两个附加参数指向
预先分配的内存用于保存队列的数据结构和数据存储
区，分别。

<a name="list5.1" title="Listing 5.1 The xQueueCreate() API function prototype"></a>

```c
QueueHandle_t xQueueCreate( UBaseType_t uxQueueLength, UBaseType_t uxItemSize );
```
***清单 5.1*** *xQueueCreate() API 功能原型*


**xQueueCreate()参数及返回值：**

- `uxQueueLength`

正在创建的队列可以容纳的最大项目数
  任何一次。

- `uxItemSize`

可存储在队列中的每个数据项的大小（以字节为单位）。

- 返回值

如果返回 NULL，则无法创建队列，因为
  FreeRTOS 没有足够的堆内存来分配
  队列数据结构和存储区域。第 2 章提供了更多内容
  有关 FreeRTOS 堆的信息。

如果返回非 NULL 值，则队列已创建
  成功，返回值是创建的句柄
  队列。

`xQueueReset()` 是 API 函数，用于恢复以前创建的队列
到原来的空状态。


### 5.3.2 xQueueSendToBack() 和 xQueueSendToFront() API 功能

正如预期的那样，`xQueueSendToBack()` 将数据发送到 back (tail)
队列的 `xQueueSendToFront()` 将数据发送到队列的 front (head)
队列。

`xQueueSend()` 相当于且完全相同，
`xQueueSendToBack()`。

> *注意：切勿从电话呼叫 `xQueueSendToFront()` 或 `xQueueSendToBack()`
> 中断服务程序。中断安全版本
>应使用 `xQueueSendToFrontFromISR()` 和 `xQueueSendToBackFromISR()`
> 在他们的位置上。这些内容将在第 7 章中进行描述。*

<a name="list5.2" title="Listing 5.2 The xQueueSendToFront() API function prototype"></a>


```c
BaseType_t xQueueSendToFront( QueueHandle_t xQueue,
                              const void * pvItemToQueue,
                              TickType_t xTicksToWait );
```
***清单 5.2*** *xQueueSendToFront() API 功能原型*


<a name="list5.3" title="Listing 5.3 The xQueueSendToBack() API function prototype"></a>


```c
BaseType_t xQueueSendToBack( QueueHandle_t xQueue,
                             const void * pvItemToQueue,
                             TickType_t xTicksToWait );
```
***清单 5.3*** *xQueueSendToBack() API 函数原型*


**xQueueSendToFront() 和 xQueueSendToBack() 函数参数和返回值**

- `xQueue`

数据所在队列的句柄为 sent (written)。
  队列句柄将从对 `xQueueCreate()` 的调用中返回
  或 `xQueueCreateStatic()` 用于创建队列。

- `pvItemToQueue`

指向要复制到队列中的数据的指针。

队列可以容纳的每个项目的大小在队列创建时设置
  创建，以便将许多字节从 `pvItemToQueue` 复制到队列中
  存储区。

- `xTicksToWait`

任务应保留在已阻止状态的最长时间
  状态等待队列上的空间变得可用，如果
  队列已经满了。

`xQueueSendToFront()` 和 `xQueueSendToBack()` 均将回归
  如果 `xTicksToWait` 为零且队列已满，则立即执行。

区块时间以滴答周期为单位指定，因此它的绝对时间
  表示取决于滴答频率。宏 `pdMS_TO_TICKS()`
  可用于将以毫秒为单位的时间转换为时间
  以刻度指定。

将 `xTicksToWait` 设置为 `portMAX_DELAY` 将导致任务等待
  indefinitely (without timing out)，前提是设置了 `INCLUDE_vTaskSuspend`
  为 1 英寸 FreeRTOSConfig.h。

- 返回值

有两种可能的返回值：

- `pdPASS`

当数据成功发送到队列时，返回`pdPASS`。

如果区块时间为 specified (`xTicksToWait` was not zero)，那么它是
    可能调用任务被置于阻塞状态以等待
    在函数返回之前队列中可用的空间，
    但在阻塞时间之前数据已成功写入队列
    已过期。

- `errQUEUE_FULL` (same value as `pdFAIL`)

如果无法将数据写入队列，则返回 `errQUEUE_FULL`
    因为队列已经满了。

如果区块时间为 specified (`xTicksToWait` was not zero) 则
    调用任务将被置于阻塞状态等待
    另一个任务或中断在队列中腾出空间，但指定
    在此之前区块时间已过期。


### 5.3.3 xQueueReceive() API 功能

`xQueueReceive()` receives (reads) 队列中的项目。收到的物品
被从队列中删除。

> *注意：切勿从中断服务程序中调用 `xQueueReceive()`。的
> 中断安全 `xQueueReceiveFromISR()` API 功能描述于
> 第 7 章.*

<a name="list5.4" title="Listing 5.4 The xQueueReceive() API function prototype"></a>

```c
BaseType_t xQueueReceive( QueueHandle_t xQueue,
                          void * const pvBuffer,
                          TickType_t xTicksToWait );
```
***清单 5.4*** *xQueueReceive() API 功能原型*


**xQueueReceive()函数参数及返回值**

- `xQueue`

数据所在队列的句柄为 received
  (read)。队列句柄将从调用中返回
  `xQueueCreate()` 或 `xQueueCreateStatic()` 用于创建队列。

- `pvBuffer`

指向将接收到的数据复制到的内存的指针。

队列保存的每个数据项的大小是在队列
  被创建。 `pvBuffer`指向的内存必须至少大
  足以容纳那么多字节。

- `xTicksToWait`

任务应保留在已阻止状态的最长时间
  状态等待队列上的数据变得可用，如果
  队列已经是空的。

如果 `xTicksToWait` 为零，则 `xQueueReceive()` 将立即返回
  如果队列已经为空。

区块时间以滴答周期为单位指定，因此它的绝对时间
  表示取决于滴答频率。宏 `pdMS_TO_TICKS()`
  可用于将以毫秒为单位的时间转换为时间
  以刻度指定。

将 `xTicksToWait` 设置为 `portMAX_DELAY` 将导致任务等待
  indefinitely (without timing out) 提供 `INCLUDE_vTaskSuspend` 已设置
  为 1 英寸 FreeRTOSConfig.h。

- 返回值

有两种可能的返回值：

- `pdPASS`

当成功从队列中读取数据时，返回 `pdPASS`。

如果区块时间是 specified (`xTicksToWait` was not zero)，那么它是
    可能调用任务被置于阻塞状态以等待
    数据在队列上可用，但数据已成功读取
    在块时间到期之前从队列中取出。</p>

- `errQUEUE_EMPTY` (same value as `pdFAIL`)

如果无法从队列中读取数据，则返回 `errQUEUE_EMPTY`
    因为队列已经空了。

如果区块时间为 specified (`xTicksToWait` was not zero,) 则
    调用任务将被置于阻塞状态等待
    另一个任务或者中断向队列发送数据，但是阻塞时间
    在此之前已过期。


### 5.3.4 uxQueueMessagesWaiting() API 功能

`uxQueueMessagesWaiting()` 查询当前队列中的项目数。

> *注意：切勿从中断服务中调用 `uxQueueMessagesWaiting()`
> 常规。中断安全 `uxQueueMessagesWaitingFromISR()` 应该是
> 用于其位置。*

<a name="list5.5" title="Listing 5.5 The uxQueueMessagesWaiting() API function prototype"></a>

```c
UBaseType_t uxQueueMessagesWaiting( QueueHandle_t xQueue );
```
***清单 5.5*** *uxQueueMessagesWaiting() API 功能原型*


**uxQueueMessagesWaiting()函数参数及返回值**

- `xQueue`

正在查询的队列的句柄。队列句柄将有
  已从对 `xQueueCreate()` 或 `xQueueCreateStatic()` 的调用返回
  用于创建队列。

- 返回值

当前正在查询的队列中的项目数。如果返回零， 
  那么队列是空的。


<a name="example5.1" title="Example 5.1 Blocking when receiving from a queue"></a>
---
***示例 5.1*** *从队列接收时阻塞*

---

本例演示创建队列，向队列发送数据
来自多个任务，并从队列接收数据。队列是
创建用于保存 `int32_t` 类型的数据项。发送到的任务
队列不指定阻塞时间，而从队列接收的任务
队列确实如此。

发送到队列的任务的优先级低于队列中的任务
从队列接收。这意味着队列永远不应该包含
不止一项，因为一旦数据发送到队列
接收任务将解除阻塞，抢占发送任务（因为它有一个
更高优先级），并删除数据，使队列空一次
再次。

该示例创建了清单 5.6 中所示任务的两个实例，一个是
连续将值 100 写入队列，另一个
连续将值 200 写入同一队列。任务参数
用于将这些值传递到每个任务实例中。

<a name="list5.6" title="Listing 5.6 Implementation of the sending task used in Example 5.1"></a>

```c
static void vSenderTask( void *pvParameters )
{

    int32_t lValueToSend;

    BaseType_t xStatus;

    /* Two instances of this task are created so the value that is sent to
       the queue is passed in via the task parameter - this way each instance 
       can use a different value. The queue was created to hold values of type 
       int32_t, so cast the parameter to the required type. */
    lValueToSend = ( int32_t ) pvParameters;

    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {

        /* Send the value to the queue.

           The first parameter is the queue to which data is being sent. The
           queue was created before the scheduler was started, so before this 
           task started to execute.

           The second parameter is the address of the data to be sent, in this 
           case the address of lValueToSend.

           The third parameter is the Block time – the time the task should be 
           kept in the Blocked state to wait for space to become available on 
           the queue should the queue already be full. In this case a block 
           time is not specified because the queue should never contain more 
           than one item, and therefore never be full. */
        xStatus = xQueueSendToBack( xQueue, &lValueToSend, 0 );

        if( xStatus != pdPASS )
        {
            /* The send operation could not complete because the queue was full-
               this must be an error as the queue should never contain more than
               one item! */
            vPrintString( "Could not send to the queue.\r\n" );
        }
    }
}
```
***清单 5.6*** *示例 5.1 中使用的发送任务的实现*


清单 5.7 显示了从以下位置接收数据的任务的实现
队列。接收任务指定出块时间为100
毫秒，然后进入Blocked状态等待数据变为
可用。当任一数据可用时，它会离开阻塞状态
队列中，或者 100 毫秒过去了，数据仍然不可用。
在这个例子中，有两个任务不断地写入队列
所以 100 毫秒的超时永远不会过期。

<a name="list5.7" title="Listing 5.7  Implementation of the receiver task for Example 5.1"></a>

```c
static void vReceiverTask( void *pvParameters )
{
    /* Declare the variable that will hold the values received from the
       queue. */
    int32_t lReceivedValue;
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 100 );

    /* This task is also defined within an infinite loop. */
    for( ;; )
    {
        /* This call should always find the queue empty because this task will
           immediately remove any data that is written to the queue. */
        if( uxQueueMessagesWaiting( xQueue ) != 0 )
        {
            vPrintString( "Queue should have been empty!\r\n" );
        }

        /* Receive data from the queue.

           The first parameter is the queue from which data is to be received.
           The queue is created before the scheduler is started, and therefore
           before this task runs for the first time.

           The second parameter is the buffer into which the received data will
           be placed. In this case the buffer is simply the address of a 
           variable that has the required size to hold the received data.

           The last parameter is the block time – the maximum amount of time 
           that the task will remain in the Blocked state to wait for data to 
           be available should the queue already be empty. */
        xStatus = xQueueReceive( xQueue, &lReceivedValue, xTicksToWait );

        if( xStatus == pdPASS )
        {
            /* Data was successfully received from the queue, print out the
               received value. */
            vPrintStringAndNumber( "Received = ", lReceivedValue );
        }
        else
        {
            /* Data was not received from the queue even after waiting for 
               100ms. This must be an error as the sending tasks are free 
               running and will be continuously writing to the queue. */
            vPrintString( "Could not receive from the queue.\r\n" );
        }
    }
}
```
***清单 5.7*** *示例 5.1 的接收器任务的实现*


清单 5.8 包含 `main()` 函数的定义。这简直就是
在启动调度程序之前创建队列和三个任务。的
创建队列最多可容纳五个 `int32_t` 值，即使
相对任务优先级意味着队列永远不会容纳超过
一次一项。

<a name="list5.8" title="Listing 5.8 The implementation of main() in Example 5.1"></a>

```c
/* Declare a variable of type QueueHandle_t. This is used to store the
   handle to the queue that is accessed by all three tasks. */
QueueHandle_t xQueue;

int main( void )
{
    /* The queue is created to hold a maximum of 5 values, each of which is
       large enough to hold a variable of type int32_t. */
    xQueue = xQueueCreate( 5, sizeof( int32_t ) );

    if( xQueue != NULL )
    {
        /* Create two instances of the task that will send to the queue. The
           task parameter is used to pass the value that the task will write 
           to the queue, so one task will continuously write 100 to the queue 
           while the other task will continuously write 200 to the queue. Both
           tasks are created at priority 1. */
        xTaskCreate( vSenderTask, "Sender1", 1000, ( void * ) 100, 1, NULL );
        xTaskCreate( vSenderTask, "Sender2", 1000, ( void * ) 200, 1, NULL );

        /* Create the task that will read from the queue. The task is created
           with priority 2, so above the priority of the sender tasks. */
        xTaskCreate( vReceiverTask, "Receiver", 1000, NULL, 2, NULL );

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    }
    else
    {
        /* The queue could not be created. */
    }

    /* If all is well then main() will never reach here as the scheduler will
       now be running the tasks. If main() does reach here then it is likely
       that there was insufficient FreeRTOS heap memory available for the idle 
       task to be created. Chapter 3 provides more information on heap memory 
       management. */
    for( ;; );
}
```
***清单 5.8*** *示例 5.1 中 main() 的实现*

图 5.2 显示了示例 5.1 生成的输出。

<a name="fig5.2" title="Figure 5.2 The output produced when Example 5.1 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image32.jpg)   
***图 5.2*** *执行例 5.1 时产生的输出*

* * *


图 5.3 演示了执行顺序。

<a name="fig5.3" title="Figure 5.3 The sequence of execution produced by Example 5.1"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image33.png)   
***图 5.3*** *示例 5.1 产生的执行顺序*

* * *


## 5.4 从多个源接收数据

在 FreeRTOS 设计中，任务从多个接收数据是很常见的。
比一个来源。接收任务需要知道数据来自哪里
来确定如何处理它。易于实现的设计模式
使用单个队列来传输包含两个的结构
数据值和数据来源，如图5.4所示。

<a name="fig5.4" title="Figure 5.4 An example scenario where structures are sent on a queue"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image34.png)   
***图 5.4*** *在队列上发送结构的示例场景*

* * *

参见图5.4：

- 创建的队列包含 `Data_t` 类型的结构。结构
  允许数据值和枚举类型指示什么
  data 表示以一条消息发送到队列。

- 中央控制器任务执行主要系统功能。这个
  必须对输入和所传达的系统状态的变化做出反应
  队列中的它。

- CAN 总线任务用于封装 CAN 总线接口
  功能。当 CAN 总线任务已接收并解码
  消息，它将已经解码的消息发送到控制器任务
  采用 `Data_t` 结构。 `eDataID` 已转会员
  结构告诉控制器任务数据是什么。在这种情况下 
  此处所示，它是电机速度值。 `lDataValue` 成员
  传输的结构告诉控制器任务实际的电机
  速度值。

- 一个Human Machine Interface (HMI)任务用于封装所有
  HMI 功能。机器操作员可能可以输入命令
  并以多种方式查询值，这些方式必须被检测和
  在 HMI 任务中解释。当输入新命令时，
  HMI 任务将命令发送到 `Data_t` 中的控制器任务
  结构。转移结构的 `eDataID` 成员告诉
  控制器任务数据是什么。在这里显示的情况下，它是一个新的
  设定点值。转移结构的`lDataValue`成员
  告诉控制器任务实际的设定点值。

Chapter (RB-TBD) 展示了如何扩展此设计模式，以便
控制器任务可以直接回复对结构进行排队的任务。


<a name="example5.2" title="Example 5.2 Blocking when sending to a queue, and sending structures on a queue"></a>
---
***示例 5.2*** *发送到队列以及发送队列上的结构时发生阻塞*

---

例 5.2 与例 5.1 类似，但任务优先级相反，
因此接收任务的优先级低于发送任务。另外，
创建的队列保存结构而不是整数。

清单 5.9 显示了示例 5.2 使用的结构的定义。

<a name="list5.9" title="Listing 5.9 The definition of the structure that is to be passed on a queue, plus the declaration of two variables for use by the example"></a>

```c
/* Define an enumerated type used to identify the source of the data. */
typedef enum
{
    eSender1,
    eSender2
} DataSource_t;

/* Define the structure type that will be passed on the queue. */
typedef struct
{
    uint8_t ucValue;
    DataSource_t eDataSource;
} Data_t;

/* Declare two variables of type Data_t that will be passed on the queue. */
static const Data_t xStructsToSend[ 2 ] =
{
    { 100, eSender1 }, /* Used by Sender1. */
    { 200, eSender2 }  /* Used by Sender2. */
};
```
***清单 5.9*** *要在队列上传递的结构的定义，以及示例使用的两个变量的声明*

在例5.1中，接收任务具有最高优先级，因此队列
绝不会包含超过一项的项目。发生这种情况是因为接收任务
一旦数据放入队列，就抢占发送任务。
在例5.2中，发送任务具有较高的优先级，因此队列
一般都会满的。这是因为，一旦接收任务
从队列中删除一个项目，它被发送者之一抢占
然后立即重新填充队列的任务。接下来就是发送任务了
重新进入阻塞状态以等待空间变得可用
再次排队。

清单5.10显示了发送任务的实现。发送的
任务指定阻塞时间为100毫秒，因此进入
阻塞状态每次队列等待空间变得可用
变满。当任一空间可用时，它会离开阻塞状态
在队列中，或者 100 毫秒过去而没有空间
可用。在这个例子中，接收任务不断地腾出空间
在队列中，因此 100 毫秒超时永远不会过期。

<a name="list5.10" title="Listing 5.10 The implementation of the sending task for Example 5.2"></a>

```c
static void vSenderTask( void *pvParameters )
{
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 100 );

    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {
        /* Send to the queue.

           The second parameter is the address of the structure being sent. The
           address is passed in as the task parameter so pvParameters is used
           directly.

           The third parameter is the Block time - the time the task should be 
           kept in the Blocked state to wait for space to become available on 
           the queue if the queue is already full. A block time is specified 
           because the sending tasks have a higher priority than the receiving 
           task so the queue is expected to become full. The receiving task 
           will remove items from the queue when both sending tasks are in the 
           Blocked state. */
        xStatus = xQueueSendToBack( xQueue, pvParameters, xTicksToWait );

        if( xStatus != pdPASS )
        {
            /* The send operation could not complete, even after waiting for 
               100ms. This must be an error as the receiving task should make 
               space in the queue as soon as both sending tasks are in the 
               Blocked state. */
            vPrintString( "Could not send to the queue.\r\n" );
        }
    }
}
```
***清单5.10*** *例5.2发送任务的实现*


接收任务的优先级最低，因此只有当两者都满足时才运行
发送任务处于阻塞状态。发送任务只进入
当队列已满时会进入阻塞状态，因此接收任务只会
当队列已满时执行。因此，它总是期望
即使没有指定块时间也可以接收数据。

清单5.11显示了接收任务的实现。

<a name="list5.11" title="Listing 5.11 The definition of the receiving task for Example 5.2"></a>

```c
static void vReceiverTask( void *pvParameters )
{
    /* Declare the structure that will hold the values received from the
       queue. */
    Data_t xReceivedStructure;
    BaseType_t xStatus;

    /* This task is also defined within an infinite loop. */
    for( ;; )
    {
        /* Because it has the lowest priority this task will only run when the
           sending tasks are in the Blocked state. The sending tasks will only
           enter the Blocked state when the queue is full so this task always 
           expects the number of items in the queue to be equal to the queue 
           length, which is 3 in this case. */
        if( uxQueueMessagesWaiting( xQueue ) != 3 )
        {
            vPrintString( "Queue should have been full!\r\n" );
        }

        /* Receive from the queue.

           The second parameter is the buffer into which the received data will
           be placed. In this case the buffer is simply the address of a 
           variable that has the required size to hold the received structure.

           The last parameter is the block time - the maximum amount of time 
           that the task will remain in the Blocked state to wait for data to 
           be available if the queue is already empty. In this case a block 
           time is not necessary because this task will only run when the 
           queue is full. */
        xStatus = xQueueReceive( xQueue, &xReceivedStructure, 0 );

        if( xStatus == pdPASS )
        {
            /* Data was successfully received from the queue, print out the
               received value and the source of the value. */
            if( xReceivedStructure.eDataSource == eSender1 )
            {
                vPrintStringAndNumber( "From Sender 1 = ", 
                                       xReceivedStructure.ucValue );
            }
            else
            {
                vPrintStringAndNumber( "From Sender 2 = ", 
                                       xReceivedStructure.ucValue );
            }
        }
        else
        {
            /* Nothing was received from the queue. This must be an error as 
               this task should only run when the queue is full. */
            vPrintString( "Could not receive from the queue.\r\n" );
        }
    }
}
```
***清单 5.11*** *示例 5.2 接收任务的定义*

`main()` 与上一个示例相比仅略有变化。队列是
创建用于保存三个 `Data_t` 结构，以及该结构的优先级
发送和接收任务相反。清单 5.12 显示了
`main()` 的实施。

<a name="list5.12" title="Listing 5.12 The implementation of main() for Example 5.2"></a>

```c
int main( void )
{
    /* The queue is created to hold a maximum of 3 structures of type Data_t. */
    xQueue = xQueueCreate( 3, sizeof( Data_t ) );

    if( xQueue != NULL )
    {
        /* Create two instances of the task that will write to the queue. The
           parameter is used to pass the structure that the task will write to
           the queue, so one task will continuously send xStructsToSend[ 0 ]
           to the queue while the other task will continuously send 
           xStructsToSend[ 1 ]. Both tasks are created at priority 2, which is
           above the priority of the receiver. */
        xTaskCreate( vSenderTask, "Sender1", 1000, &( xStructsToSend[ 0 ] ),
                     2, NULL );
        xTaskCreate( vSenderTask, "Sender2", 1000, &( xStructsToSend[ 1 ] ),
                     2, NULL );

        /* Create the task that will read from the queue. The task is created
           with priority 1, so below the priority of the sender tasks. */
        xTaskCreate( vReceiverTask, "Receiver", 1000, NULL, 1, NULL );

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    }
    else
    {
        /* The queue could not be created. */
    }

    /* If all is well then main() will never reach here as the scheduler will
       now be running the tasks. If main() does reach here then it is likely
       that there was insufficient heap memory available for the idle task to 
       be created. Chapter 3 provides more information on heap memory 
       management. */
    for( ;; );
}
```
***清单 5.12*** *示例 5.2 的 main() 的实现*

图 5.5 显示了示例 5.2 生成的输出。

<a name="fig5.5" title="Figure 5.5 The output produced by Example 5.2"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image35.jpg)   
***图 5.5*** *示例 5.2 产生的输出*

* * *

图 5.6 演示了执行顺序
发送任务的优先级高于
接收任务。下面给出的是图 5.6 的进一步解释，并且
关于为什么前四个消息源自同一任务的描述。

<a name="fig5.6" title="Figure 5.6 The sequence of execution produced by Example 5.2"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image36.png)   
***图 5.6*** *示例 5.2 产生的执行顺序*

* * *

**图 5.6 的要点**

- t1

任务发送者 1 执行并向队列发送 3 个数据项。

- t2

队列已满，因此发送方 1 进入阻塞状态等待
  下一次发送完成。任务发送者 2 现在具有最高优先级
  任务可以运行，因此进入Running状态。

- t3

任务发送者2发现队列已经满了，因此进入Blocked状态
  状态等待其第一次发送完成。任务接收者现在是
  可以运行的优先级最高的任务，因此进入运行状态。

- t4

两个任务的优先级高于接收任务的优先级
  优先级正在等待队列上有可用空间，
  导致任务接收者一旦被删除就被抢占
  队列中的一项。任务发送者 1 和发送者 2 具有相同的任务
  优先级，因此调度程序选择一直在等待的任务
  最长的任务将进入运行状态 - 在这种情况下
  是任务发送者 1。

- t5

任务发送者 1 向队列发送另一个数据项。有
  队列中只有一个空间，因此任务 Sender 1 进入 Blocked 状态
  等待下一次发送完成。任务接收者又是
  可以运行的最高优先级任务，因此它进入运行状态。

任务发送者 1 现在已向队列发送了四个项目，任务发送者 2
  仍在等待将其第一个项目发送到队列。

-t6

两个任务的优先级高于接收任务的优先级
  优先级正在等待队列上的空间变得可用，因此任务
  接收器一旦从接收器中取出一项，就会被抢占
  队列。这次发送者 2 比发送者 1 等待的时间更长，所以
  发送方2进入运行状态。

- t7

任务发送者 2 向队列发送一个数据项。只有一个
  队列中有空间，因此发送方 2 进入阻塞状态以等待其
  接下来发送完成。发送者 1 和发送者 2 都在等待任务
  队列上有可用空间，因此任务接收者是唯一的
  可以进入Running状态的任务。


## 5.5 处理大型或可变大小的数据

### 5.5.1 排队指针

如果队列中存储的数据量很大，那么
最好使用队列来传输指向数据的指针，而不是
而不是将数据本身逐字节复制到队列中或从队列中复制出来。
传输指针在处理时间和
创建队列所需的 RAM 量。不过排队的时候
指针，必须格外小心以确保：

- 所指向的 RAM 的所有者已明确定义。

当通过指针在任务之间共享内存时，必须
  确保两个任务不会同时修改内存内容，或者
  采取任何其他可能导致内存内容无效的操作
  或不一致。理想情况下，只允许发送任务
  在指针被发送到队列之前访问内存，并且只有
  接收任务应该被允许访问内存后
  已从队列接收到指针。

- 指向的 RAM 仍然有效。

如果指向的内存是动态分配的或获得的
  从预先分配的缓冲区池中，那么只有一个任务应该是
  负责释放内存。任何任务都不应尝试访问
  释放后的内存。

永远不要使用指针来访问已分配的数据
  一个任务栈。堆栈帧结束后数据将无效
  改变了。

清单 5.13、5.14 和 5.15 举例说明了如何
使用队列将指向缓冲区的指针从一个任务发送到另一个任务：

- 清单 5.13 创建了一个最多可以容纳 5 个指针的队列。

- 清单 5.14 分配一个缓冲区，将一个字符串写入缓冲区，然后
  将指向缓冲区的指针发送到队列。

- 清单 5.15 从队列接收一个指向缓冲区的指针，然后
  打印缓冲区中包含的字符串。

<a name="list5.13" title="Listing 5.13 Creating a queue that holds pointers"></a>

```c
/* Declare a variable of type QueueHandle_t to hold the handle of the
   queue being created. */
QueueHandle_t xPointerQueue;

/* Create a queue that can hold a maximum of 5 pointers, in this case
   character pointers. */
xPointerQueue = xQueueCreate( 5, sizeof( char * ) );
```
***清单 5.13*** *创建一个保存指针的队列*

<a name="list5.14" title="Listing 5.14 Using a queue to send a pointer to a buffer"></a>

```c
/* A task that obtains a buffer, writes a string to the buffer, then
   sends the address of the buffer to the queue created in Listing 5.13. */
void vStringSendingTask( void *pvParameters )
{
    char *pcStringToSend;
    const size_t xMaxStringLength = 50;
    BaseType_t xStringNumber = 0;

    for( ;; )
    {
        /* Obtain a buffer that is at least xMaxStringLength characters big.
           The implementation of prvGetBuffer() is not shown – it might obtain
           the buffer from a pool of pre-allocated buffers, or just allocate 
           the buffer dynamically. */
        pcStringToSend = ( char * ) prvGetBuffer( xMaxStringLength );

        /* Write a string into the buffer. */
        snprintf( pcStringToSend, xMaxStringLength, "String number %d\r\n",
                  xStringNumber );

        /* Increment the counter so the string is different on each iteration
           of this task. */
        xStringNumber++;

        /* Send the address of the buffer to the queue that was created in
           Listing 5.13. The address of the buffer is stored in the 
           pcStringToSend variable.*/
        xQueueSend( xPointerQueue,   /* The handle of the queue. */
                    &pcStringToSend, /* The address of the pointer that points
                                        to the buffer. */
                    portMAX_DELAY );
    }
}
```
***清单 5.14*** *使用队列发送指向缓冲区的指针*

<a name="list5.15" title="Listing 5.15 Using a queue to receive a pointer to a buffer"></a>

```c
/* A task that receives the address of a buffer from the queue created
   in Listing 5.13, and written to in Listing 5.14. The buffer contains a
   string, which is printed out. */

void vStringReceivingTask( void *pvParameters )
{
    char *pcReceivedString;

    for( ;; )
    {
        /* Receive the address of a buffer. */
        xQueueReceive( xPointerQueue,     /* The handle of the queue. */
                       &pcReceivedString, /* Store the buffer's address in 
                                             pcReceivedString. */
                       portMAX_DELAY );

        /* The buffer holds a string, print it out. */
        vPrintString( pcReceivedString );

        /* The buffer is not required any more - release it so it can be freed,
           or re-used. */
        prvReleaseBuffer( pcReceivedString );
    }
}
```
***清单 5.15*** *使用队列接收指向缓冲区的指针*

### 5.5.2 使用队列发送不同类型和长度的数据[^9]

[^9]：FreeRTOS 消息缓冲区是一种更轻量级的替代方案
保存可变长度数据的队列。

本书前面的章节展示了两种强大的设计
图案；将结构发送到队列，并将指针发送到
队列。结合这些技术允许任务使用单个队列来
从任何数据源接收任何数据类型。实施
FreeRTOS+TCP TCP/IP 堆栈提供了一个实际示例来说明这是如何进行的
实现了。

TCP/IP 堆栈在其自己的任务中运行，必须处理来自
许多不同的来源。不同的事件类型关联
不同类型和长度的数据。 `IPStackEvent_t`结构描述
TCP/IP 任务之外发生的所有事件，并发送到
队列中的 TCP/IP 任务。清单 5.16 显示了 `IPStackEvent_t` 结构。
`IPStackEvent_t` 结构体的 `pvData` 成员是一个指针，可以
用于直接保存值，或指向缓冲区。

<a name="list5.16" title="Listing 5.16 The structure used to send events to the TCP/IP stack task in FreeRTOS+TCP"></a>

```c
/* A subset of the enumerated types used in the TCP/IP stack to
   identify events. */
typedef enum
{
    eNetworkDownEvent = 0, /* The network interface has been lost, or needs
                              (re)connecting. */
    eNetworkRxEvent,       /* A packet has been received from the network. */
    eTCPAcceptEvent,       /* FreeRTOS_accept() called to accept or wait for a
                              new client. */

/* Other event types appear here but are not shown in this listing. */

} eIPEvent_t;

/* The structure that describes events, and is sent on a queue to the
   TCP/IP task. */
typedef struct IP_TASK_COMMANDS
{
    /* An enumerated type that identifies the event. See the eIPEvent_t
       definition above. */
    eIPEvent_t eEventType;

    /* A generic pointer that can hold a value, or point to a buffer. */
    void *pvData;

} IPStackEvent_t;
```
***清单 5.16*** *用于将事件发送到 FreeRTOS 中的 TCP/IP 堆栈任务的结构+TCP*

示例 TCP/IP 事件及其相关数据包括：

- `eNetworkRxEvent`：从网络收到数据包。

网络接口将数据接收事件发送到 TCP/IP 任务
  使用 `IPStackEvent_t` 类型的结构。该结构的`eEventType`
  成员设置为 `eNetworkRxEvent`，结构体的 `pvData` 成员为
  用于指向包含接收到的数据的缓冲区。上市
  图59示出了伪代码示例。

  <a name="list5.17" title="Listing 5.17 Pseudo code showing how an IPStackEvent_t structure is used to send data received from the network to the TCP/IP task"></a>

  ```c
  void vSendRxDataToTheTCPTask( NetworkBufferDescriptor_t *pxRxedData )
  {
      IPStackEvent_t xEventStruct;
  
      /* Complete the IPStackEvent_t structure. The received data is stored in
         pxRxedData. */
      xEventStruct.eEventType = eNetworkRxEvent;
      xEventStruct.pvData = ( void * ) pxRxedData;
  
      /* Send the IPStackEvent_t structure to the TCP/IP task. */
      xSendEventStructToIPTask( &xEventStruct );
  }
  ```
***清单 5.17*** *伪代码显示如何使用 IPStackEvent_t 结构将从网络接收的数据发送到 TCP/IP 任务*

- `eTCPAcceptEvent`：套接字用于接受或等待连接
  来自客户。

调用 `FreeRTOS_accept()` 的任务将接受事件发送到
  TCP/IP 任务使用 `IPStackEvent_t` 类型的结构。该结构的
  `eEventType` 成员设置为 `eTCPAcceptEvent`，并且该结构的
  `pvData` 成员设置为正在接受的套接字的句柄
  连接。清单 5.18 显示了一个伪代码示例。

  <a name="list5.18" title="Listing 5.18 Pseudo code showing how an IPStackEvent_t structure is used to send the handle of a socket that is accepting a connection to the TCP/IP task"></a>

  ```c
  void vSendAcceptRequestToTheTCPTask( Socket_t xSocket )
  {
      IPStackEvent_t xEventStruct;

      /* Complete the IPStackEvent_t structure. */
      xEventStruct.eEventType = eTCPAcceptEvent;
      xEventStruct.pvData = ( void * ) xSocket;

      /* Send the IPStackEvent_t structure to the TCP/IP task. */
      xSendEventStructToIPTask( &xEventStruct );
  }
  ```
***清单 5.18*** *伪代码显示如何使用 IPStackEvent_t 结构将接受连接的套接字句柄发送到 TCP/IP 任务*

- `eNetworkDownEvent`：网络需要连接或重新连接。

网络接口将网络关闭事件发送到 TCP/IP 任务
  使用 `IPStackEvent_t` 类型的结构。该结构的`eEventType`
  成员设置为 `eNetworkDownEvent`。网络宕机事件不是
  与任何数据关联，因此该结构的 `pvData` 成员不是
  使用过。清单 5.19 显示了一个伪代码示例。

  <a name="list5.19" title="Listing 5.19 Pseudo code showing how an IPStackEvent_t structure is used to send a network down event to the TCP/IP task"></a>

  ```c
  void vSendNetworkDownEventToTheTCPTask( Socket_t xSocket )
  {
      IPStackEvent_t xEventStruct;

      /* Complete the IPStackEvent_t structure. */
      xEventStruct.eEventType = eNetworkDownEvent;

      xEventStruct.pvData = NULL; /* Not used, but set to NULL for
                                     completeness. */

      /* Send the IPStackEvent_t structure to the TCP/IP task. */
      xSendEventStructToIPTask( &xEventStruct );
  }
  ```
***清单 5.19*** *伪代码显示如何使用 IPStackEvent_t 结构将网络关闭事件发送到 TCP/IP 任务*

清单 5.20 显示了接收和处理这些事件的代码
  在 TCP/IP 任务内。可以看出，`eEventType`成员
  从队列接收的 `IPStackEvent_t` 结构用于确定
  如何解释 `pvData` 成员。

  <a name="list5.20" title="Listing 5.20 Pseudo code showing how an IPStackEvent_t structure is received and processed"></a>

  ```c
  IPStackEvent_t xReceivedEvent;

  /* Block on the network event queue until either an event is received, or 
     xNextIPSleep ticks pass without an event being received. eEventType is 
     set to eNoEvent in case the call to xQueueReceive() returns because it 
     timed out, rather than because an event was received. */
  xReceivedEvent.eEventType = eNoEvent;
  xQueueReceive( xNetworkEventQueue, &xReceivedEvent, xNextIPSleep );

  /* Which event was received, if any? */
  switch( xReceivedEvent.eEventType )
  {
      case eNetworkDownEvent :
           /* Attempt to (re)establish a connection. This event is not 
              associated with any data. */
           prvProcessNetworkDownEvent();
           break;

      case eNetworkRxEvent:
           /* The network interface has received a new packet. A pointer to the
              received data is stored in the pvData member of the received 
              IPStackEvent_t structure. Process the received data. */
           prvHandleEthernetPacket( ( NetworkBufferDescriptor_t * )
                                    ( xReceivedEvent.pvData ) );
           break;

      case eTCPAcceptEvent:
           /* The FreeRTOS_accept() API function was called. The handle of the
              socket that is accepting a connection is stored in the pvData 
              member of the received IPStackEvent_t structure. */
           xSocket = ( FreeRTOS_Socket_t * ) ( xReceivedEvent.pvData );
           xTCPCheckNewClient( xSocket );
           break;

      /* Other event types are processed in the same way, but are not shown
         here. */

  }
  ```
***清单 5.20*** *显示如何接收和处理 IPStackEvent_t 结构的伪代码*

## 5.6 从多个队列接收

### 5.6.1 队列集

通常应用程序设计需要单个任务来接收数据
不同大小、不同含义的数据、不同来源的数据
来源。上一节演示了如何以简洁的方式做到这一点
使用接收结构的单个队列的有效方法。然而，
有时，应用程序的设计者会遇到以下限制：
限制他们的设计选择，需要使用单独的队列
对于某些数据源。例如，集成第三方代码
设计中可能会假设存在专用队列。在这样的
在这种情况下可以使用“队列集”。

队列集允许任务从多个队列接收数据，而无需
该任务依次轮询每个队列以确定哪个队列（如果有）包含
数据。

使用队列集从多个源接收数据的设计是
与达到相同效果的设计相比，不太整洁，效率也较低
使用接收结构的单个队列的功能。为此
由于设计限制，建议仅使用队列集
使它们的使用绝对必要。

以下部分描述了如何使用以下设置的队列：

- 创建队列集。

- 将队列添加到集合中。

信号量也可以添加到队列集中。信号量将在本书后面进行描述。

- 从队列集中读取以确定该集中的哪些队列包含数据。

当作为集合成员的队列接收数据时，该队列的句柄
  接收队列发送到队列集合，当有消息时返回
  任务调用从队列集中读取的函数。因此，如果一个
  队列句柄是从队列集合中返回的，然后引用该队列
  已知句柄包含数据，然后任务可以读取
  直接从该队列中。

> *注意：如果队列是队列集的成员，那么您必须从中读取
  > 每次从队列集中接收到队列句柄时，您都会
  > 在从队列接收到句柄之前不得从队列中读取
  > 队列设置。*

通过设置 `configUSE_QUEUE_SETS` 启用队列设置功能
FreeRTOSConfig.h 中的编译时配置常量为 1。


### 5.6.2 xQueueCreateSet() API 功能

队列集必须在使用之前显式创建。在
在撰写本文时，`xQueueCreateSetStatic()` 尚未实现。
然而队列集本身就是队列，因此可以创建一个
通过使用特制的调用来设置使用预分配的内存
`xQueueCreateStatic()`。

队列集由句柄引用，句柄是类型的变量
`QueueSetHandle_t`。 `xQueueCreateSet()` API 函数创建队列集
并返回引用创建的队列集的 `QueueSetHandle_t`。

<a name="list5.21" title="Listing 5.21 The xQueueCreateSet() API function prototype"></a>

```c
QueueSetHandle_t xQueueCreateSet( const UBaseType_t uxEventQueueLength);
```
***清单 5.21*** *xQueueCreateSet() API 功能原型*


**xQueueCreateSet()参数及返回值**

- `uxEventQueueLength`

当作为队列集成员的队列接收到数据时，
  接收队列的句柄被发送到队列集。
  `uxEventQueueLength` 定义队列句柄的最大数量
  正在创建的集合可以在任何一个时间保存。

仅当队列中的队列存在时，队列句柄才会发送到队列集。
  设置接收数据。如果队列已满，则无法接收数据，所以不
  如果集合中的所有队列都可以，则可以将队列句柄发送到队列集合
  已满。因此，队列设置的最大项目数
  一次必须持有的是队列中每个队列的长度之和
  设置。

举个例子，如果集合中有三个空队列，并且每个队列
  队列的长度为 5，那么该集合中的队列总共可以
  收到十五个 items (three queues multiplied by five items each)
  在集合中的所有队列都已满之前。在那个例子中
  `uxEventQueueLength` 必须设置为 15 以保证队列集可以
  接收发送给它的每一个项目。

信号量也可以添加到队列集中。信号量被覆盖
  本书稍后会介绍。为了计算必要的
  `uxEventQueueLength`，二进制信号量的长度为一，长度
  互斥锁的长度为 1，计数信号量的长度由下式给出
  信号量的最大计数值。

作为另一个示例，如果队列集将包含一个具有
  长度为三，二进制semaphore (which has a length of one)，
  `uxEventQueueLength` 必须设置为 four (three plus one)。

- 返回值

如果返回 NULL，则无法创建队列集，因为
  FreeRTOS 没有足够的堆内存来分配
  队列集合数据结构和存储区域。第 3 章提供了更多内容
  有关 FreeRTOS 堆的信息。

如果返回非 NULL 值，则队列集已创建
  成功，返回值是创建的队列集的句柄。


### 5.6.3 xQueueAddToSet() API 功能

`xQueueAddToSet()` 将队列或信号量添加到队列集中。信号量
本书稍后将对此进行描述。

<a name="list5.22" title="Listing 5.22 The xQueueAddToSet() API function prototype"></a>

```c
BaseType_t xQueueAddToSet( QueueSetMemberHandle_t xQueueOrSemaphore,
                           QueueSetHandle_t xQueueSet );
```
***清单 5.22*** *xQueueAddToSet() API 功能原型*

**xQueueAddToSet()参数及返回值**

- `xQueueOrSemaphore`

正在添加到队列集中的队列或信号量的句柄。

队列句柄和信号量句柄都可以转换为 `QueueSetMemberHandle_t` 类型。

- `xQueueSet`

队列或信号量要添加到的队列集的句柄。

- 返回值

有两种可能的返回值：

1.`pdPASS`

这表明队列集创建成功。

1.`pdFAIL`

这表明队列或信号量无法添加到队列集中。

队列和二进制信号量只能在它们被添加到集合中时
  空的。计数信号量只能在其计数达到时才添加到集合中
  为零。队列和信号量一次只能是一组的成员。


### 5.6.4 xQueueSelectFromSet() API 功能

`xQueueSelectFromSet()` 从队列集中读取队列句柄。

当作为集合成员的队列或信号量接收数据时，
接收队列或信号量的句柄被发送到队列集，并且
当任务调用 `xQueueSelectFromSet()` 时返回。如果一个句柄是
从对 `xQueueSelectFromSet()` 的调用返回，然后队列或
已知句柄引用的信号量包含数据并且
然后调用任务必须直接从队列或信号量中读取。

> *注意：不要从队列或信号量中读取数据
> 除非首先返回队列或信号量的句柄，否则设置
> 从呼叫 `xQueueSelectFromSet()`。仅读取队列中的一项或
> semaphore 每次返回队列句柄或信号量句柄
> 来自对 `xQueueSelectFromSet()` 的调用。*

<a name="list5.23" title="Listing 5.23 The xQueueSelectFromSet() API function prototype"></a>

```c
QueueSetMemberHandle_t xQueueSelectFromSet( QueueSetHandle_t xQueueSet,
                                            const TickType_t xTicksToWait );
```
***清单 5.23*** *xQueueSelectFromSet() API 功能原型*

**xQueueSelectFromSet()参数及返回值**

- `xQueueSet`

队列句柄或信号量所在的队列集的句柄
  手柄为 received (read)。队列集句柄将是
  从用于创建队列的 `xQueueCreateSet()` 调用返回
  设置。

- `xTicksToWait`

调用任务应在任务中保留的最长时间
  阻塞状态等待接收来自队列或信号量句柄
  队列集合，如果集合中的所有队列和信号量都为空。

如果 `xTicksToWait` 为零，则 `xQueueSelectFromSet()` 将返回
  如果集合中的所有队列和信号量都为空，则立即执行。

区块时间以滴答周期为单位指定，因此它的绝对时间
  表示取决于滴答频率。宏 `pdMS_TO_TICKS()`
  可用于将以毫秒为单位的时间转换为时间
  以刻度指定。

将 `xTicksToWait` 设置为 `portMAX_DELAY` 将导致任务等待
  indefinitely (without timing out) 提供 `INCLUDE_vTaskSuspend` 已设置
  为 1 英寸 FreeRTOSConfig.h。

- 返回值

不是 NULL 的返回值将是队列的句柄或
  已知包含数据的信号量。如果块时间为 specified
  (`xTicksToWait` was not zero)，则调用任务可能是
  置于阻塞状态以等待数据可用
  集合中的队列或信号量，但已成功读取句柄
  阻塞时间到期之前设置的队列。句柄作为
  `QueueSetMemberHandle_t` 型，可铸造为 `QueueHandle_t`
  型或 `SemaphoreHandle_t` 型。

如果返回值为 NULL，则无法从
  队列集。如果区块时间为 specified (`xTicksToWait` was not zero)
  然后调用任务被置于阻塞状态
  等待另一个任务或中断将数据发送到队列或信号量
  在集合中，但块时间在此之前就已过期。


<a name="example5.3" title="Example 5.3 Using a Queue Set"></a>
---
***示例 5.3*** *使用队列集</i></h3>

---

此示例创建两个发送任务和一个接收任务。的
发送任务将数据发送到两个单独队列上的接收任务，
每个任务一个队列。将两个队列添加到队列集中，并且
接收任务从队列集中读取以确定哪一个
两个队列包含数据。

任务、队列和队列集均在 `main()` 中创建 - 请参阅
其实现见清单 5.24。

<a name="list5.24" title="Listing 5.24  Implementation of main() for Example 5.3"></a>

```c
/* Declare two variables of type QueueHandle_t. Both queues are added
   to the same queue set. */
static QueueHandle_t xQueue1 = NULL, xQueue2 = NULL;

/* Declare a variable of type QueueSetHandle_t. This is the queue set
   to which the two queues are added. */
static QueueSetHandle_t xQueueSet = NULL;

int main( void )
{
    /* Create the two queues, both of which send character pointers. The 
       priority of the receiving task is above the priority of the sending 
       tasks, so the queues will never have more than one item in them at 
       any one time*/
    xQueue1 = xQueueCreate( 1, sizeof( char * ) );
    xQueue2 = xQueueCreate( 1, sizeof( char * ) );

    /* Create the queue set. Two queues will be added to the set, each of
       which can contain 1 item, so the maximum number of queue handles the 
       queue set will ever have to hold at one time is 2 (2 queues multiplied 
       by 1 item per queue). */
    xQueueSet = xQueueCreateSet( 1 * 2 );

    /* Add the two queues to the set. */
    xQueueAddToSet( xQueue1, xQueueSet );
    xQueueAddToSet( xQueue2, xQueueSet );

    /* Create the tasks that send to the queues. */
    xTaskCreate( vSenderTask1, "Sender1", 1000, NULL, 1, NULL );
    xTaskCreate( vSenderTask2, "Sender2", 1000, NULL, 1, NULL );

    /* Create the task that reads from the queue set to determine which of
       the two queues contain data. */
    xTaskCreate( vReceiverTask, "Receiver", 1000, NULL, 2, NULL );

    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();

    /* As normal, vTaskStartScheduler() should not return, so the following
       lines will never execute. */
    for( ;; );
    return 0;
}
```
***清单 5.24*** *示例 5.3 的 main() 的实现*

第一个发送任务使用`xQueue1`发送一个字符指针到
每100毫秒接收一次任务。第二个发送任务使用
`xQueue2` 每200发送一个字符指针给接收任务
毫秒。字符指针指向一个字符串，该字符串标识
发送任务。清单 5.25 显示了这两个任务的实现。

<a name="list5.25" title="Listing 5.25 The sending tasks used in Example 5.3"></a>

```c
void vSenderTask1( void *pvParameters )
{
    const TickType_t xBlockTime = pdMS_TO_TICKS( 100 );
    const char * const pcMessage = "Message from vSenderTask1\r\n";

    /* As per most tasks, this task is implemented within an infinite loop. */

    for( ;; )
    {

        /* Block for 100ms. */
        vTaskDelay( xBlockTime );

        /* Send this task's string to xQueue1. It is not necessary to use a
           block time, even though the queue can only hold one item. This is 
           because the priority of the task that reads from the queue is 
           higher than the priority of this task; as soon as this task writes 
           to the queue it will be pre-empted by the task that reads from the 
           queue, so the queue will already be empty again by the time the 
           call to xQueueSend() returns. The block time is set to 0. */
        xQueueSend( xQueue1, &pcMessage, 0 );
    }
}

/*-----------------------------------------------------------*/

void vSenderTask2( void *pvParameters )
{
    const TickType_t xBlockTime = pdMS_TO_TICKS( 200 );
    const char * const pcMessage = "Message from vSenderTask2\r\n";

    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {
        /* Block for 200ms. */
        vTaskDelay( xBlockTime );

        /* Send this task's string to xQueue2. It is not necessary to use a
           block time, even though the queue can only hold one item. This is 
           because the priority of the task that reads from the queue is 
           higher than the priority of this task; as soon as this task writes 
           to the queue it will be pre-empted by the task that reads from the 
           queue, so the queue will already be empty again by the time the 
           call to xQueueSend() returns. The block time is set to 0. */
        xQueueSend( xQueue2, &pcMessage, 0 );
    }
}
```
***清单 5.25*** *示例 5.3 中使用的发送任务*


发送任务写入的队列是同一队列的成员
设置。每次任务发送到其中一个队列时，该队列的句柄
队列被发送到队列集。接收任务调用
`xQueueSelectFromSet()` 从队列集中读取队列句柄。
接收任务从集合中接收到队列句柄后，它就知道
接收到的句柄引用的队列包含数据，因此它读取
直接从队列中获取数据。它从队列中读取的数据是
指向接收任务打印出的字符串的指针。

如果对 `xQueueSelectFromSet()` 的调用超时，则返回 NULL。在示例中
5.3、`xQueueSelectFromSet()` 的调用具有无限的出块时间，因此
永远不会超时，并且只能返回有效的队列句柄。
因此，接收任务不需要检查是否
`xQueueSelectFromSet()` 在使用返回值之前返回了 NULL。

`xQueueSelectFromSet()` 仅返回队列句柄，如果队列
句柄引用包含数据，因此不需要使用
从队列读取时的阻塞时间。

清单 5.26 显示了接收任务的实现。

<a name="list5.26" title="Listing 5.26 The receive task used in Example 5.3"></a>

```c
void vReceiverTask( void *pvParameters )
{
    QueueHandle_t xQueueThatContainsData;
    char *pcReceivedString;

    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {
        /* Block on the queue set to wait for one of the queues in the set to
           contain data. Cast the QueueSetMemberHandle_t value returned from
           xQueueSelectFromSet() to a QueueHandle_t, as it is known all the 
           members of the set are queues (the queue set does not contain any 
           semaphores). */
        xQueueThatContainsData = ( QueueHandle_t ) xQueueSelectFromSet(
                                                     xQueueSet, portMAX_DELAY );

        /* An indefinite block time was used when reading from the queue set,
           so xQueueSelectFromSet() will not have returned unless one of the 
           queues in the set contained data, and xQueueThatContainsData cannot
           be NULL. Read from the queue. It is not necessary to specify a 
           block time because it is known the queue contains data. The block 
           time is set to 0. */
        xQueueReceive( xQueueThatContainsData, &pcReceivedString, 0 );

        /* Print the string received from the queue. */
        vPrintString( pcReceivedString );
    }
}
```
***清单 5.26*** *示例 5.3 中使用的接收任务*

图 5.7 显示了示例 5.3 生成的输出。可见
接收任务从两个发送任务接收字符串。街区
`vSenderTask1()` 使用的时间是 `vSenderTask1()` 使用的区块时间的一半
`vSenderTask2()`，导致打印出 `vSenderTask1()` 发送的字符串
是 `vSenderTask2()` 发送频率的两倍。

<a name="fig5.7" title="Figure 5.7 The output produced when Example 5.3 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image37.jpg)   
***图 5.7*** *执行例 5.3 时产生的输出*
* * *


### 5.6.5 更现实的队列集用例

例 5.3 演示了一个非常简单的情况；仅队列集
包含队列，并且它包含的两个队列都用于发送
字符指针。在实际应用中，队列集可能包含
队列和信号量，并且队列可能并不都持有相同的内容
数据类型。当出现这种情况时，需要测试该值
在使用返回值之前由 `xQueueSelectFromSet()` 返回。
清单 5.27 演示了如何使用从
当集合具有以下成员时为 `xQueueSelectFromSet()`：

- 二进制信号量。
- 从中读取字符指针的队列。
- 从中读取 `uint32_t` 值的队列。

清单 5.27 假设队列和信号量已经创建
并添加到队列集中。

<a name="list5.27" title="Listing 5.27 Using a queue set that contains queues and semaphores"></a>

```c
/* The handle of the queue from which character pointers are received. */
QueueHandle_t xCharPointerQueue;

/* The handle of the queue from which uint32_t values are received. */
QueueHandle_t xUint32tQueue;

/* The handle of the binary semaphore. */
SemaphoreHandle_t xBinarySemaphore;

/* The queue set to which the two queues and the binary semaphore belong. */
QueueSetHandle_t xQueueSet;

void vAMoreRealisticReceiverTask( void *pvParameters )
{
    QueueSetMemberHandle_t xHandle;
    char *pcReceivedString;
    uint32_t ulRecievedValue;
    const TickType_t xDelay100ms = pdMS_TO_TICKS( 100 );

    for( ;; )
    {
        /* Block on the queue set for a maximum of 100ms to wait for one of the
           members of the set to contain data. */
        xHandle = xQueueSelectFromSet( xQueueSet, xDelay100ms );

        /* Test the value returned from xQueueSelectFromSet(). If the returned
           value is NULL then the call to xQueueSelectFromSet() timed out. If 
           the returned value is not NULL then the returned value will be the 
           handle of one of the set's members. The QueueSetMemberHandle_t 
           value can be cast to either a QueueHandle_t or a SemaphoreHandle_t.
           Whether an explicit cast is required depends on the compiler. */

        if( xHandle == NULL )
        {
            /* The call to xQueueSelectFromSet() timed out. */
        }
        else if( xHandle == ( QueueSetMemberHandle_t ) xCharPointerQueue )
        {
            /* The call to xQueueSelectFromSet() returned the handle of the 
               queue that receives character pointers. Read from the queue. 
               The queue is known to contain data, so a block time of 0 is 
               used. */
            xQueueReceive( xCharPointerQueue, &pcReceivedString, 0 );

            /* The received character pointer can be processed here... */
        }
        else if( xHandle == ( QueueSetMemberHandle_t ) xUint32tQueue )
        {
            /* The call to xQueueSelectFromSet() returned the handle of the 
               queue that receives uint32_t types. Read from the queue. The 
               queue is known to contain data, so a block time of 0 is used. */
            xQueueReceive(xUint32tQueue, &ulRecievedValue, 0 );

            /* The received value can be processed here... */
        }
        else if( xHandle == ( QueueSetMemberHandle_t ) xBinarySemaphore )
        {
            /* The call to xQueueSelectFromSet() returned the handle of the 
               binary semaphore. Take the semaphore now. The semaphore is 
               known to be available so a block time of 0 is used. */
            xSemaphoreTake( xBinarySemaphore, 0 );

            /* Whatever processing is necessary when the semaphore is taken 
               can be performed here... */
        }
    }
}
```
***清单 5.27*** *使用包含队列和信号量的队列集*


## 5.7 使用队列创建邮箱

嵌入式社区内的术语尚未达成共识，并且
“邮箱”在不同的 RTOS 中具有不同的含义。在这本书中，
术语“邮箱”用于指代长度为 1 的队列。一个
队列可以被描述为邮箱，因为它的使用方式
应用程序，而不是因为它与应用程序有功能差异
队列：

- 队列用于将数据从一个任务发送到另一个任务，或者从
  任务的中断服务程序。发件人将物品放入
  队列，接收者从队列中删除该项目。的
  数据通过队列从发送方传递到接收方。

- 邮箱用于保存可由任何任务或任何任务读取的数据
  中断服务程序。数据不经过
  邮箱，但会保留在邮箱中，直到被覆盖。
  发件人覆盖邮箱中的值。接收器读取
  邮箱中的值，但不会从邮箱中删除该值
  邮箱。

本章介绍两个队列 API 函数，使队列能够
用作邮箱。

清单 5.28 显示了如何创建队列以用作邮箱。

<a name="list5.28" title="Listing 5.28 A queue being created for use as a mailbox"></a>

```c
/* A mailbox can hold a fixed size data item. The size of the data item is set
   when the mailbox (queue) is created. In this example the mailbox is created 
   to hold an Example_t structure. Example_t includes a time stamp to allow the
   data held in the mailbox to note the time at which the mailbox was last 
   updated. The time stamp used in this example is for demonstration purposes 
   only - a mailbox can hold any data the application writer wants, and the 
   data does not need to include a time stamp. */
typedef struct xExampleStructure
{
    TickType_t xTimeStamp;
    uint32_t ulValue;
} Example_t;

/* A mailbox is a queue, so its handle is stored in a variable of type
   QueueHandle_t. */
QueueHandle_t xMailbox;

void vAFunction( void )
{
    /* Create the queue that is going to be used as a mailbox. The queue has 
       a length of 1 to allow it to be used with the xQueueOverwrite() API 
       function, which is described below. */
    xMailbox = xQueueCreate( 1, sizeof( Example_t ) );
}
```
***清单 5.28*** *创建一个队列以用作邮箱*


### 5.7.1 xQueueOverwrite() API 功能

与 `xQueueSendToBack()` API 函数一样，`xQueueOverwrite()` API
函数将数据发送到队列。与 `xQueueSendToBack()` 不同，如果队列
已满，则 `xQueueOverwrite()` 覆盖已满的数据
在队列中。

`xQueueOverwrite()` 只能与长度为
一。覆盖模式将始终写入队列的前面，并且
更新队列指针的前面，但不会更新消息
等待。  如果定义了 `configASSERT`，则如果队列
长度> 1.

> *注意：切勿从中断服务程序中调用 `xQueueOverwrite()`。
> 中断安全版本 `xQueueOverwriteFromISR()` 应该用于
> 它的位置。*

<a name="list5.29" title="Listing 5.29 The xQueueOverwrite() API function prototype"></a>

```c
BaseType_t xQueueOverwrite( QueueHandle_t xQueue, const void * pvItemToQueue );
```
***清单 5.29*** *xQueueOverwrite() API 功能原型*

**xQueueOverwrite()参数及返回值**

- `xQueue`

数据所在队列的句柄为 sent (written)。队列句柄 
  将从对 `xQueueCreate()` 或 `xQueueCreateStatic()` 的调用中返回 
  用于创建队列。

- `pvItemToQueue`

指向要复制到队列中的数据的指针。

队列可以容纳的每个项目的大小是在队列创建时设置的
   创建，因此这么多字节将从 `pvItemToQueue` 复制到
   队列存储区。

- 返回值

即使队列已满，`xQueueOverwrite()` 也会写入队列，
  因此 `pdPASS` 是唯一可能的返回值。

清单 5.30 显示了如何使用 `xQueueOverwrite()` 写入清单 5.28 中创建的 mailbox
(queue)。

<a name="list5.30" title="Listing 5.30 Using the xQueueOverwrite() API function"></a>

```c
void vUpdateMailbox( uint32_t ulNewValue )
{
    /* Example_t was defined in Listing 5.28. */
    Example_t xData;

    /* Write the new data into the Example_t structure.*/
    xData.ulValue = ulNewValue;

    /* Use the RTOS tick count as the time stamp stored in the Example_t
       structure. */
    xData.xTimeStamp = xTaskGetTickCount();

    /* Send the structure to the mailbox - overwriting any data that is 
       already in the mailbox. */
    xQueueOverwrite( xMailbox, &xData );
}
```
***清单 5.30*** *使用 xQueueOverwrite() API 函数*


### 5.7.2 xQueuePeek() API 功能

`xQueuePeek()` receives (reads) 队列中的项目*不*删除
队列中的项目。 `xQueuePeek()`从头部接收数据
队列而不修改队列中存储的数据或顺序
哪些数据存储在队列中。

> *注意：切勿从中断服务程序中调用 `xQueuePeek()`。的
> 应使用中断安全版本 `xQueuePeekFromISR()` 来代替它。*

*`xQueuePeek()` 与 `xQueuePeek()` 具有相同的函数参数和返回值
`xQueueReceive()`.*

<a name="list5.31" title="Listing 5.31 The xQueuePeek() API function prototype"></a>

```c
BaseType_t xQueuePeek( QueueHandle_t xQueue,
                       void * const pvBuffer,
                       TickType_t xTicksToWait );
```
***清单 5.31*** *xQueuePeek() API 功能原型*


清单 5.32 显示 `xQueuePeek()` 用于接收发布到的项目
清单 5.30 中的 mailbox (queue)。

<a name="list5.32" title="Listing 5.32 Using the xQueuePeek() API function"></a>

```c
BaseType_t vReadMailbox( Example_t *pxData )
{
    TickType_t xPreviousTimeStamp;
    BaseType_t xDataUpdated;

    /* This function updates an Example_t structure with the latest value
       received from the mailbox. Record the time stamp already contained in 
       *pxData before it gets overwritten by the new data. */
    xPreviousTimeStamp = pxData->xTimeStamp;

    /* Update the Example_t structure pointed to by pxData with the data
       contained in the mailbox. If xQueueReceive() was used here then the 
       mailbox would be left empty, and the data could not then be read by 
       any other tasks. Using xQueuePeek() instead of xQueueReceive() ensures 
       the data remains in the mailbox.

       A block time is specified, so the calling task will be placed in the
       Blocked state to wait for the mailbox to contain data should the mailbox
       be empty. An infinite block time is used, so it is not necessary to 
       check the value returned from xQueuePeek(), as xQueuePeek() will only 
       return when data is available. */
    xQueuePeek( xMailbox, pxData, portMAX_DELAY );

    /* Return pdTRUE if the value read from the mailbox has been updated since
       this function was last called. Otherwise return pdFALSE. */
    if( pxData->xTimeStamp > xPreviousTimeStamp )
    {
        xDataUpdated = pdTRUE;
    }
    else
    {
        xDataUpdated = pdFALSE;
    }

    return xDataUpdated;
}
```
***清单 5.32*** *使用 xQueuePeek() API 函数*
