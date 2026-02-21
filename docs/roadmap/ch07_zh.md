# 7 中断管理

## 7.1 简介

### 7.1.1 事件

嵌入式实时系统必须采取行动响应事件
源于环境。例如，一个数据包到达
以太网 peripheral (the event) 可能需要将其传递给 TCP/IP
processing (the action) 堆栈。不平凡的系统必须
源自多个来源的服务事件，所有这些都将
具有不同的处理开销和响应时间要求。在
对于每种情况，都必须做出最佳事件处理的判断
实施策略：

- 应如何检测该事件？通常使用中断，但是
  也可以轮询输入。

- 当使用中断时，应该执行多少处理
  里面的中断服务routine (ISR)，外面有多少？它
  通常希望每个 ISR 尽可能短。

- 事件如何传达到 main (non-ISR) 代码，以及如何
  该代码的结构可以最好地适应处理
  潜在的异步发生？

FreeRTOS 不强加任何特定的事件处理策略
应用程序设计器，但确实提供了允许选择的功能
策略以简单且可维护的方式实施。

区分任务的优先级、
以及中断的优先级：

- 任务是与硬件无关的软件功能
  正在运行哪个 FreeRTOS。任务的优先级分配在
  应用程序编写者编写的软件，以及软件算法（
  调度程序）决定哪个任务将被置于运行状态。

- 虽然是用软件编写的，但中断服务程序是一个
  硬件功能，因为硬件控制哪个中断
  服务例程将运行以及何时运行。任务只会运行
  当没有 ISR 运行时，因此优先级最低的中断
  会中断最高优先级的任务，并且没有办法
  抢占 ISR 的任务。

FreeRTOS 将运行的所有架构都能够处理
中断，但有关中断入口和中断的详细信息
优先级分配，因架构而异。


### 7.1.2 范围

本章内容包括：

- 可以在中断内使用哪些 FreeRTOS API 函数
  服务常规。
- 将中断处理推迟到任务的方法。
- 如何创建和使用二进制信号量和计数信号量。
- 二进制信号量和计数信号量之间的差异。
- 如何使用队列将数据传入和传出中断服务
  例行公事。
- 中断嵌套模型可用于某些 FreeRTOS 端口。


## 7.2 从 ISR 使用 FreeRTOS API

### 7.2.1 中断安全 API

通常需要使用 FreeRTOS 提供的功能
API 的函数来自于 routine (ISR) 的中断服务，但是很多 FreeRTOS
API 函数执行在 ISR 内部无效的操作。最
其中值得注意的是将调用 API 函数的任务放入
阻塞状态 — 如果从 ISR 调用 API 函数，则它是
不是从任务中调用的，因此没有可以调用的任务
置于阻塞状态。 FreeRTOS 通过提供解决了这个问题
API 部分功能的两个版本；一个供任务使用的版本，以及
一种供 ISR 使用的版本。打算从ISR使用的功能有
他们的名字后面附加了“FromISR”。

> *注意：切勿调用没有“FromISR”的 FreeRTOS API 函数
> 其名称取自 ISR。*


### 7.2.2 使用单独的中断安全 API 的好处

在中断中使用单独的 API 可以使任务代码更多
高效，ISR 代码更高效，中断入口更高效
更简单。要了解原因，请考虑替代解决方案，该解决方案将具有
提供每个 API 函数的单一版本
从任务和 ISR 调用。如果相同版本的API
可以从任务和 ISR 调用该函数，然后：

- API 函数需要额外的逻辑来确定它们是否
  已从任务或 ISR 中调用。额外的逻辑将
  通过函数引入新的路径，使得函数
  更长、更复杂、更难测试。

- 当函数被调用时，某些 API 函数参数将被废弃。
  从任务中调用，而其他函数在该函数时就会过时
  是从 ISR 调用的。

- 每个 FreeRTOS 端口都需要提供一种机制来确定
  执行context (task or ISR)。

- 不容易确定执行的架构
  context (task or ISR) 需要额外的、浪费的、更多的
  使用复杂，并且允许非标准中断入口代码
  软件提供的执行上下文。


### 7.2.3 使用单独的中断安全 API 的缺点

某些 API 函数具有两个版本，允许任务和 ISR
效率更高，但引入了新问题；有时是
需要调用不属于 FreeRTOS API 的函数，但是
从任务和 ISR 中使用 FreeRTOS API。

这通常只是集成第三方代码时的一个问题，因为
这是软件设计失控的唯一一次
应用程序编写者的。如果这确实成为一个问题，那么问题就来了
可以使用以下技术之一来克服：

- 将中断处理推迟到任务[^12]，因此 API 函数为
   仅从任务上下文中调用。

- 如果您使用的是支持中断嵌套的 FreeRTOS 端口，
  然后使用以“FromISR”结尾的 API 函数的版本，如下
  该版本可以从任务和 ISR 中调用。 （反过来则不是
  true，不得调用不以“FromISR”结尾的 API 函数
  来自 ISR。）

- 第三方代码通常包含 RTOS 抽象层
  可以用来测试函数所在的上下文
  为called (task or interrupt)，然后调用API函数
  这是适合上下文的。


[^12]：延迟中断处理将在下一节中介绍
这本书。


### 7.2.4 xHigherPriorityTaskWoken 参数

本节介绍 `xHigherPriorityTaskWoken` 的概念
参数。如果您不完全理解这一点，请不要担心
部分，因为以下部分提供了实际示例。

如果上下文切换是通过中断执行的，则任务运行
中断退出的时间可能与正在运行的任务不同
当进入中断时——中断将中断一个
任务，但返回到另一个任务。

某些 FreeRTOS API 函数可以将任务从阻塞状态移至
就绪状态。这已经在诸如
`xQueueSendToBack()`，如果有任务，则会解锁任务
处于阻塞状态，等待有关该主题的数据可用
队列。

如果由 FreeRTOS API 函数解锁的任务的优先级
高于Running状态任务的优先级，那么，在
按照FreeRTOS的调度策略，切换到更高的
优先任务应该发生。当切换到更高优先级的任务时
实际发生取决于 API 函数的上下文
称为：

- 如果从任务调用 API 函数：

如果 FreeRTOSConfig.h 中的 `configUSE_PREEMPTION` 设置为 1，则
  API 内自动切换到更高优先级的任务
  函数，换句话说，在 API 函数退出之前。这已经是
  如图 6.6 所示，其中写入定时器命令队列的结果
  在写入的函数之前切换到 RTOS 守护程序任务
  命令队列已退出。

- 如果从中断调用 API 函数：

内部不会自动切换到更高优先级的任务
  一个中断。相反，设置一个变量来通知应用程序
  作者认为应该执行上下文切换。中断安全 API
  functions (those that end in "FromISR") 有一个指针参数
  称为 `pxHigherPriorityTaskWoken`，用于此目的。

如果应执行上下文切换，则中断安全 API
  函数会将 `*pxHigherPriorityTaskWoken` 设置为 `pdTRUE`。为了能够
  检测到这种情况已经发生，变量指向
  `pxHigherPriorityTaskWoken` 必须先初始化为 `pdFALSE`，然后才能使用
  第一次使用。

如果应用程序编写者选择不请求上下文切换
  ISR，则较高优先级的任务将保持就绪状态
  直到调度程序下次运行，在最坏的情况下将是
  在下一个滴答中断期间。

FreeRTOS API 函数只能将 `*pxHighPriorityTaskWoken` 设置为
  `pdTRUE`。如果 ISR 调用多个 FreeRTOS API 函数，则
  相同的变量可以作为 `pxHigherPriorityTaskWoken` 参数传递
  在每个 API 函数调用中，变量只需要是
  首次使用前初始化为`pdFALSE`。

不发生上下文切换的原因有几个
自动在 API 函数的中断安全版本内：

- 避免不必要的上下文切换

在需要执行中断之前，中断可能会执行多次
  任务来执行任何处理。例如，考虑一个场景
  其中任务处理由中断接收的字符串
  驱动UART； UART ISR 切换到
  每次收到角色时都会执行任务，因为该任务只会
  在完整的字符串被处理后执行处理
  收到。

- 控制执行顺序

中断可能会偶尔发生，并且发生的时间不可预测。
  FreeRTOS 专家用户可能希望暂时避免不可预测的情况
  在他们的特定时间点切换到不同的任务
  应用程序，尽管这也可以使用 FreeRTOS 来实现
  调度程序锁定机制。

- 便携性

它是可在所有 FreeRTOS 端口上使用的最简单的机制。

- 效率

针对较小处理器架构的端口仅允许
  在 ISR 的最后请求上下文切换，以及
  消除该限制将需要额外的、更复杂的
  代码。它还允许多次调用 FreeRTOS API 函数
  在同一个 ISR 内，不会生成多个请求
  同一 ISR 内的上下文切换。

- 在 RTOS 滴答中断中执行

正如本书后面将看到的，可以添加
  应用程序代码进入 RTOS 滴答中断。结果
  尝试在滴答中断内进行上下文切换是相关的
  在使用的 FreeRTOS 端口上。充其量，这将导致
  对调度程序进行不必要的调用。

`pxHigherPriorityTaskWoken` 参数的使用是可选的。如果不是
需要，然后将 `pxHigherPriorityTaskWoken` 设置为 NULL。


### 7.2.5 portYIELD\_FROM\_ISR() 和 portEND\_SWITCHING\_ISR() 宏

本节介绍用于请求上下文的宏
从 ISR 切换。如果您不完全理解，请不要担心
本节尚未完成，下面提供了实际示例
部分。

`taskYIELD()` 是一个宏，可以在任务中调用来请求上下文
开关。 `portYIELD_FROM_ISR()` 和 `portEND_SWITCHING_ISR()` 都是
`taskYIELD()` 的中断安全版本。 `portYIELD_FROM_ISR()` 和
`portEND_SWITCHING_ISR()` 两者的使用方式相同，并且执行相同的操作
东西[^13]。某些 FreeRTOS 端口仅提供这两个宏之一。
较新的 FreeRTOS 端口提供这两个宏。本书中的例子使用
`portYIELD_FROM_ISR()`。

[^13]：从历史上看，`portEND_SWITCHING_ISR()` 是在
需要中断处理程序才能使用程序集的 FreeRTOS 端口
代码包装器，`portYIELD_FROM_ISR()` 是 FreeRTOS 中使用的名称
允许用 C 语言编写整个中断处理程序的端口。


<a name="list7.1" title="Listing 7.1 The portEND\_SWITCHING\_ISR() macros"></a>

```c
portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
```
***清单 7.1*** *portEND_SWITCHING\_ISR() 宏*


<a name="list7.2" title="Listing 7.2 The portYIELD\_FROM\_ISR() macros"></a>

```c
portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
```
***清单 7.2*** *portYIELD\_FROM\_ISR() 宏*


`xHigherPriorityTaskWoken` 参数从中断安全中传递出去
API函数可以直接用作调用中的参数
`portYIELD_FROM_ISR()`。

如果 `portYIELD_FROM_ISR()` `xHigherPriorityTaskWoken` 参数为
`pdFALSE` (zero)，则不请求上下文切换，并且宏
没有影响。如果 `portYIELD_FROM_ISR()` `xHigherPriorityTaskWoken`
参数不是 `pdFALSE`，则请求上下文切换，并且
处于运行状态的任务可能会发生变化。中断总会返回
到运行状态的任务，即使任务处于运行状态
当中断执行时改变。

大多数 FreeRTOS 端口允许在任何地方调用 `portYIELD_FROM_ISR()`
在 ISR 内。一些 FreeRTOS 端口（主要是较小的端口）
架构），只允许在最开始调用 `portYIELD_FROM_ISR()`
ISR 的末尾。


## 7.3 延迟中断处理

通常认为最佳实践是保持 ISR 尽可能短
可能的。其原因包括：

- 即使任务被分配了非常高的优先级，它们也会
  仅当硬件没有提供中断服务时才运行。

- ISR 可以 disrupt (add 'jitter' to) 开始时间和
  任务的执行时间。

- 根据运行 FreeRTOS 的架构，它可能
  不可能接受任何新的中断，或者至少是一个子集
  当 ISR 正在执行时，新中断的数量。

- 应用程序编写者需要考虑以下后果：
  防范变量、外设和内存等资源
  任务和 ISR 同时访问缓冲区。

- 某些FreeRTOS端口允许中断嵌套，但中断嵌套
  会增加复杂性并降低可预测性。越短的
  中断越大，嵌套的可能性就越小。

中断服务程序必须记录中断的原因，并且
清除中断。中断所需的任何其他处理
通常可以在任务中执行，允许中断服务程序
尽快退出。这称为“延迟中断”
处理'，因为中断所需的处理是
从 ISR“推迟”到任务。

将中断处理推迟到任务还允许应用程序
作者确定相对于其他任务的处理优先级
应用程序，并使用所有 FreeRTOS API 功能。

如果中断处理被推迟的任务的优先级是
高于任何其他任务的优先级，那么处理将是
立即执行，就像处理已在
ISR 本身。该场景如图7.1所示，其中任务1是
一个普通的应用程序任务，任务2是中断的任务
处理被推迟。


<a name="fig7.1" title="Figure 7.1 Completing interrupt processing in a high priority task"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image48.png)
***图7.1*** *完成高优先级任务中的中断处理*

* * *

在图 7.1 中，中断处理在时间 t2 开始，并且有效
在时间 t4 结束，但仅花费时间 t2 和 t3 之间的时间段
ISR。如果未使用延迟中断处理，则
时间 t2 和 t4 之间的整个时间段将花费在 ISR 中。

关于什么时候最好执行所有操作并没有绝对的规则
ISR 中的中断所需的处理，以及何时最好
将部分处理推迟到任务中。将处理推迟到
任务在以下情况下最有用：

- 中断所需的处理并不简单。对于
  例如，如果中断只是将模拟结果存储到
  数字转换，那么几乎可以肯定这是最好的执行方式
  ISR 内部，但如果还必须传递转换结果
  通过软件过滤器，那么最好执行过滤器
  在一个任务中。

- 方便中断处理执行动作
  无法在 ISR 内部执行的操作，例如写入控制台，
  或分配内存。

- 中断处理不是确定性的——这意味着它不是
  提前知道处理需要多长时间。

以下部分描述并演示了所介绍的概念
本章到目前为止，包括可用于
实现延迟中断处理。


## 7.4 用于同步的二进制信号量

二进制信号量 API 的中断安全版本可用于
每次发生特定中断时有效地解锁任务
使任务与中断同步。这使得大多数
中断事件处理要在synchronized内部实现
任务，只有非常快且短的部分直接保留在
ISR。如上一节所述，使用二进制信号量
将中断处理“推迟”到任务[^14]。

[^14]：使用中断来解除任务阻塞会更有效
直接任务通知而不是使用二进制信号量。
直到第 10 章“任务”才涉及直接任务通知
通知。

如图 7.1 所示，如果中断处理是
时间特别紧迫，则优先处理延迟处理
可以设置任务以确保该任务始终抢占其他任务
系统。然后可以实现 ISR 以包含对
`portYIELD_FROM_ISR()`，确保ISR直接返回任务
哪个中断处理被推迟。这有以下效果
确保整个事件处理连续执行（无需
中断）及时，就像它已全部在 ISR 中实现一样
本身。图 7.2 重复了图 7.1 中所示的场景，但是
更新文本以描述如何执行延迟处理
可以使用信号量来控制任务。


<a name="fig7.2" title="Figure 7.2 Using a binary semaphore to implement deferred interrupt processing"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image49.png)
***图7.2*** *使用二进制信号量实现延迟中断处理*

* * *

延迟处理任务使用对信号量的阻塞“take”调用
作为进入阻塞状态以等待事件发生的一种手段。
当事件发生时，ISR 对同一事件使用“give”操作
信号量来解锁任务，以便所需的事件处理可以
继续。

“获取信号量”和“给出信号量”是具有以下概念的概念：
根据使用场景不同含义也不同。在这个中断中
同步场景，可以考虑二值信号量
从概念上讲，它是一个长度为 1 的队列。队列可以包含一个
任何时候最多只能有一项，因此始终为空或 full
(hence, binary)。通过调用 `xSemaphoreTake()`，任务
中断处理被有效地推迟尝试读取
具有阻塞时间的队列，如果满足以下条件，则导致任务进入阻塞状态：
队列是空的。当事件发生时，ISR 使用
`xSemaphoreGiveFromISR()` 函数可将 token (the semaphore) 放入
队列，使队列满了。这会导致任务退出
阻塞状态并删除令牌，使队列再次为空。
当任务完成处理后，它会再次尝试
从队列中读取，发现队列为空，重新进入Blocked
状态等待下一个事件。这个序列被证明在
图 7.3。

图 7.3 显示了中断“给出”信号量，尽管它有
不是首先“采取”它，并且任务“采取”信号量，但从来没有
还给它。这就是为什么该场景被描述为
在概念上类似于写入和读取队列。它经常
导致混乱，因为它不遵循与其他信号量相同的规则
使用场景，其中需要信号量的任务必须始终提供信号量
返回——例如第 8 章“资源管理”中描述的场景。


<a name="fig7.3" title="Figure 7.3 Using a binary semaphore to synchronize a task with an interrupt"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image50.png)
***图 7.3*** *使用二进制信号量使任务与中断同步*

* * *


### 7.4.1 xSemaphoreCreateBinary() API 功能

FreeRTOS 还包括 `xSemaphoreCreateBinaryStatic()`
函数，它分配创建二进制文件所需的内存
编译时静态信号量：处理所有各种类型
FreeRTOS 信号量存储在类型变量中
`SemaphoreHandle_t`。

在使用信号量之前，必须先创建它。创建二进制文件
信号量，使用 `xSemaphoreCreateBinary()` API 函数[^15]。

[^15]：一些 Semaphore API 函数实际上是宏，而不是函数。
为了简单起见，它们在全文中都称为函数
这本书。


<a name="list7.3" title="Listing 7.3 The xSemaphoreCreateBinary() API function prototype"></a>

```c
SemaphoreHandle_t xSemaphoreCreateBinary( void );
```
***清单 7.3*** *xSemaphoreCreateBinary() API 功能原型*

**xSemaphoreCreateBinary() 返回值**

- 返回值

如果返回 NULL，则无法创建信号量，因为
  FreeRTOS 没有足够的堆内存来分配
  信号量数据结构。

如果返回非NULL值，则表明该信号量已被
  创建成功。返回值应存储为句柄
  到创建的信号量。


### 7.4.2 xSemaphoreTake() API 功能

“获取”信号量意味着“获得”或“接收”信号量。的
仅当信号量可用时才可以获取。

所有各种类型的 FreeRTOS 信号量，除了递归互斥体之外，
可以使用 `xSemaphoreTake()` 函数“获取”。

不得在中断服务程序中使用 `xSemaphoreTake()`。


<a name="list7.4" title="Listing 7.4 The xSemaphoreTake() API function prototype"></a>

```c
BaseType_t xSemaphoreTake( SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait );
```
***清单 7.4*** *xSemaphoreTake() API 功能原型*

**xSemaphoreTake()参数及返回值**

- `xSemaphore`

信号量被“占用”。

信号量由 `SemaphoreHandle_t` 类型的变量引用。它
  在使用之前必须显式创建。

- `xTicksToWait`

任务应保留在已阻止状态的最长时间
  如果信号量尚不可用，则状态等待信号量。

如果 `xTicksToWait` 为零，则 `xSemaphoreTake()` 将返回
  如果信号量不可用，则立即执行。

区块时间以滴答周期为单位指定，因此它的绝对时间
  表示取决于滴答频率。宏 `pdMS_TO_TICKS()`
  可用于将以毫秒为单位的时间转换为时间
  以刻度指定。

将 `xTicksToWait` 设置为 `portMAX_DELAY` 将导致任务等待
  indefinitely (without a timeout)（如果 `INCLUDE_vTaskSuspend` 设置为 1）
  FreeRTOSConfig.h。

- 返回值

有两种可能的返回值：

- `pdPASS`

仅当调用 `xSemaphoreTake()` 时，才会返回 `pdPASS`
    成功获取信号量。

如果区块时间为 specified (`xTicksToWait` was not zero)，那么它是
    可能调用任务被置于阻塞状态等待
    对于信号量，如果它不是立即可用，但信号量
    在区块时间到期之前变得可用。

- `pdFALSE`

信号量不可用。

如果区块时间为 specified (`xTicksToWait` was not zero)，则
    调用任务将被置于阻塞状态等待
    信号量变得可用，但块时间之前已过期
    这件事发生了。


### 7.4.3 xSemaphoreGiveFromISR() API 功能

可以使用以下命令“给出”二进制和计数信号量[^16]
`xSemaphoreGiveFromISR()` 函数。

[^16]：计数信号量将在本书的后面部分中描述。

`xSemaphoreGiveFromISR()` 是中断安全版本
`xSemaphoreGive()`，因此 `pxHigherPriorityTaskWoken` 参数
已在本章开头进行了描述。


<a name="list" title="Listing 7.5 The xSemaphoreGiveFromISR() API function prototype"></a>

```c
BaseType_t xSemaphoreGiveFromISR( SemaphoreHandle_t xSemaphore,
                                  BaseType_t *pxHigherPriorityTaskWoken );
```
***清单 7.5*** *xSemaphoreGiveFromISR() API 功能原型*

**xSemaphoreGiveFromISR()参数及返回值**

- `xSemaphore`

信号量被“给出”。

信号量由 `SemaphoreHandle_t` 类型的变量引用，
  并且在使用之前必须显式创建。

- `pxHigherPriorityTaskWoken`

单个信号量可能有一个或多个
  任务阻塞在其上等待信号量变得可用。
  调用`xSemaphoreGiveFromISR()`可以使信号量可用，等等
  导致等待信号量离开阻塞状态的任务
  状态。如果调用 `xSemaphoreGiveFromISR()` 导致任务离开
  阻塞状态，且非阻塞任务的优先级高于
  当前正在执行 task (the task that was interrupted)，那么，
  在内部，`xSemaphoreGiveFromISR()` 将设置 `*pxHigherPriorityTaskWoken`
  至 `pdTRUE`。

如果 `xSemaphoreGiveFromISR()` 将此值设置为 `pdTRUE`，则通常
  上下文切换应该在中断退出之前执行。这个
  会保证中断直接返回到最高优先级
  就绪状态任务。

- 返回值

有两种可能的返回值：

- `pdPASS`

仅当调用 `xSemaphoreGiveFromISR()` 时才会返回 `pdPASS`
    是成功的。

- `pdFAIL`

如果信号量已经可用，则无法给出它，并且
    `xSemaphoreGiveFromISR()` 将返回 `pdFAIL`。


<a name="example7.1" title="Example 7.1 Using a binary semaphore to synchronize a task with an interrupt"></a>
---
***示例 7.1*** *使用二进制信号量使任务与中断同步*

---

此示例使用二进制信号量来解除任务对中断的阻塞
服务例程，有效地将任务与中断同步。

一个简单的周期性任务用于每隔一段时间生成一个软件中断
500 毫秒。使用软件中断是为了方便，因为
挂钩到某个目标中的真实中断的复杂性
环境。清单 7.6 显示了周期性任务的实现。
请注意，该任务在之前和之后都打印出一个字符串
产生中断。这使得执行顺序可以是
在执行示例时产生的输出中观察到。


<a name="list7.6" title="Listing 7.6 Implementation of the task that periodically generates a software interrupt in Example 7.1"></a>

```c
/* The number of the software interrupt used in this example. The code
   shown is from the Windows project, where numbers 0 to 2 are used by the
   FreeRTOS Windows port itself, so 3 is the first number available to the
   application. */
#define mainINTERRUPT_NUMBER 3

static void vPeriodicTask( void *pvParameters )
{
    const TickType_t xDelay500ms = pdMS_TO_TICKS( 500UL );

    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {
        /* Block until it is time to generate the software interrupt again. */
        vTaskDelay( xDelay500ms );

        /* Generate the interrupt, printing a message both before and after
           the interrupt has been generated, so the sequence of execution is
           evident from the output.

           The syntax used to generate a software interrupt is dependent on
           the FreeRTOS port being used. The syntax used below can only be
           used with the FreeRTOS Windows port, in which such interrupts are
           only simulated. */
        vPrintString( "Periodic task - About to generate an interrupt.\r\n" );
        vPortGenerateSimulatedInterrupt( mainINTERRUPT_NUMBER );
        vPrintString( "Periodic task - Interrupt generated.\r\n\r\n\r\n" );
    }
}
```
***清单 7.6*** *例 7.1 中周期性生成软件中断的任务的实现*


清单 7.7 显示了中断任务的实现
处理被推迟——与软件同步的任务
通过使用二进制信号量来中断。同样，字符串是
在任务的每次迭代中打印出来，因此顺序
任务和中断执行从以下时产生的输出中可以明显看出
该示例已执行。

应该注意的是，虽然清单 7.7 中所示的代码已经足够了
对于例 7.1，中断是由软件生成的，它不是
适合硬件产生中断的场景
外围设备。下面的小节描述了如何结构
需要更改代码以使其适合与硬件一起使用
产生中断。


<a name="list7.7." title="Listing 7.7 The implementation of the task to which the interrupt processing is deferred (the task that..."></a>

```c
static void vHandlerTask( void *pvParameters )
{
    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {
        /* Use the semaphore to wait for the event. The semaphore was created
           before the scheduler was started, so before this task ran for the
           first time. The task blocks indefinitely, meaning this function
           call will only return once the semaphore has been successfully
           obtained - so there is no need to check the value returned by
           xSemaphoreTake(). */
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );

        /* To get here the event must have occurred. Process the event (in
           this Case, just print out a message). */
        vPrintString( "Handler task - Processing event.\r\n" );
    }
}
```
***清单7.7*** *例7.1中中断处理为deferred (the task that synchronizes with the interrupt)的任务的实现*


清单 7.8 显示了 ISR。除了“给予”之外，这几乎没有什么作用
信号量来解锁中断处理被推迟的任务。

请注意 `xHigherPriorityTaskWoken` 变量的使用方式。它被设置为
`pdFALSE` 在调用 `xSemaphoreGiveFromISR()` 之前，然后用作
调用 `portYIELD_FROM_ISR()` 时的参数。上下文切换将是
在 `portYIELD_FROM_ISR()` 宏内部请求，如果
`xHigherPriorityTaskWoken` 等于 `pdTRUE`。

ISR的原型，以及调用强制上下文的宏
开关，对于 FreeRTOS Windows 端口都是正确的，并且可能是
其他 FreeRTOS 端口有所不同。具体参见端口
FreeRTOS.org 网站上的文档页面以及示例
FreeRTOS 下载中提供，查找所需的语法
您正在使用的端口。

与运行 FreeRTOS 的大多数架构不同，FreeRTOS Windows
端口需要 ISR 返回值。实施
Windows 端口提供的 `portYIELD_FROM_ISR()` 宏包括
return 语句，因此清单 7.8 没有显示返回的值
明确地。


<a name="list7.8" title="Listing 7.8 The ISR for the software interrupt used in Example 7.1"></a>

```c
static uint32_t ulExampleInterruptHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken;

    /* The xHigherPriorityTaskWoken parameter must be initialized to
       pdFALSE as it will get set to pdTRUE inside the interrupt safe
       API function if a context switch is required. */
    xHigherPriorityTaskWoken = pdFALSE;

    /* 'Give' the semaphore to unblock the task, passing in the address of
       xHigherPriorityTaskWoken as the interrupt safe API function's
       pxHigherPriorityTaskWoken parameter. */
    xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );

    /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR().
       If xHigherPriorityTaskWoken was set to pdTRUE inside
       xSemaphoreGiveFromISR() then calling portYIELD_FROM_ISR() will request
       a context switch. If xHigherPriorityTaskWoken is still pdFALSE then
       calling portYIELD_FROM_ISR() will have no effect. Unlike most FreeRTOS
       ports, the Windows port requires the ISR to return a value - the return
       statement is inside the Windows version of portYIELD_FROM_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```
***清单 7.8*** *示例 7.1 中使用的软件中断的 ISR*

`main()` 函数创建二进制信号量，创建任务，
安装中断处理程序，并启动调度程序。的
实现如清单 7.9 所示。

调用安装中断处理程序的函数的语法是
特定于 FreeRTOS Windows 端口，对于其他端口可能有所不同
FreeRTOS 端口。请参阅端口特定文档页面
FreeRTOS.org 网站，以及 FreeRTOS 中提供的示例
下载，找到您正在使用的端口所需的语法。


<a name="list7.9" title="Listing 7.9 The implementation of main() for Example 7.1"></a>

```c
int main( void )
{
    /* Before a semaphore is used it must be explicitly created. In this
       example a binary semaphore is created. */
    xBinarySemaphore = xSemaphoreCreateBinary();

    /* Check the semaphore was created successfully. */
    if( xBinarySemaphore != NULL )
    {
        /* Create the 'handler' task, which is the task to which interrupt
           processing is deferred. This is the task that will be synchronized
           with the interrupt. The handler task is created with a high priority
           to ensure it runs immediately after the interrupt exits. In this
           case a priority of 3 is chosen. */
        xTaskCreate( vHandlerTask, "Handler", 1000, NULL, 3, NULL );

        /* Create the task that will periodically generate a software
           interrupt. This is created with a priority below the handler task
           to ensure it will get preempted each time the handler task exits
           the Blocked state. */
        xTaskCreate( vPeriodicTask, "Periodic", 1000, NULL, 1, NULL );

        /* Install the handler for the software interrupt. The syntax necessary
           to do this is dependent on the FreeRTOS port being used. The syntax
           shown here can only be used with the FreeRTOS windows port, where
           such interrupts are only simulated. */
        vPortSetInterruptHandler( mainINTERRUPT_NUMBER,
                                  ulExampleInterruptHandler );

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    }

    /* As normal, the following line should never be reached. */
    for( ;; );
}
```
***清单 7.9*** *示例 7.1 中 main() 的实现*


例 7.1 产生如图 7.4 所示的输出。正如预期的那样，
`vHandlerTask()` 一旦中断就进入运行状态
生成，因此任务的输出分割了任务产生的输出
周期性任务。图 7.5 提供了进一步的解释。


<a name="fig7.4" title="Figure 7.4 The output produced when Example 7.1 is executed"></a>
<a name="fig7.5" title="Figure 7.5 The sequence of execution when Example 7.1 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image51.jpg)
***图 7.4*** *执行例 7.1 时产生的输出*

![](https://cloud.rocketpi.club/cloud/image52.png)
***图7.5*** *执行例7.1时的执行顺序*

* * *


### 7.4.4 改进示例 7.1 中使用的任务的实现

例 7.1 使用二进制信号量来同步任务
中断。执行顺序如下：

1. 发生中断。

1. ISR 执行并“给出”信号量以解锁任务。

1. 任务在ISR之后立即执行，并‘拿’了
   信号量。

1. 任务处理事件，然后尝试“获取”信号量
   再次进入阻塞状态，因为信号量还没有
   available (another interrupt had not yet occurred)。

仅当满足以下条件时，示例 7.1 中使用的任务结构才足够：
中断发生的频率相对较低。要了解原因，
考虑如果第二个、然后第三个中断发生，会发生什么
发生在任务完成第一个任务的处理之前
中断：

- 当第二个 ISR 执行时，信号量将为空，因此
  ISR 将给出信号量，任务将处理第二个
  处理完第一个事件后立即发生事件。
  该场景如图 7.6 所示。

- 当执行第三个 ISR 时，信号量已经是
  可用，防止 ISR 再次发出信号量，因此
  任务不会知道第三个事件已经发生。那个场景是
  如图 7.7 所示。


<a name="fig7.6" title="Figure 7.6 The scenario when one interrupt occurs before the task has finished processing the first event"></a>
<a name="fig7.7" title="Figure 7.7 The scenario when two interrupts occur before the task has finished processing the first event"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image53.png)
***图7.6*** *任务处理完第一个事件之前发生中断的情况*

![](https://cloud.rocketpi.club/cloud/image54.png)
***图7.7*** *任务处理完第一个事件之前发生两次中断的场景*

* * *

例 7.1 中使用的延迟中断处理任务如图所示
清单 7.7 的结构使得它只处理之间的一个事件
每次调用 `xSemaphoreTake()`。这对于例 7.1 来说已经足够了，因为
生成事件的中断是由软件触发的，并且
发生在可预测的时间。在实际应用中，中断是
由硬件生成，并在不可预测的时间发生。因此，要
最大限度地减少错过中断的机会，延迟中断
处理任务必须结构化，以便它处理所有事件
在每次调用 `xSemaphoreTake()`[^17] 之间已经可用。
清单 7.10 对此进行了演示，其中显示了延迟中断如何
可以构造 UART 的处理程序。在清单 7.10 中，假设
UART 每次接收到字符时都会生成接收中断，
并且 UART 将接收到的字符放入硬件 FIFO（
硬件缓冲区）。

[^17]：或者，计数信号量，或直接任务
通知，可用于对事件进行计数。计数信号量是
在下一节中描述。直接任务通知是
第 10 章“任务通知”中进行了描述。直接任务
通知是首选方法，因为它们是最常用的
在运行时间和 RAM 使用方面都很高效。

例 7.1 中使用的延迟中断处理任务还有另外一个任务
弱点；当它调用 `xSemaphoreTake()` 时，它没有使用超时。
相反，该任务将 `portMAX_DELAY` 作为 `xSemaphoreTake()` 传递
`xTicksToWait` 参数，这会导致任务等待 indefinitely
(without a timeout) 信号量可用。不定
示例代码中经常使用超时，因为它们的使用简化了
示例的结构，因此使示例更容易
明白。然而，无限期超时通常是不好的做法
真正的应用程序，因为它们使得很难从
错误。作为一个例子，考虑一个任务正在等待的场景
一个中断来提供信号量，但硬件中的错误状态是
防止中断产生：

- 如果任务正在等待而没有超时，它不会知道
  错误状态，并将永远等待。

- 如果任务等待超时，则 `xSemaphoreTake()` 将
  超时后返回`pdFAIL`，任务即可
  下次执行时检测并清除错误。这个场景
  代码清单 7.10 也对此进行了演示。


<a name="list7.10" title="Listing 7.10 The recommended structure of a deferred interrupt processing task, using a UART receive..."></a>

```c
static void vUARTReceiveHandlerTask( void *pvParameters )
{
    /* xMaxExpectedBlockTime holds the maximum time expected between two
       interrupts. */
    const TickType_t xMaxExpectedBlockTime = pdMS_TO_TICKS( 500 );

    /* As per most tasks, this task is implemented within an infinite loop. */
    for( ;; )
    {
        /* The semaphore is 'given' by the UART's receive (Rx) interrupt.
           Wait a maximum of xMaxExpectedBlockTime ticks for the next
           interrupt. */
        if( xSemaphoreTake( xBinarySemaphore, xMaxExpectedBlockTime ) == pdPASS)
        {
            /* The semaphore was obtained. Process ALL pending Rx events before
               calling xSemaphoreTake() again. Each Rx event will have placed a
               character in the UART's receive FIFO, and UART_RxCount() is
               assumed to return the number of characters in the FIFO. */
            while( UART_RxCount() > 0 )
            {
                /* UART_ProcessNextRxEvent() is assumed to process one Rx
                   character, reducing the number of characters in the FIFO
                   by 1. */
                UART_ProcessNextRxEvent();
            }

            /* No more Rx events are pending (there are no more characters in
               the FIFO), so loop back and call xSemaphoreTake() to wait for
               the next interrupt. Any interrupts occurring between this point
               in the code and the call to xSemaphoreTake() will be latched in
               the semaphore, so will not be lost. */
        }
        else
        {
            /* An event was not received within the expected time. Check for,
               and if necessary clear, any error conditions in the UART that
               might be preventing the UART from generating any more
               interrupts. */
            UART_ClearErrors();
        }
    }
}
```
***清单 7.10*** *延迟中断处理任务的推荐结构，以 UART 接收处理程序为例*


## 7.5 计数信号量

正如二进制信号量可以被视为具有长度的队列一样
其中之一，计数信号量可以被认为是具有
长度大于一。任务对数据不感兴趣
存储在队列中——只是队列中的项目数。
FreeRTOSConfig.h 中的 `configUSE_COUNTING_SEMAPHORES` 必须设置为 1
计算可用信号量。

每次“给出”计数信号量时，其队列中的另一个空间
使用过。队列中的项目数是信号量的“计数”值。

计数信号量通常用于两件事：

1. 统计事件[^18]

在这种情况下，事件处理程序每次都会“给出”一个信号量
   事件发生，导致信号量的计数值变为
   每次“给予”都会增加。任务每次都会“获取”一个信号量
   它处理一个事件，导致信号量的计数值是
   每次“采取”时递减。计数值就是差值
   已发生的事件数与已发生的事件数之间的关系
   已被处理。该机制如图 7.8 所示。

用于对事件进行计数的计数信号量是通过以下命令创建的
   初始计数值为零。

[^18]：使用直接任务来计数事件更有效
   通知比使用计数信号量更重要。直接任务
   直到第 10 章才涉及通知。

1.资源管理。

在该场景中，count值表示资源的数量
   可用。为了获得资源的控制权，任务必须首先获得
   信号量，减少信号量的计数值。当计数时
   值为零，没有免费资源。当一个任务
   完成资源，它“给出”信号量
   返回，这会增加信号量的计数值。

创建用于管理资源的计数信号量
   它们的初始计数值等于资源的数量
   可用。第 7 章介绍使用信号量进行管理
   资源。


<a name="fig7.8" title="Figure 7.8 Using a counting semaphore to 'count' events"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image55.png)
***图 7.8*** *使用计数信号量来“计数”事件*

* * *

### 7.5.1 xSemaphoreCreateCounting() API 功能

FreeRTOS 还包括 `xSemaphoreCreateCountingStatic()`
函数，它分配创建计数所需的内存
编译时静态信号量：处理所有各种类型
FreeRTOS 信号量存储在 `SemaphoreHandle_t` 类型的变量中。

在使用信号量之前，必须先创建它。创建计数
信号量，使用 `xSemaphoreCreateCounting()` API 函数。


<a name="list7.11" title="Listing 7.11 The xSemaphoreCreateCounting() API function prototype"></a>

```c
SemaphoreHandle_t xSemaphoreCreateCounting( UBaseType_t uxMaxCount,
                                            UBaseType_t uxInitialCount );
```
***清单 7.11*** *xSemaphoreCreateCounting() API 功能原型*


**xSemaphoreCreateCounting()参数及返回值**

- `uxMaxCount`

信号量计数的最大值。继续
   与队列类比，`uxMaxCount` 值实际上是队列的长度
   队列。

当信号量用于计数或锁存事件时，`uxMaxCount`
   是可以锁存的最大事件数。

当信号量用于管理对集合的访问时
   资源数，`uxMaxCount` 应设置为资源总数
   可用的。

- `uxInitialCount`

信号量创建后的初始计数值。

当信号量用于计数或锁存事件时，
  `uxInitialCount` 应设置为零（因为当信号量
  创建后我们假设还没有发生任何事件）。

当信号量用于管理对集合的访问时
  资源，`uxInitialCount` 应设置为等于 `uxMaxCount`（因为
  创建信号量时，我们假设所有资源都可用）。

- 返回值

如果返回 NULL，则无法创建信号量，因为
  FreeRTOS 没有足够的堆内存来分配
  信号量数据结构。第 3 章提供有关堆的更多信息
  内存管理。

如果返回非NULL值，则表明该信号量已被
  创建成功。返回值应存储为句柄
  到创建的信号量。


<a name="example7.2" title="Example 7.2 Using a counting semaphore to synchronize a task with an interrupt"></a>
---
***示例 7.2*** *使用计数信号量使任务与中断同步*

---

例 7.2 通过使用计数改进了例 7.1 的实现
信号量代替二进制信号量。 `main()` 更改为包括
调用 `xSemaphoreCreateCounting()` 代替调用
`xSemaphoreCreateBinary()`。新的 API 调用如清单 7.12 所示。


<a name="list7.12" title="Listing 7.12 The call to xSemaphoreCreateCounting() used to create the counting semaphore in Example 7.2"></a>

```c
/* Before a semaphore is used it must be explicitly created. In this example a
   counting semaphore is created. The semaphore is created to have a maximum
   count value of 10, and an initial count value of 0. */
xCountingSemaphore = xSemaphoreCreateCounting( 10, 0 );
```
***清单 7.12*** *对 xSemaphoreCreateCounting() 的调用用于创建示例 7.2 中的计数信号量*


为了模拟高频发生的多个事件，中断
服务例程被更改为“给予”信号量一次以上
中断。每个事件都被锁存在信号量的计数值中。的
修改后的中断服务程序如清单 7.13 所示。


<a name="list7.13" title="Listing 7.13 The implementation of the interrupt service routine used by Example 7.2"></a>

```c
static uint32_t ulExampleInterruptHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken;

    /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE
       as it will get set to pdTRUE inside the interrupt safe API function if
       a context switch is required. */
    xHigherPriorityTaskWoken = pdFALSE;

    /* 'Give' the semaphore multiple times. The first will unblock the deferred
       interrupt handling task, the following 'gives' are to demonstrate that
       the semaphore latches the events to allow the task to which interrupts
       are deferred to process them in turn, without events getting lost. This
       simulates multiple interrupts being received by the processor, even
       though in this case the events are simulated within a single interrupt
       occurrence. */
    xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );
    xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );
    xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );

    /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR().
       If xHigherPriorityTaskWoken was set to pdTRUE inside
       xSemaphoreGiveFromISR() then calling portYIELD_FROM_ISR() will request
       a context switch. If xHigherPriorityTaskWoken is still pdFALSE then
       calling portYIELD_FROM_ISR() will have no effect. Unlike most FreeRTOS
       ports, the Windows port requires the ISR to return a value - the return
       statement is inside the Windows version of portYIELD_FROM_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```
***清单 7.13*** *例 7.2 使用的中断服务程序的实现*

所有其他函数与示例 7.1 中使用的函数保持不变。

执行例 7.2 时产生的输出如图 7.9 所示。
可以看出，中断处理被推迟到的任务
每次中断时处理所有 three (simulated) 事件
生成的。事件被锁存到信号量的计数值中，
允许任务依次处理它们。


<a name="fig7.9" title="Figure 7.9 The output produced when Example 7.2 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image56.jpg)
***图 7.9*** *执行例 7.2 时产生的输出*

* * *


## 7.6 将工作推迟到 RTOS 守护进程任务

到目前为止提出的延迟中断处理示例需要
应用程序编写者为每个使用中断的中断创建一个任务
延迟处理技术。也可以使用
`xTimerPendFunctionCallFromISR()`[^19] API 延迟中断函数
处理 RTOS 守护进程任务，这样就无需创建
每个中断都有单独的任务。将中断处理推迟到
守护进程任务被称为“集中式延迟中断处理”。

[^19]：守护任务原本是
之所以称为定时服务任务，是因为它本来只是用来
执行软件定时器回调函数。因此，
`xTimerPendFunctionCall()` 在 timers.c 中实现，并且，
符合函数名称前缀的约定
实现该函数的文件的名称，
函数的名称以“Timer”为前缀。

第 6 章介绍了软件定时器相关的 FreeRTOS API 的功能
将命令发送到计时器命令队列上的守护程序任务。的
`xTimerPendFunctionCall()` 和 `xTimerPendFunctionCallFromISR()` API
函数使用相同的计时器命令队列来发送“执行函数”
命令到守护进程任务。发送到守护任务的函数是
在守护进程任务的上下文中执行。

集中式延迟中断处理的优点包括：

- 降低资源使用率

它消除了为每个延迟创建单独任务的需要
  中断。

- 简化的用户模型

延迟中断处理函数是标准 C 函数。

集中式延迟中断处理的缺点包括：

- 灵活性较差

无法设置每个延迟中断的优先级
  分别处理任务。每个延迟中断处理函数
  以守护进程任务的优先级执行。如章节中所述
  6、守护进程任务的优先级由
  `configTIMER_TASK_PRIORITY` 编译时配置常量
  FreeRTOSConfig.h。

- 减少决定论

`xTimerPendFunctionCallFromISR()` 发送命令到后面
  定时器命令队列。定时器命令中已有的命令
  队列将在“执行”之前由守护进程任务处理
  function'命令发送到队列
  `xTimerPendFunctionCallFromISR()`。

不同的中断有不同的时序约束，所以很常见
在同一个进程中使用两种延迟中断处理的方法
应用程序。


### 7.6.1 xTimerPendFunctionCallFromISR() API 功能

`xTimerPendFunctionCallFromISR()` 是中断安全版本
`xTimerPendFunctionCall()`。 API 两个功能都允许提供一个功能
由应用程序编写者执行，因此在
RTOS 守护进程任务的上下文。要执行的函数，以及
函数的输入参数的值被发送到守护进程
定时器命令队列上的任务。函数实际执行的时间是
因此取决于守护进程任务相对于其他任务的优先级
应用程序中的任务。


<a name="list7.14" title="Listing 7.14 The xTimerPendFunctionCallFromISR() API function prototype"></a>

```c
BaseType_t xTimerPendFunctionCallFromISR( PendedFunction_t
                                          xFunctionToPend,
                                          void *pvParameter1,
                                          uint32_t ulParameter2,
                                          BaseType_t *pxHigherPriorityTaskWoken );
```
***清单 7.14*** *xTimerPendFunctionCallFromISR() API 功能原型*


<a name="list7.15" title="Listing 7.15 The prototype to which a function passed in the xFunctionToPend parameter of xTimerPendFunctionCallFromISR()..."></a>

```c
void vPendableFunction( void *pvParameter1, uint32_t ulParameter2 );
```
***清单 7.15*** *传入 xTimerPendFunctionCallFromISR() 的 xFunctionToPend 参数的函数必须符合的原型*


**xTimerPendFunctionCallFromISR()参数及返回值**

- `xFunctionToPend`

指向将在守护程序 task
  (in effect, just the function name) 中执行的函数的指针。函数的原型必须
  与清单 7.15 中所示的相同。

- `pvParameter1`

将传递给执行函数的值
  守护进程任务作为该函数的 `pvParameter1` 参数。参数
  具有 `void *` 类型，允许它用于传递任何数据类型。对于
  例如，整数类型可以直接转换为 `void *`，或者
  `void *` 可用于指向结构。

- `ulParameter2`

将传递给执行函数的值
  守护进程任务作为该函数的 `ulParameter2` 参数。

- `pxHigherPriorityTaskWoken`

`xTimerPendFunctionCallFromISR()` 写入定时器命令
  队列。如果RTOS守护任务处于Blocked状态等待数据
  在定时器命令队列上变得可用，然后写入
  定时器命令队列将导致守护任务离开阻塞状态
  状态。如果守护任务的优先级高于
  当前正在执行的 task (the task that was interrupted)，那么，
  在内部，`xTimerPendFunctionCallFromISR()` 将设置
  `*pxHigherPriorityTaskWoken` 至 `pdTRUE`。

如果 `xTimerPendFunctionCallFromISR()` 将此值设置为 `pdTRUE`，则
  上下文切换必须在中断退出之前执行。这个
  将确保中断直接返回到守护程序任务，如下所示
  守护任务将是最高优先级的就绪状态任务。

- 返回值

有两种可能的返回值：

- `pdPASS`

如果写入“执行函数”命令，将返回 `pdPASS`
    到定时器命令队列。

- `pdFAIL`

如果“执行函数”命令无法执行，将返回 `pdFAIL`
    被写入定时器命令队列，因为定时器命令队列
    已经满了。第6章介绍如何设置定时器的时长
    命令队列。


<a name="example7.3" title="Example 7.3 Centralized deferred interrupt processing"></a>
---
***示例 7.3*** *集中式延迟中断处理*

---

例 7.3 提供了与例 7.1 类似的功能，但没有
使用信号量，并且无需创建专门执行的任务
中断所需的处理。相反，处理过程是
由 RTOS 守护程序任务执行。

例 7.3 使用的中断服务程序如清单 7.16 所示。
它调用 `xTimerPendFunctionCallFromISR()` 将指针传递给
将调用 `vDeferredHandlingFunction()` 的函数添加到守护进程任务中。的
延迟中断处理是由
`vDeferredHandlingFunction()` 函数。

中断服务程序增加一个名为
每次执行时`ulParameterValue`。 `ulParameterValue` 用作
调用 `xTimerPendFunctionCallFromISR()` 时 `ulParameter2` 的值，因此
也将在调用中用作 `ulParameter2` 的值
执行 `vDeferredHandlingFunction()` 时为 `vDeferredHandlingFunction()`
通过守护进程任务。该函数的其他参数 `pvParameter1` 不是
本例中使用。


<a name="list7.16" title="Listing 7.16 The software interrupt handler used in Example 7.3"></a>

```c
static uint32_t ulExampleInterruptHandler( void )
{
    static uint32_t ulParameterValue = 0;
    BaseType_t xHigherPriorityTaskWoken;

    /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE
       as it will get set to pdTRUE inside the interrupt safe API function if
       a context switch is required. */
    xHigherPriorityTaskWoken = pdFALSE;

    /* Send a pointer to the interrupt's deferred handling function to the
       daemon task. The deferred handling function's pvParameter1 parameter
       is not used so just set to NULL. The deferred handling function's
       ulParameter2 parameter is used to pass a number that is incremented by
       one each time this interrupt handler executes. */
    xTimerPendFunctionCallFromISR( vDeferredHandlingFunction, /* Function to execute */
                                   NULL, /* Not used */
                                   ulParameterValue, /* Incrementing value. */
                                   &xHigherPriorityTaskWoken );
    ulParameterValue++;

    /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR(). If
       xHigherPriorityTaskWoken was set to pdTRUE inside
       xTimerPendFunctionCallFromISR() then calling portYIELD_FROM_ISR() will
       request a context switch. If xHigherPriorityTaskWoken is still pdFALSE
       then calling portYIELD_FROM_ISR() will have no effect. Unlike most
       FreeRTOS ports, the Windows port requires the ISR to return a value -
       the return statement is inside the Windows version
       of portYIELD_FROM_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```
***清单 7.16*** *示例 7.3 中使用的软件中断处理程序*


`vDeferredHandlingFunction()`的实现如清单所示
7.17。它打印出一个固定的字符串，及其`ulParameter2`的值
参数。

`vDeferredHandlingFunction()` 必须具有清单中所示的原型
7.15，尽管在这个例子中，只有一个参数是
实际使用过。


<a name="list7.17" title="Listing 7.17 The function that performs the processing necessitated by the interrupt in Example 7.3"></a>

```c
static void vDeferredHandlingFunction( void *pvParameter1, uint32_t ulParameter2 )
{
    /* Process the event - in this case just print out a message and the value
       of ulParameter2. pvParameter1 is not used in this example. */
    vPrintStringAndNumber( "Handler function - Processing event ", ulParameter2 );
}
```
***清单 7.17*** *执行例 7.3 中中断所需处理的函数*


例 7.3 使用的 `main()` 函数如清单 7.18 所示。它是
比例 7.1 使用的 `main()` 函数更简单，因为它不
创建信号量或任务来执行延迟中断
处理。

`vPeriodicTask()`是周期性生成软件的任务
中断。它的创建优先级低于
守护进程任务，以确保它一旦被守护进程任务抢占
守护进程任务离开阻塞状态。


<a name="list7.18" title="Listing 7.18 The implementation of main() for Example 7.3"></a>

```c
int main( void )
{
    /* The task that generates the software interrupt is created at a priority
       below the priority of the daemon task. The priority of the daemon task
       is set by the configTIMER_TASK_PRIORITY compile time configuration
       constant in FreeRTOSConfig.h. */
    const UBaseType_t ulPeriodicTaskPriority = configTIMER_TASK_PRIORITY - 1;

    /* Create the task that will periodically generate a software interrupt. */
    xTaskCreate( vPeriodicTask, "Periodic", 1000, NULL, ulPeriodicTaskPriority,
                 NULL );

    /* Install the handler for the software interrupt. The syntax necessary to
       do this is dependent on the FreeRTOS port being used. The syntax shown
       here can only be used with the FreeRTOS windows port, where such
       interrupts are only simulated. */
    vPortSetInterruptHandler( mainINTERRUPT_NUMBER, ulExampleInterruptHandler );

    /* Start the scheduler so the created task starts executing. */
    vTaskStartScheduler();

    /* As normal, the following line should never be reached. */
    for( ;; );
}
```
***清单 7.18*** *示例 7.3 中 main() 的实现*


例 7.3 产生如图 7.10 所示的输出。的优先级
守护任务的优先级高于生成该任务的任务的优先级
软件中断，因此`vDeferredHandlingFunction()`由
一旦中断产生，守护进程就会执行任务。这导致
`vDeferredHandlingFunction()` 输出的消息出现在
周期性任务输出两条消息，就像
信号量用于解锁专用的延迟中断处理
任务。图 7.11 提供了进一步的解释。


<a name="fig7.10" title="Figure 7.10 The output produced when Example 7.3 is executed"></a>
<a name="fig7.11" title="Figure 7.11 The sequence of execution when Example 7.3 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image57.jpg)
***图 7.10*** *执行例 7.3 时产生的输出*

![](https://cloud.rocketpi.club/cloud/image58.png)
***图7.11*** *执行例7.3时的执行顺序*

* * *


## 7.7 在中断服务例程中使用队列

二进制和计数信号量用于传达事件。队列
用于通信事件和传输数据。

`xQueueSendToFrontFromISR()` 是 `xQueueSendToFront()` 的版本
可在中断服务程序中安全使用，`xQueueSendToBackFromISR()`
是可以在中断中安全使用的 `xQueueSendToBack()` 版本
服务例程，`xQueueReceiveFromISR()` 是
`xQueueReceive()` 可在中断服务程序中安全使用。


### 7.7.1 xQueueSendToFrontFromISR() 和 xQueueSendToBackFromISR() API 功能


<a name="list7.19" title="Listing 7.19 The xQueueSendToFrontFromISR() API function prototype"></a>

```c
BaseType_t xQueueSendToFrontFromISR( QueueHandle_t xQueue,
                                     const void *pvItemToQueue
                                     BaseType_t *pxHigherPriorityTaskWoken );
```
***清单 7.19*** *xQueueSendToFrontFromISR() API 功能原型*


<a name="list7.20" title="Listing 7.20 The xQueueSendToBackFromISR() API function prototype"></a>

```c
BaseType_t xQueueSendToBackFromISR( QueueHandle_t xQueue,
                                    const void *pvItemToQueue
                                    BaseType_t *pxHigherPriorityTaskWoken );
```
***清单 7.20*** *xQueueSendToBackFromISR() API 功能原型*


`xQueueSendFromISR()` 和 `xQueueSendToBackFromISR()` 功能相同。

**xQueueSendToFrontFromISR() 和 xQueueSendToBackFromISR() 参数和返回值**

- `xQueue`

数据所在队列的句柄为 sent (written)。
  队列句柄将从对 `xQueueCreate()` 的调用中返回
  用于创建队列。

- `pvItemToQueue`

指向要放入队列的项目的指针。

队列将保存的每个项目的大小是在队列创建时定义的。
  创建，因此这么多字节将从 `pvItemToQueue` 复制到
  队列存储区。

- `pxHigherPriorityTaskWoken`

单个队列可能有一个或多个任务
  阻塞在它上面，等待数据变得可用。呼唤
  `xQueueSendToFrontFromISR()`或`xQueueSendToBackFromISR()`可以制作数据
  可用，从而导致此类任务离开阻塞状态。如果
  调用 API 函数会导致任务离开阻塞状态，并且
  未阻塞任务的优先级高于当前正在执行的任务
  task (the task that was interrupted)，那么，在内部，API 函数
  将把 `*pxHigherPriorityTaskWoken` 设置为 `pdTRUE`。

如果 `xQueueSendToFrontFromISR()` 或 `xQueueSendToBackFromISR()` 设置此
  值为 `pdTRUE`，则应在之前执行上下文切换
  中断退出。这将确保中断返回
  直接执行最高优先级的就绪状态任务。

- 返回值

有两种可能的返回值：

- `pdPASS`

仅当数据已成功发送到队列时，才会返回 `pdPASS`。

- `errQUEUE_FULL

如果由于以下原因无法将数据发送到队列，则返回 `errQUEUE_FULL`
    队列已经满了。


### 7.7.2 使用 ISR 队列时的注意事项

队列提供了一种简单方便的方式来传递数据
中断任务，但如果数据较多，使用队列效率不高
以高频率到达。

FreeRTOS 下载中的许多演示应用程序都包含一个简单的
UART 驱动程序使用队列将字符传出 UART
收到 ISR。在这些演示中，使用队列有两个原因：
演示从 ISR 使用的队列，并故意加载
系统以测试 FreeRTOS 端口。使用队列的 ISR
这种方式绝对不是为了代表一种有效的
设计，除非数据到达缓慢，否则建议
生产代码不会复制此技术。更高效的技术，
适合生产代码的包括：

- 使用Direct Memory Access (DMA)硬件来接收和缓冲
  字符。该方法实际上没有软件开销。一个
  然后可以使用直接任务通知[^20]来解锁
  仅在传输中断后才处理缓冲区的任务
  已检测到。

[^20]：直接任务通知提供了最有效的方法
  从 ISR 解锁任务。直接任务通知是
  第 10 章“任务通知”中介绍了这一点。

- 将每个接收到的字符复制到线程安全的 RAM 缓冲区中[^21]。
  同样，可以使用直接任务通知来解锁任务
  它将在完整的消息被处理后处理缓冲区
  已收到，或检测到传输中断后。

[^21]：“流缓冲区”作为 FreeRTOS+TCP
  ([https://www.FreeRTOS.org/tcp](http://www.FreeRTOS.org/tcp)) 的一部分提供，可以
  用于此目的。

- 直接在ISR内处理接收到的字符，然后
  使用队列仅发送处理数据的结果（而不是
  比原始数据）到任务。先前已证明这一点
  图 5.4。

<a name="example7.4" title="Example 7.4 Sending and receiving on a queue from within an interrupt"></a>
---
***示例 7.4*** *在中断内通过队列发送和接收*

---

此示例演示 `xQueueSendToBackFromISR()` 和
`xQueueReceiveFromISR()` 在同一中断中使用。和以前一样，
为了方便起见，中断由软件生成。

创建一个周期性任务，每 200 个向队列发送 5 个数字
毫秒。仅在所有五个之后才生成软件中断
值已发送。任务实现如清单 7.21 所示。


<a name="list7.21" title="Listing 7.21 The implementation of the task that writes to the queue in Example 7.4"></a>

```c
static void vIntegerGenerator( void *pvParameters )
{
    TickType_t xLastExecutionTime;
    uint32_t ulValueToSend = 0;
    int i;

    /* Initialize the variable used by the call to vTaskDelayUntil(). */
    xLastExecutionTime = xTaskGetTickCount();

    for( ;; )
    {
        /* This is a periodic task. Block until it is time to run again. The
           task will execute every 200ms. */
        vTaskDelayUntil( &xLastExecutionTime, pdMS_TO_TICKS( 200 ) );

        /* Send five numbers to the queue, each value one higher than the
           previous value. The numbers are read from the queue by the interrupt
           service routine. The interrupt service routine always empties the
           queue, so this task is guaranteed to be able to write all five
           values without needing to specify a block time. */
        for( i = 0; i < 5; i++ )
        {
            xQueueSendToBack( xIntegerQueue, &ulValueToSend, 0 );
            ulValueToSend++;
        }

        /* Generate the interrupt so the interrupt service routine can read the
           values from the queue. The syntax used to generate a software
           interrupt is dependent on the FreeRTOS port being used. The syntax
           used below can only be used with the FreeRTOS Windows port, in which
           such interrupts are only simulated. */
        vPrintString( "Generator task - About to generate an interrupt.\r\n" );
        vPortGenerateSimulatedInterrupt( mainINTERRUPT_NUMBER );
        vPrintString( "Generator task - Interrupt generated.\r\n\r\n\r\n" );
    }
}
```
***清单7.21*** *例7.4中写入队列的任务的实现*


中断服务程序重复调用`xQueueReceiveFromISR()`
直到周期性任务写入队列的所有值都已完成
读出，队列为空。每个的最后两位
接收到的值用作字符串数组的索引。一个指针
然后将相应索引位置处的字符串发送到
使用对 `xQueueSendFromISR()` 的调用来设置不同的队列。实施情况
中断服务程序的框图如清单 7.22 所示。


<a name="list7.22" title="Listing 7.22 The implementation of the interrupt service routine used by Example 7.4"></a>

```c
static uint32_t ulExampleInterruptHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken;
    uint32_t ulReceivedNumber;

    /* The strings are declared static const to ensure they are not allocated
       on the interrupt service routine's stack, and so exist even when the
       interrupt service routine is not executing. */

    static const char *pcStrings[] =
    {
        "String 0\r\n",
        "String 1\r\n",
        "String 2\r\n",
        "String 3\r\n"
    };

    /* As always, xHigherPriorityTaskWoken is initialized to pdFALSE to be
       able to detect it getting set to pdTRUE inside an interrupt safe API
       function.  Note that as an interrupt safe API function can only set
       xHigherPriorityTaskWoken to pdTRUE, it is safe to use the same
       xHigherPriorityTaskWoken variable in both the call to
       xQueueReceiveFromISR() and the call to xQueueSendToBackFromISR(). */
    xHigherPriorityTaskWoken = pdFALSE;

    /* Read from the queue until the queue is empty. */
    while( xQueueReceiveFromISR( xIntegerQueue,
                                 &ulReceivedNumber,
                                 &xHigherPriorityTaskWoken ) != errQUEUE_EMPTY )
    {
        /* Truncate the received value to the last two bits (values 0 to 3
           inclusive), then use the truncated value as an index into the
           pcStrings[] array to select a string (char *) to send on the other
           queue. */
        ulReceivedNumber &= 0x03;
        xQueueSendToBackFromISR( xStringQueue,
                                 &pcStrings[ ulReceivedNumber ],
                                 &xHigherPriorityTaskWoken );
    }

    /* If receiving from xIntegerQueue caused a task to leave the Blocked
       state, and if the priority of the task that left the Blocked state is
       higher than the priority of the task in the Running state, then
       xHigherPriorityTaskWoken will have been set to pdTRUE inside
       xQueueReceiveFromISR().

       If sending to xStringQueue caused a task to leave the Blocked state, and
       if the priority of the task that left the Blocked state is higher than
       the priority of the task in the Running state, then
       xHigherPriorityTaskWoken will have been set to pdTRUE inside
       xQueueSendToBackFromISR().

       xHigherPriorityTaskWoken is used as the parameter to portYIELD_FROM_ISR().
       If xHigherPriorityTaskWoken equals pdTRUE then calling portYIELD_FROM_ISR()
       will request a context switch. If xHigherPriorityTaskWoken is still
       pdFALSE then calling portYIELD_FROM_ISR() will have no effect.

       The implementation of portYIELD_FROM_ISR() used by the Windows port
       includes a return statement, which is why this function does not
       explicitly return a value. */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
```
***清单 7.22*** *例 7.4 使用的中断服务程序的实现*


从中断服务接收字符指针的任务
例程会阻塞队列，直到消息到达为止，打印出每个消息
收到时的字符串。其实现如清单 7.23 所示。


<a name="list7.23" title="Listing 7.23 The task that prints out the strings received from the interrupt service routine in Example 7.4"></a>

```c
static void vStringPrinter( void *pvParameters )
{
    char *pcString;

    for( ;; )
    {
        /* Block on the queue to wait for data to arrive. */
        xQueueReceive( xStringQueue, &pcString, portMAX_DELAY );

        /* Print out the string received. */
        vPrintString( pcString );
    }
}
```
***清单 7.23*** *打印从例 7.4 中的中断服务程序接收到的字符串的任务*

正常情况下，`main()` 在启动之前创建所需的队列和任务
调度程序。其实现如清单 7.24 所示。


<a name="list7.24" title="Listing 7.24 The main() function for Example 7.4"></a>

```c
int main( void )
{
    /* Before a queue can be used it must first be created. Create both queues
       used by this example. One queue can hold variables of type uint32_t, the
       other queue can hold variables of type char*. Both queues can hold a
       maximum of 10 items. A real application should check the return values
       to ensure the queues have been successfully created. */
    xIntegerQueue = xQueueCreate( 10, sizeof( uint32_t ) );
    xStringQueue = xQueueCreate( 10, sizeof( char * ) );

    /* Create the task that uses a queue to pass integers to the interrupt
       service routine. The task is created at priority 1. */
    xTaskCreate( vIntegerGenerator, "IntGen", 1000, NULL, 1, NULL );

    /* Create the task that prints out the strings sent to it from the
       interrupt service routine. This task is created at the higher
       priority of 2. */
    xTaskCreate( vStringPrinter, "String", 1000, NULL, 2, NULL );

    /* Install the handler for the software interrupt. The syntax necessary to
       do this is dependent on the FreeRTOS port being used. The syntax shown
       here can only be used with the FreeRTOS Windows port, where such
       interrupts are only simulated. */
    vPortSetInterruptHandler( mainINTERRUPT_NUMBER, ulExampleInterruptHandler );

    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();

    /* If all is well then main() will never reach here as the scheduler will
       now be running the tasks. If main() does reach here then it is likely
       that there was insufficient heap memory available for the idle task
       to be created. Chapter 2 provides more information on heap memory
       management. */
    for( ;; );
}
```
***清单 7.24*** *示例 7.4 的 main() 函数*

执行例 7.4 时产生的输出如图 7.12 所示。
可以看出，中断接收所有五个整数，并产生
五个字符串作为响应。图 7.13 给出了更多解释。


<a name="fig7.12" title="Figure 7.12 The output produced when Example 7.4 is executed"></a>
<a name="fig7.13" title="Figure 7.13 The sequence of execution produced by Example 7.4"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image59.jpg)
***图 7.12*** *执行例 7.4 时产生的输出*

![](https://cloud.rocketpi.club/cloud/image60.png)
***图 7.13*** *示例 7.4 产生的执行顺序*

* * *


## 7.8 中断嵌套

任务优先级和任务优先级之间出现混淆是很常见的。
中断优先级。本节讨论中断优先级，其中
是中断服务 routines (ISRs) 执行的优先级
相对于彼此。分配给任务的优先级绝不是
与分配给中断的优先级有关。硬件决定何时
ISR 将执行，而软件决定任务何时执行。
响应硬件中断而执行的 ISR 将中断
任务，但任务不能抢占 ISR。

支持中断嵌套的端口需要其中之一或两者
下面详述的常量在 FreeRTOSConfig.h 中定义。
`configMAX_SYSCALL_INTERRUPT_PRIORITY` 和
`configMAX_API_CALL_INTERRUPT_PRIORITY` 都定义相同的属性。
较旧的 FreeRTOS 端口使用 `configMAX_SYSCALL_INTERRUPT_PRIORITY`，较新的 FreeRTOS 端口使用 `configMAX_SYSCALL_INTERRUPT_PRIORITY`
FreeRTOS 端口使用 `configMAX_API_CALL_INTERRUPT_PRIORITY`。

**控制中断嵌套的常量**

- `configMAX_SYSCALL_INTERRUPT_PRIORITY` 或 `configMAX_API_CALL_INTERRUPT_PRIORITY`

设置中断安全的最高中断优先级
  可以调用FreeRTOS API函数。

- `configKERNEL_INTERRUPT_PRIORITY`

设置tick中断使用的中断优先级，并且必须
  始终设置为尽可能最低的中断优先级。

如果正在使用的 FreeRTOS 端口未同时使用
  `configMAX_SYSCALL_INTERRUPT_PRIORITY` 常量，则任何中断
  使用中断安全的 FreeRTOS API 函数也必须在
  优先级由 `configKERNEL_INTERRUPT_PRIORITY` 定义。

每个中断源都有一个数字优先级和一个逻辑优先级：

- 数字优先

数字优先级只是分配给中断的编号
  优先。例如，如果为中断分配优先级为 7，
  那么它的数字优先级是 7。同样，如果分配了一个中断
  优先级为 200，则其数字优先级为 200。

- 逻辑优先级

中断的逻辑优先级描述了该中断的优先级
  超过其他中断。

如果两个不同优先级的中断同时发生，则
  处理器将为两个中断中的任何一个执行 ISR
  在执行 ISR 之前具有较高的逻辑优先级
  两个中断中哪个具有较低的逻辑优先级。

interrupt (nest with) 任何具有较低中断的中断
  逻辑优先级，但中断不能 interrupt (nest with) 任何
  具有相同或更高逻辑优先级的中断。

中断的数字优先级和逻辑优先级之间的关系
优先级取决于处理器架构；在某些处理器上，
分配给中断的数字优先级越高*越高*
该中断的逻辑优先级将是，而在其他处理器上
架构中分配给中断的数字优先级越高
中断逻辑优先级的*低*。

通过设置创建完整的中断嵌套模型
`configMAX_SYSCALL_INTERRUPT_PRIORITY` 为更高逻辑中断
优先级高于 `configKERNEL_INTERRUPT_PRIORITY`。这体现在
图 7.14 显示了一个场景：

- 处理器有七个独特的中断优先级。
- 分配数字优先级为 7 的中断具有更高的逻辑优先级
  优先级高于分配数字优先级 1 的中断。
- `configKERNEL_INTERRUPT_PRIORITY` 设置为 1。
- `configMAX_SYSCALL_INTERRUPT_PRIORITY` 设置为 3。


<a name="fig7.14" title="Figure 7.14 Constants affecting interrupt nesting behavior"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image61.png)
***图 7.14*** *影响中断嵌套行为的常量*

* * *

参见图7.14：

- 使用优先级 1 到 3（含）的中断被阻止
  当内核或应用程序位于关键区域内时执行
  部分。以这些优先级运行的 ISR 可以使用中断安全
  FreeRTOS API 功能。第 8 章描述了关键部分。

- 使用优先级 4 或更高优先级的中断不受
  关键部分，因此调度程序所做的任何事情都不会阻止这些
  立即中断执行——在限制范围内
  硬件本身。在这些优先级上执行的 ISR 不能使用任何
  FreeRTOS API 功能。

- 通常，需要非常严格计时的功能 accuracy
  (motor control, for example) 将使用上述优先级
  `configMAX_SYSCALL_INTERRUPT_PRIORITY` 确保调度程序执行
  不将抖动引入中断响应时间。


### 7.8.1 ARM Cortex-M[^22] 和 ARM GIC 用户注意事项

[^22]：本节仅部分适用于 Cortex-M0 和 Cortex-M0+ 内核。

Cortex-M 处理器上的中断配置很混乱且容易发生
到错误。为了协助您的开发，FreeRTOS Cortex-M 端口
自动检查中断配置，但前提是
`configASSERT()` 已定义。 `configASSERT()` 在第 11.2 节中进行了描述。

ARM Cortex 内核和 ARM 通用中断 Controllers (GICs) 使用
数字*低*优先级数字代表逻辑*高*
优先级中断。这似乎违反直觉，并且很容易
忘记了。如果您希望为中断分配逻辑上较低的优先级，
那么它必须被赋予一个数值较高的值。如果您想分配
一个中断具有逻辑上的高优先级，那么它必须被分配一个
数值较低。

Cortex-M 中断控制器允许最多 8 位
用于指定每个中断优先级，使 255 成为可能的最低优先级
优先。零是最高优先级。然而，Cortex-M
微控制器通常只实现八种可能的子集
位。实际实现的位数取决于
微控制器系列。

当仅实现了八个可能位的子集时，
仅是可以使用的字节的最高有效位 - 留下
最低有效位未实现。未实现的位可以占用
任何值，但将它们设置为 1 是正常的。这可以通过以下方式证明
图 7.15 显示了二进制 101 的优先级如何存储在
Cortex-M 微控制器实现了四个优先级位。


<a name="fig7.15" title="Figure 7.15 How a priority of binary 101 is stored by a Cortex-M microcontroller that implements four priority bits"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image62.png)
***图 7.15*** *实现四个优先级位的 Cortex-M 微控制器如何存储二进制 101 的优先级*
* * *

在图 7.15 中，二进制值 101 已被移入最
有效四位，因为最低有效四位不是
已实施。未实现的位已设置为 1。

某些库函数希望在调用后指定优先级值
已被上移至 implemented (most significant) 位。当
使用这样的函数，可以指定如图7.15所示的优先级
如十进制 95。十进制 95 是二进制 101 上移 4 得到的
二进制 101nnnn （其中“n”是未实现的位），并且
未实现的位设置为 1 以生成二进制 1011111。

一些库函数期望在之前指定优先级值
它们已被上移至 implemented (most significant) 位。
当使用这样的功能时，图 7.15 中所示的优先级必须是
指定为十进制 5。十进制 5 是没有任何移位的二进制 101。

`configMAX_SYSCALL_INTERRUPT_PRIORITY` 和 `configKERNEL_INTERRUPT_PRIORITY`
必须以允许它们直接写入的方式指定
Cortex-M 寄存器，因此优先级值移位后
直到已实现的位。

`configKERNEL_INTERRUPT_PRIORITY` 必须始终设置为最低
可能的中断优先级。未实现的优先级位可以设置为
1、所以常量总是可以设置为255，无论优先级是多少
位实际上已实现。

Cortex-M 中断的默认优先级为零——最高优先级
可能的优先级。 Cortex-M 硬件的实现并不
允许 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 设置为 0，因此
使用 FreeRTOS API 的中断的优先级绝不能被保留
为其默认值。
