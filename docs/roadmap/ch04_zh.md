# 4 任务管理

## 4.1 简介

### 4.1.1 范围

本章内容包括：

- FreeRTOS 如何为应用程序中的每个任务分配处理时间。
- FreeRTOS 如何选择在任何给定时间应执行哪个任务。
- 每个任务的相对优先级如何影响系统行为。
- 任务可以存在的状态。

本章还讨论：

- 如何实施任务。
- 如何创建任务的一个或多个实例。
- 如何使用任务参数。
- 如何更改已创建任务的优先级。
- 如何删除任务。
- 如何使用任务实现周期性处理。 （后面的章节
  描述了如何使用软件定时器执行相同的操作。）
- 空闲任务何时执行以及如何使用它。

本章中提出的概念是理解的基础
如何使用 FreeRTOS 以及 FreeRTOS 应用程序的行为方式。因此，
这是本书中最详细的一章。

## 4.2 任务功能

任务作为 C 函数实现。任务必须实现预期的功能
原型如清单 4.1 所示。它接受一个 void 指针参数并返回
无效。

<a name="list4.1" title="Listing 4.1 The task function prototype"></a>

```c
void vATaskFunction( void * pvParameters );
```

***清单 4.1*** *任务函数原型*


每个任务本身就是一个小程序。它有一个入口点，
通常会无限循环地永远运行，并且不会退出。
清单 4.2 显示了一个典型任务的结构。

不允许 FreeRTOS 任务从实现的函数返回
以任何方式。它不能包含“return”语句并且必须
不允许在其实现函数结束之后执行。
如果不再需要某个任务，则应将其显式删除
如清单 4.2 所示。

单个任务函数定义可用于创建任意数量的
每个创建的任务都是一个单独的执行实例的任务。每个实例都有
它自己的堆栈，以及它自己定义的任何 automatic (stack) 变量的副本
在任务本身之内。


<a name="list4.2" title="Listing 4.2 The structure of a typical task function"></a>


```c
void vATaskFunction( void * pvParameters )
{
    /*
     * Stack-allocated variables can be declared normally when inside a function.
     * Each instance of a task created using this example function will have its
     * own separate instance of lStackVariable allocated on the task's stack.
     */
    long lStackVariable = 0;

    /*
     * In contrast to stack allocated variables, variables declared with the `static`
     * keyword are allocated to a specific location in memory by the linker.
     * This means that all tasks calling vATaskFunction will share the same
     * instance of lStaticVariable.
     */
    static long lStaticVariable = 0;

    for( ;; )
    {
        /* The code to implement the task functionality will go here. */
    }

    /*
     * If the task implementation ever exits the above loop, then the task
     * must be deleted before reaching the end of its implementing function.
     * When NULL is passed as a parameter to the vTaskDelete() API function,
     * this indicates that the task to be deleted is the calling (this) task.
     */
    vTaskDelete( NULL );
}
```

***清单 4.2*** *典型任务函数的结构*

## 4.3 顶级任务状态

一个应用程序可能包含许多任务。如果处理器运行
应用程序包含单个核心，则只能执行一个任务
在任何给定时间。这意味着任务可能存在于以下两个之一中
状态：*正在运行*和*未运行*。这个简单的模型被认为是
首先。在本章后面，我们将描述几个子状态
*未运行*状态。

当处理器正在执行该任务的任务时，该任务处于“正在运行”状态
代码。当任务处于“未运行”状态时，该任务将暂停并且其
状态已被保存，以便调度程序下次可以恢复执行
决定它应该进入*运行*状态。当任务恢复执行时，
它从离开之前即将执行的指令开始执行此操作
*运行*状态。


<a name="fig4.1" title="Figure 4.1 Top level task states and transitions"></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.1_top_level_task_states.png)
***图 4.1*** *顶级任务状态和转换*

***

从“未运行”状态转换为“正在运行”状态的任务是
据说已“转入”或“交换”。相反，一个任务
从*正在运行*状态转换到*未运行*状态据说是
已被“转出”或“换出”。 FreeRTOS 调度程序是
唯一可以将任务切换到*运行*状态的实体。

## 4.4 任务创建

六个 API 函数可用于创建任务：
`xTaskCreate()`,
`xTaskCreateStatic()`,
`xTaskCreateRestricted()`,
`xTaskCreateRestrictedStatic()`,
`xTaskCreateAffinitySet()`，和
`xTaskCreateStaticAffinitySet()`

每个任务需要两个 RAM 块：一个用于保存其任务控制 Block (TCB)
和一个来存储其堆栈。 FreeRTOS API 函数名称中带有“Static”
使用预先分配的 RAM 块作为参数传递给函数。
相反，名称中不带“Static”的 API 函数会分配所需的
RAM 在运行时动态地从系统堆中获取。

某些 FreeRTOS 端口支持在“受限”或“非特权”模式下运行的任务。
名称中带有“Restricted”的 FreeRTOS API 函数创建的任务
以对系统内存的有限访问来执行。 API 功能无
名称中的“受限”创建以“特权模式”执行的任务，并且
可以访问系统的整个内存映射。

支持 Symmetric Multi Processing (SMP) 的 FreeRTOS 端口允许执行不同的任务
在同一 CPU 的多个内核上同时运行。对于这些端口，您可以
通过使用名称中带有“Affinity”的函数来指定任务将在哪个核心上运行。

FreeRTOS任务创建API函数相当复杂。本节中的大多数例子
文档使用 `xTaskCreate()` 因为它是这些函数中最简单的。

### 4.4.1 xTaskCreate() API 功能

清单 4.3 显示了 `xTaskCreate()` API 函数原型。
`xTaskCreateStatic()` 有两个附加参数指向
预先分配的内存用于保存任务的数据结构和堆栈，
分别。 [Section 2.5: Data Types and Coding Style Guide](ch02.md#25-data-types-and-coding-style-guide)
描述所使用的数据类型和命名约定。


<a name="list4.3" title="Listing 4.3 The xTaskCreate() API function prototype"></a>


```c
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
                        const char * const pcName,
                        configSTACK_DEPTH_TYPE usStackDepth,
                        void * pvParameters,
                        UBaseType_t uxPriority,
                        TaskHandle_t * pxCreatedTask );
```

***清单 4.3*** *xTaskCreate() API 函数原型*

**xTaskCreate() 参数及返回值：**

- `pvTaskCode`

任务只是永远不会退出的 C 函数，因此通常是
  作为无限循环实现。 `pvTaskCode` 参数只是一个
  指向实现任务的函数的指针（实际上，只是
  函数的名称）。

- `pcName`

任务的描述性名称。 FreeRTOS 不使用它
  无论如何，它纯粹是作为调试辅助工具包含在内。通过以下方式识别任务
  人类可读的名称比通过句柄识别要简单得多。

应用程序定义的常量 `configMAX_TASK_NAME_LEN` 定义
  任务名称的最大长度，包括 NULL 终止符。
  提供更长的字符串会导致字符串被截断。

- `usStackDepth`

指定分配给任务使用的堆栈大小。
  使用 `xTaskCreateStatic()` 而不是 `xTaskCreate()` 来使用预分配
  内存而不是动态分配的内存。

注意该值指定堆栈可以容纳的字数，而不是
  字节数。例如，如果堆栈是 32 位宽并且
  `usStackDepth`为128，则`xTaskCreate()`分配512字节的堆栈
  space (128 * 4 bytes)。

`configSTACK_DEPTH_TYPE` 是一个宏，允许应用程序编写者
  指定用于保存堆栈大小的数据类型。 `configSTACK_DEPTH_TYPE`
  如果未定义，则默认为 `uint16_t`，因此#define `configSTACK_DEPTH_TYPE`
  如果堆栈深度成倍增加，则为 `FreeRTOSConfig.h` 中的 `unsigned long` 或 `size_t`
  堆栈宽度大于 65535（最大可能的 16 位数字）。

[Section 13.3 Stack Overflow](ch13.md#133-stack-overflow)，描述了
  选择最佳堆栈大小的实用方法。

- `pvParameters`

实现任务的函数接受单个 void pointer (`void *`)
  参数。 `pvParameters` 是使用该值传递到任务中的值
  参数。

- `uxPriority`

定义任务的优先级。 0 是最低优先级并且
  `(configMAX_PRIORITIES – 1)` 是最高优先级。 [Section 4.5](#45-task-priorities)
  描述用户定义的 `configMAX_PRIORITIES` 常量。

如果定义了大于 `(configMAX_PRIORITIES – 1)` 的 `uxPriority`，则会
  上限为 `(configMAX_PRIORITIES – 1)`。

- `pxCreatedTask`

指向存储已创建任务句柄的位置的指针。这个手柄可以
  用于将来的 API 调用，例如更改任务的优先级或删除
  任务。

`pxCreatedTask` 是一个可选参数，如果
  不需要任务句柄。

- 返回值

有两种可能的返回值：

- `pdPASS`

这表明任务创建成功。

- `pdFAIL`

这表明没有足够的堆内存可用于创建
    任务。 [Chapter 3](ch03.md#3-heap-memory-management) 提供更多
    有关堆内存管理的信息。


<a name="example4.1" title="Example 4.1 Creating tasks"></a>
---
***示例 4.1*** *创建任务*

---

以下示例演示了创建两个简单任务所需的步骤
然后启动新创建的任务。这些任务只是打印出一个字符串
通过使用粗略的繁忙循环定期创建周期延迟。两者都
任务以相同的优先级创建，并且除了
它们打印出来的字符串 - 请参阅清单 4.4 和清单 4.5 了解它们各自的信息
实施。有关在以下环境中使用 `printf()` 的警告，请参阅第 8 章：
任务。

<a name="list4.4" title="Listing 4.4 Implementation of the first task used in Example 4.1"></a>


```c
void vTask1( void * pvParameters )
{
    /* ulCount is declared volatile to ensure it is not optimized out. */
    volatile unsigned long ulCount;

    for( ;; )
    {
        /* Print out the name of the current task task. */
        vPrintLine( "Task 1 is running" );

        /* Delay for a period. */
        for( ulCount = 0; ulCount < mainDELAY_LOOP_COUNT; ulCount++ )
        {
            /*
             * This loop is just a very crude delay implementation. There is
             * nothing to do in here. Later examples will replace this crude
             * loop with a proper delay/sleep function.
             */
        }
    }
}
```

***清单 4.4*** *示例 4.1 中使用的第一个任务的实现*


<a name="list4.5" title="Listing 4.5 Implementation of the second task used in Example 4.1"></a>


```c
void vTask2( void * pvParameters )
{
    /* ulCount is declared volatile to ensure it is not optimized out. */
    volatile unsigned long ulCount;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /* Print out the name of this task. */
        vPrintLine( "Task 2 is running" );

        /* Delay for a period. */
        for( ulCount = 0; ulCount < mainDELAY_LOOP_COUNT; ulCount++ )
        {
            /*
             * This loop is just a very crude delay implementation. There is
             * nothing to do in here. Later examples will replace this crude
             * loop with a proper delay/sleep function.
             */
        }
    }
}
```

***清单 4.5*** *示例 4.1 中使用的第二个任务的实现*

main() 函数在启动调度程序之前创建任务 - 请参阅
其实现见清单4.6。


<a name="list4.6" title="Listing 4.6 Starting the Example 4.1 tasks"></a>


```c
int main( void )
{
    /*
     * Variables declared here may no longer exist after starting the FreeRTOS
     * scheduler. Do not attempt to access variables declared on the stack used
     * by main() from tasks.
     */

    /*
     * Create one of the two tasks. Note that a real application should check
     * the return value of the xTaskCreate() call to ensure the task was
     * created successfully.
     */
    xTaskCreate( vTask1,  /* Pointer to the function that implements the task.*/
                 "Task 1",/* Text name for the task. */
                 1000,    /* Stack depth in words. */
                 NULL,    /* This example does not use the task parameter. */
                 1,       /* This task will run at priority 1. */
                 NULL );  /* This example does not use the task handle. */

    /* Create the other task in exactly the same way and at the same priority.*/
    xTaskCreate( vTask2, "Task 2", 1000, NULL, 1, NULL );

    /* Start the scheduler so the tasks start executing. */
    vTaskStartScheduler();

    /*
     * If all is well main() will not reach here because the scheduler will now
     * be running the created tasks. If main() does reach here then there was
     * not enough heap memory to create either the idle or timer tasks
     * (described later in this book). Chapter 3 provides more information on
     * heap memory management.
     */
    for( ;; );
}
```

***清单 4.6*** *启动示例 4.1 任务*

执行该示例会产生如图 4.2 所示的输出。


<a name="fig4.2" title="Figure 4.2 The output produced when executing Example 4.1"></a>

***

```console
C:\Temp>rtosdemo
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
```

***图 4.2*** *执行示例 4.1 时产生的输出[^4]*

***

[^4]：屏幕截图显示每个任务准确地打印出其消息
在下一个任务执行之前执行一次。这是一个人为的场景
这是使用 FreeRTOS Windows 模拟器的结果。窗户
模拟器并不是真正的实时。另外，写入 Windows
控制台需要相对较长的时间并导致一系列
Windows 系统调用。在真正的嵌入式上执行相同的代码
具有快速且非阻塞打印功能的目标可能会导致
每个任务在被切换之前多次打印其字符串
以允许其他任务运行。

图 4.2 显示了两个任务似乎同时执行；
然而，这两个任务在同一个处理器核心上执行，因此不能
就这样吧。实际上，这两个任务都在快速进入和退出
*运行*状态。两个任务都以相同的优先级运行，因此共享
同一处理器核心上的时间。图4.3显示了它们的实际执行情况
模式。

图 4.3 底部的箭头显示了从时间 t1 开始经过的时间
以后。彩色线显示每个点正在执行哪个任务
时间 — 例如，任务 1 在时间 t1 和 t2 之间执行。

任何时候只能有一个任务处于*运行*状态。所以，作为一
任务进入*运行* state (the task is switched in)，其他
输入*未运行* state (the task is switched out)。


<a name="fig4.3" title="Figure 4.3 The actual execution pattern of the two Example 4.1 tasks"></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.3_example_4.1_execution_pattern.png)
***图4.3*** *示例4.1中两个任务的实际执行模式*

***

示例 4.1 在启动之前从 `main()` 中创建了这两个任务
调度程序。也可以从另一个任务中创建任务
任务。例如，任务 2 可以从任务 1 中创建，如下所示
如清单 4.7 所示。


<a name="list4.7" title="Listing 4.7 Creating a task from within another task after the scheduler has started"></a>


```c
void vTask1( void * pvParameters )
{
    const char *pcTaskName = "Task 1 is running\r\n";
    volatile unsigned long ul; /* volatile to ensure ul is not optimized away. */

    /*
     * If this task code is executing then the scheduler must already have
     * been started. Create the other task before entering the infinite loop.
     */
    xTaskCreate( vTask2, "Task 2", 1000, NULL, 1, NULL );

    for( ;; )
    {
        /* Print out the name of this task. */
        vPrintLine( pcTaskName );

        /* Delay for a period. */
        for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
        {
            /*
             * This loop is just a very crude delay implementation. There is
             * nothing to do in here. Later examples will replace this crude
             * loop with a proper delay/sleep function.
             */
        }
    }
}
```

***清单 4.7*** *在调度程序启动后从另一个任务中创建一个任务*

<a name="example4.2" title="Example 4.2 Using the task parameter"></a>
---
***示例 4.2*** *使用任务参数*

---

例 4.1 中创建的两个任务几乎相同，唯一不同的是
它们之间的区别在于它们打印出来的文本字符串。如果你创建
单个任务执行的两个实例，并使用该任务
参数将字符串传递到每个实例，这将删除
重复。

例 4.2 将例 4.1 中使用的两个任务函数替换为
单任务函数名为 `vTaskFunction()`，如清单 4.8 所示。
注意任务参数如何转换为 `char *` 以获取字符串
该任务应该打印出来。


<a name="list4.8" title="Listing 4.8 The single task function used to create two tasks in Example 4.2"></a>

```c
void vTaskFunction( void * pvParameters )
{

    char *pcTaskName;
    volatile unsigned long ul; /* volatile to ensure ul is not optimized away. */

    /*
     * The string to print out is passed in via the parameter. Cast this to a
     * character pointer.
     */
    pcTaskName = ( char * ) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /* Print out the name of this task. */
        vPrintLine( pcTaskName );

        /* Delay for a period. */
        for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
        {
            /*
             * This loop is just a very crude delay implementation. There is
             * nothing to do in here. Later exercises will replace this crude
             * loop with a proper delay/sleep function.
             */
        }
    }
}
```

***清单 4.8*** *示例 4.2 中用于创建两个任务的单任务函数*

清单 4.9 创建了由以下方法实现的任务的两个实例
`vTaskFunction()`，使用任务的参数传递不同的字符串
进入每个。两个任务在控制下独立执行
FreeRTOS 调度程序并具有自己的堆栈，因此具有自己的副本
`pcTaskName` 和 `ul` 变量。


<a name="list4.9" title="Listing 4.9 The main() function for Example 2"></a>


```c
/*
 * Define the strings that will be passed in as the task parameters. These are
 * defined const and not on the stack used by main() to ensure they remain
 * valid when the tasks are executing.
 */
static const char * pcTextForTask1 = "Task 1 is running";
static const char * pcTextForTask2 = "Task 2 is running";

int main( void )
{
    /*
     * Variables declared here may no longer exist after starting the FreeRTOS
     * scheduler. Do not attempt to access variables declared on the stack used
     * by main() from tasks.
     */

    /* Create one of the two tasks. */
    xTaskCreate( vTaskFunction,             /* Pointer to the function that
                                               implements the task. */
                 "Task 1",                  /* Text name for the task. This is to
                                               facilitate debugging only. */
                 1000,                      /* Stack depth - small microcontrollers
                                               will use much less stack than this.*/
                 ( void * ) pcTextForTask1, /* Pass the text to be printed into
                                               the task using the task parameter. */
                 1,                         /* This task will run at priority 1. */
                 NULL );                    /* The task handle is not used in
                                               this example. */

    /*
     * Create the other task in exactly the same way. Note this time that
     * multiple tasks are being created from the SAME task implementation
     * (vTaskFunction). Only the value passed in the parameter is different.
     * Two instances of the same task definition are being created.
     */
    xTaskCreate( vTaskFunction,
                 "Task 2",
                 1000,
                 ( void * ) pcTextForTask2,
                 1,
                 NULL );

    /* Start the scheduler so the tasks start executing. */
    vTaskStartScheduler();

    /*
     * If all is well main() will not reach here because the scheduler will
     * now be running the created tasks. If main() does reach here then there
     * was not enough heap memory to create either the idle or timer tasks
     * (described later in this book). Chapter 3 provides more information on
     * heap memory management.
     */
    for( ;; )
    {
    }
}
```

***清单 4.9*** *示例 2 的 main() 函数*


示例 4.2 的输出与图 1 中示例 1 所示的输出完全相同
4.2.

## 4.5 任务优先级

FreeRTOS 调度程序始终确保最高优先级的任务
run 是选择进入*运行*状态的任务。平等的任务
优先级依次转变为*运行*状态。

用于创建任务的 API 函数的 `uxPriority` 参数
赋予任务初始优先级。 `vTaskPrioritySet()` API 功能
创建任务后更改其优先级。

应用程序定义的 `configMAX_PRIORITIES` 编译时配置
常量设置可用优先级的数量。低数字优先级
值表示低优先级任务，优先级 0 是最低的
优先级可能 - 因此有效优先级范围从 0 到
`(configMAX_PRIORITIES – 1)`。任意数量的任务可以共享相同的任务
优先。

FreeRTOS 调度程序有两种算法实现，用于
选择 *Running* 状态任务和最大允许值
`configMAX_PRIORITIES` 取决于所使用的实现：

### 4.5.1 通用调度程序

通用调度程序是用 C 编写的，可与所有 FreeRTOS 一起使用
架构端口。它没有对 `configMAX_PRIORITIES` 施加上限。
一般来说，建议最小化 `configMAX_PRIORITIES`，因为更多
值需要更多 RAM，并将导致最坏情况执行时间更长。

### 4.5.2 架构优化的调度程序

架构优化的实现是以特定于架构的方式编写的
汇编代码并且比通用 c 实现性能更高，并且
所有 `configMAX_PRIORITIES` 值的最坏情况执行时间相同。

架构优化的实现施加了最大值
`configMAX_PRIORITIES` 在 32 位架构上为 32，在 64 位架构上为 64
架构。与通用方法一样，建议保留
`configMAX_PRIORITIES` 处于实用的最低值，因为较高
值需要更多 RAM。

将 FreeRTOSConfig.h 中的 `configUSE_PORT_optimized_TASK_SELECTION` 设置为 1
使用架构优化实现，或 0 使用通用
实施。并非所有 FreeRTOS 端口都有优化的架构
实施。那些默认的
如果未定义，则 `configUSE_PORT_optimized_TASK_SELECTION` 为 1。
那些没有的，默认 `configUSE_PORT_optimized_TASK_SELECTION` 为 0
如果未定义。

## 4.6 时间测量和滴答中断

[Section 4.12, Scheduling Algorithms](#412-scheduling-algorithms)，描述了
称为“时间切片”的可选功能。示例中使用了时间切片
到目前为止所呈现的，是在他们产生的输出中观察到的行为。
在示例中，两个任务都是以相同的优先级创建的，并且两个任务都以相同的优先级创建
任务始终能够运行。因此，每个任务执行一段时间
slice'，在时间片开始时进入 *Running* 状态，并且
在时间片结束时退出*运行*状态。在图 4.3 中，
t1 和 t2 之间的时间等于单个时间片。

调度程序在每个时间片结束时执行以选择下一个时间片
要运行的任务[^5]。周期性中断，称为“滴答中断”，是
用于此目的。 `configTICK_RATE_HZ` 编译时配置
常量设置滴答中断的频率，因此
每个时间片的长度。例如，将 `configTICK_RATE_HZ` 设置为
100 (Hz) 表示每个时间片持续 10 毫秒。时间
两次滴答中断之间的时间称为“滴答周期”，因此一次
切片等于一个刻度周期。

[^5]：需要注意的是，时间片的结束并不是时间片的结束时间。
调度程序只能选择要运行的新任务的地方。正如我们将要
在本书中进行演示时，调度程序还将选择
在当前正在执行的任务之后立即运行的新任务
进入*阻塞*状态，或者当中断移动到更高的状态时
优先任务进入*就绪*状态。

图 4.4 对图 4.3 进行了扩展，还显示了
调度程序。在图 4.4 中，顶行显示了调度程序何时
正在执行，细箭头显示了从a开始的执行顺序
任务到tick中断，然后从tick中断返回到a
不同的任务。

`configTICK_RATE_HZ` 的最佳值取决于应用，
尽管典型值为 100。


<a name="fig4.4" title="Figure 4.4 The execution sequence expanded to show the tick interrupt executing"></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.4_expanded_execution_sequence_with_tick_interrupt.png)
***图 4.4*** *执行序列扩展以显示节拍中断执行*

***

FreeRTOS API 调用通常以刻度周期的倍数指定时间
简称为“蜱”。 `pdMS_TO_TICKS()` 宏转换时间
以毫秒为单位指定到以刻度为单位指定的时间。决议
可用取决于定义的刻度频率，并且 `pdMS_TO_TICKS()`
如果节拍频率高于 1KHz，则不能使用（如果
`configTICK_RATE_HZ` 大于 1000）。清单 4.10 显示了如何使用
`pdMS_TO_TICKS()` 将指定为 200 毫秒的时间转换为
以刻度为单位指定的等效时间。


<a name="list4.10" title="Listing 4.10 Using the pdMS\_TO\_TICKS() macro to convert 200 milliseconds..."></a>

```c
/*
 * pdMS_TO_TICKS() takes a time in milliseconds as its only parameter,
 * and evaluates to the equivalent time in tick periods. This example shows
 * xTimeInTicks being set to the number of tick periods that are equivalent
 * to 200 milliseconds.
 */
TickType_t xTimeInTicks = pdMS_TO_TICKS( 200 );
```

***清单 4.10*** *使用 pdMS\_TO\_TICKS() 宏转换 200 毫秒
转换为刻度周期中的等效时间*

使用 `pdMS_TO_TICKS()` 指定以毫秒为单位的时间，而不是
直接作为刻度，确保应用程序中指定的时间不会
如果滴答频率改变则改变。

“滴答计数”是具有的滴答中断总数
自调度程序启动以来发生，假设滴答计数尚未
溢出了。用户应用程序不必考虑溢出
指定延迟时间，因为 FreeRTOS 管理时间一致性
内部。

[Section 4.12: Scheduling Algorithms](#412-scheduling-algorithms)
描述影响调度程序何时进行的配置常量
选择要运行的新任务以及何时执行滴答中断。

<a name="example4.3" title="Example 4.3 Experimenting with priorities"></a>
---
***示例 4.3*** *试验优先级*

---

调度程序将始终确保可以运行最高优先级的任务
是选择进入*运行*状态的任务。到目前为止的例子
创建了两个具有相同优先级的任务，因此都进入并退出了
依次*运行*状态。这个例子看看当任务执行时会发生什么
有不同的优先级。清单 4.11 显示了用于创建
任务，第一个任务的优先级为 1，第二个任务的优先级为 2。
实现这两个任务的单个函数没有改变；它仍然
定期打印一个字符串，使用空循环来创建延迟。


<a name="list4.11" title="Listing 4.11. Creating two tasks at different priorities"></a>


```c
/*
 * Define the strings that will be passed in as the task parameters.
 * These are defined const and not on the stack to ensure they remain valid
 * when the tasks are executing.
 */
static const char * pcTextForTask1 = "Task 1 is running";
static const char * pcTextForTask2 = "Task 2 is running";

int main( void )
{
    /* Create the first task with a priority of 1. */
    xTaskCreate( vTaskFunction,             /* Task Function    */
                 "Task 1",                  /* Task Name        */
                 1000,                      /* Task Stack Depth */
                 ( void * ) pcTextForTask1, /* Task Parameter   */
                 1,                         /* Task Priority    */
                 NULL );

    /* Create the second task at a higher priority of 2. */
    xTaskCreate( vTaskFunction,             /* Task Function    */
                 "Task 2",                  /* Task Name        */
                 1000,                      /* Task Stack Depth */
                 ( void * ) pcTextForTask2, /* Task Parameter   */
                 2,                         /* Task Priority    */
                 NULL );

    /* Start the scheduler so the tasks start executing. */
    vTaskStartScheduler();

    /* Will not reach here. */
    return 0;
}
```

***清单 4.11*** *创建两个不同优先级的任务*

图 4.5 显示了示例 4.3 生成的输出。

调度程序将始终选择可以运行的最高优先级任务。
任务2的优先级高于任务1，并且可以一直运行；因此，
调度程序始终选择任务 2，而任务 1 从不执行。任务1
据说任务 2 的处理时间“匮乏”——它无法打印它的
字符串，因为它永远不会处于 *Running* 状态。


<a name="fig4.5" title="Figure 4.5 Running both tasks at different priorities"></a>

***
```console
C:\Temp>rtosdemo
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
Task 2 is running
```

***图 4.5*** *以不同的优先级运行两个任务*
***

任务 2 总是可以运行，因为它永远不需要等待任何事情——它是
要么围绕空循环循环，要么打印到终端。


<a name="fig4.6" title="Figure 4.6 The execution pattern when one task has a higher priority than the..."></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.6_execution_pattern_higher_priority_task.png)
***图4.6*** *当一个任务的优先级高于其他任务时的执行模式
其他来自示例 4.3*

## 4.7 扩展*Not Running*状态

到目前为止，创建的任务总是有要执行的处理并且有
从来不需要等待任何事情——而且因为他们从来不需要等待任何事情，
他们总是能够进入*运行*状态。这种“连续处理”
任务的用处有限，因为它们只能在最低级别创建
优先。如果它们以任何其他优先级运行，它们将阻止较低优先级的任务
永远不要运行的优先级。

为了使这些任务有用，必须将它们重写为事件驱动的。安
事件驱动任务只有 work (processing) 在事件发生后执行
触发它并且不能在该时间之前进入 *Running* 状态。
调度程序始终选择可以运行的最高优先级任务。如果
无法选择高优先级任务，因为它正在等待
事件，调度程序必须选择一个较低优先级的任务
可以运行。因此，编写事件驱动任务意味着任务可以
以不同优先级创建，没有最高优先级任务
使所有优先级较低的任务缺乏处理时间。

### 4.7.1 *阻塞*状态

等待事件的任务被称为处于“阻塞”状态，
*未运行*状态的子状态。

任务可以进入*阻塞*状态来等待两种不同类型的
事件：

1. Temporal (time-related) 事件 — 这些事件在延迟期发生时发生
   过期或达到绝对时间。例如，一个任务可以
   进入*阻塞*状态等待10毫秒过去。

1. 同步事件——这些事件源自另一个任务
   或中断。例如，任务可能会进入*阻塞*状态
   等待数据到达队列。同步事件涵盖
   广泛的事件类型。

FreeRTOS 队列、二进制信号量、计数信号量、互斥体、
递归互斥体、事件组、流缓冲区、消息缓冲区和
直接到任务通知都可以创建同步事件。
后面的章节将介绍其中的大部分功能。

任务可以有效地阻塞同步事件并超时
同时阻止两种类型的事件。例如，一个任务可以
选择等待最多 10 毫秒数据到达
队列。如果数据在 10 天内到达，任务将离开 *Blocked* 状态
毫秒或如果 10 毫秒过去而没有数据到达。

### 4.7.2 *暂停*状态

*挂起*也是*未运行*的子状态。暂停中的任务
状态对调度程序不可用。进入的唯一途径是
挂起状态是通过调用 `vTaskSuspend()` API 函数，
唯一的出路是通过调用 `vTaskResume()` 或
`xTaskResumeFromISR()` API 功能。大多数应用程序不使用
暂停状态。

### 4.7.3 就绪状态

处于“未运行”状态并且不是“已阻止”或“已暂停”的任务
据说处于“就绪”状态。他们可以奔跑，因此“准备好”
运行，但当前不处于 *Running* 状态。

### 4.7.4 完成状态转换图

图 4.7 扩展了简化的状态图以包含所有
本节中描述的*未运行*子状态。中创建的任务
 到目前为止的示例尚未使用*阻止*或*暂停*状态。他们有
 仅在 *Ready* 状态和 *Running* 状态之间转换，如下所示
 图 4.7 中的粗线。


<a name="fig4.7" title="Figure 4.7 Full task state machine"></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.7_full_task_state_machine.png)
***图 4.7*** *完整任务状态机*

***

<a name="example4.4" title="Example 4.4 Using the Blocked state to create a delay"></a>
---
***示例 4.4*** *使用*阻塞*状态创建 delay</i></h3>

---

到目前为止，示例中创建的所有任务均已
'periodic'——他们延迟了一段时间然后打印出他们的字符串，
然后再次延迟，依此类推。延迟已经产生得很
粗暴地使用空循环——任务轮询一个递增的
循环计数器，直到达到固定值。例4.3清楚
证明了该方法的缺点。较高优先级的任务
在执行空循环时保持*运行*状态，
“饿死”任何处理时间的较低优先级任务。

任何形式的民意调查都有其他一些缺点，尤其是
其中之一就是它的低效率。在轮询期间，任务并不真正
有任何工作要做，但它仍然使用最大处理时间，所以
浪费处理器周期。示例 4.4 通过替换来纠正此行为
轮询空循环，调用 `vTaskDelay()` API 函数，其
原型如清单 4.12 所示。新的任务定义是
如清单 4.13 所示。请注意，`vTaskDelay()` API 函数是
仅当 FreeRTOSConfig.h 中的 `INCLUDE_vTaskDelay` 设置为 1 时才可用。

`vTaskDelay()` 将调用任务置于*阻塞*状态，持续一段时间
滴答中断的数量。该任务不占用任何处理时间
当它处于 *Blocked* 状态时，因此任务仅使用处理时间
当确实有工作要做时。


<a name="list4.12" title="Listing 4.12 The vTaskDelay() API function prototype"></a>


```c
void vTaskDelay( TickType_t xTicksToDelay );
```

***清单 4.12*** *vTaskDelay() API 函数原型*

**vTaskDelay 参数：**

- `xTicksToDelay`

调用任务将保留的滴答中断数
  在转换回就绪状态之前处于*阻塞*状态
  状态。

例如，如果任务调用 `vTaskDelay( 100 )`，则当滴答计数
  是10,000，那么它会立即进入*Blocked*状态，并且
  保持在*阻塞*状态，直到滴答计数达到 10,100。

宏 `pdMS_TO_TICKS()` 可用于转换指定的时间
  以毫秒为单位指定的时间。例如，调用
  `vTaskDelay( pdMS_TO_TICKS( 100 ) )`导致调用任务剩余
  处于 *阻塞* 状态 100 毫秒。


<a name="list4.13" title="Listing 4.13 The source code for the example task after replacing the null loop delay with a call..."></a>


```c
void vTaskFunction( void * pvParameters )
{
    char * pcTaskName;
    const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );

    /*
     * The string to print out is passed in via the parameter. Cast this to a
     * character pointer.
     */
    pcTaskName = ( char * ) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /* Print out the name of this task. */
        vPrintLine( pcTaskName );

        /*
         * Delay for a period. This time a call to vTaskDelay() is used which
         * places the task into the Blocked state until the delay period has
         * expired. The parameter takes a time specified in 'ticks', and the
         * pdMS_TO_TICKS() macro is used (where the xDelay250ms constant is
         * declared) to convert 250 milliseconds into an equivalent time in
         * ticks.
         */
        vTaskDelay( xDelay250ms );
    }
}
```

***清单 4.13*** *替换空循环后示例任务的源代码
延迟调用 vTaskDelay()*

即使这两个任务仍在不同的时间创建
优先级，两者现在都将运行。例 4.4 的输出如下所示
图 4.8 确认了预期行为。


<a name="fig4.8" title="Figure 4.8 The output produced when Example 4.4 is executed"></a>

***

```console
C:\Temp>rtosdemo
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 1 is running
```

***图 4.8*** *执行例 4.4 时产生的输出*
***

图 4.9 中显示的执行序列解释了为什么两个任务都会运行，
即使它们是按不同的优先级创建的。的执行
为了简单起见，调度程序本身被省略。

空闲任务是在调度程序启动时自动创建的，
确保始终至少有一项任务可以运行（至少一项任务
处于*就绪*状态）。 [Section 4.8: The Idle Task and the Idle Task Hook](#48-the-idle-task-and-the-idle-task-hook)
更详细地描述了空闲任务。


<a name="fig4.9" title="Figure 4.9 The execution sequence when the tasks use vTaskDelay() in place of the null loop"></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.9_vTaskDelay_execution_sequence.png)
***图4.9*** *任务使用vTaskDelay()代替时的执行顺序
空循环*

***

只是这两个任务的执行方式发生了变化，其本身并没有改变
功能。比较图 4.9 和图 4.4 可以清楚地看出
该功能正在以更有效的方式实现
方式。

图 4.4 显示了任务使用空循环时的执行模式
造成延迟，所以总是能够运行。结果，他们使用一个
它们之间百分之百的可用处理器时间。图4.9
显示任务进入*阻塞*状态时的执行模式
他们的整个延迟期。他们仅在以下情况下才使用处理器时间
实际上有需要执行的工作（在本例中只是一个
消息被打印出来），因此只使用了一小部分
可用的处理时间。

在图 4.9 所示的场景中，每次任务离开 *Blocked* 状态
在重新进入之前，它们会执行一小部分时间周期
封锁状态。大多数时候没有应用程序任务可以
run (no application tasks in the *Ready* state)，因此，没有
可以选择进入*运行*状态的应用程序任务。同时
这样的话，空闲任务就运行了。处理时间量
分配给空闲是衡量空闲处理能力的指标
系统。使用 RTOS 可以显着增加备用
处理能力只需让应用程序完全
事件驱动。

图 4.10 中的粗线显示了任务执行的转换
在示例 4.4 中，每个任务现在都通过 *Blocked* 状态进行转换
在返回到*就绪*状态之前。


<a name="fig4.10" title="Figure 4.10 Bold lines indicate the state transitions performed by the tasks..."></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.10_example_4.4_state_machine.png)
***图4.10*** *粗线表示任务执行的状态转换
在示例 4.4* 中

***


### 4.7.5 vTaskDelayUntil() API 功能

`vTaskDelayUntil()` 与 `vTaskDelay()` 类似。正如刚才所证明的，
`vTaskDelay()`参数指定tick中断的数量
应该发生在调用 `vTaskDelay()` 的任务和同一任务之间
再次脱离*阻塞*状态。时间的长度
任务保持在阻塞状态由 `vTaskDelay()` 指定
参数，但任务离开阻塞状态的时间是
相对于调用 `vTaskDelay()` 的时间。

`vTaskDelayUntil()` 的参数指定了确切的刻度
调用任务应从阻塞状态移出的计数值
状态进入*就绪*状态。 `vTaskDelayUntil()` 是要使用的 API 函数
当需要固定的执行周期时（您希望任务
以固定频率定期执行），作为
调用任务的解锁是绝对的，而不是相对于何时
函数为 called (as is the case with `vTaskDelay()`)。


<a name="list4.14" title="Listing 4.14 vTaskDelayUntil() API function prototype"></a>

```c
void vTaskDelayUntil( TickType_t * pxPreviousWakeTime,
                      TickType_t xTimeIncrement );
```

***清单 4.14*** *vTaskDelayUntil() API 函数原型*

**vTaskDelayUntil() 参数**

- `pxPreviousWakeTime`

该参数的命名假设为 `vTaskDelayUntil()`
  用于实现定期执行的任务
  固定频率。在这种情况下，`pxPreviousWakeTime` 将时间保持在
  该任务最后留下的是*已阻止* state (was 'woken' up)。这次
  用作计算任务完成时间的参考点
  接下来应该离开*阻止*状态。

`pxPreviousWakeTime`指向的变量被更新
  自动在 `vTaskDelayUntil()` 功能中；它不会
  通常由应用程序代码修改，但必须初始化为
  首次使用之前的当前滴答计数。清单 4.15 演示了如何
  初始化变量。

- `xTimeIncrement`

该参数的命名也是基于以下假设：
  `vTaskDelayUntil()` 用于实现执行的任务
  定期并以固定频率设置
  `xTimeIncrement` 值。

`xTimeIncrement` 在“刻度”中指定。宏 `pdMS_TO_TICKS()` 可以
  用于将以毫秒为单位的时间转换为时间
  以刻度指定。

<a name="example4.5" title="Example 4.5 Converting the example tasks to use vTaskDelayUntil()"></a>
---
***示例 4.5*** *将示例任务转换为使用 vTaskDelayUntil()*

---

例 4.4 中创建的两个任务是周期性任务，但是使用
`vTaskDelay()` 不保证它们运行的频率是
固定，因为任务离开*阻塞*状态的时间是
相对于他们调用 `vTaskDelay()` 时的情况。转换任务以供使用
`vTaskDelayUntil()` 而不是 `vTaskDelay()` 解决了这个潜在问题。


<a name="list4.15" title="Listing 4.15 The implementation of the example task using vTaskDelayUntil()"></a>


```c
void vTaskFunction( void * pvParameters )
{
    char * pcTaskName;
    TickType_t xLastWakeTime;

    /*
     * The string to print out is passed in via the parameter. Cast this to a
     * character pointer.
     */
    pcTaskName = ( char * ) pvParameters;

    /*
     * The xLastWakeTime variable needs to be initialized with the current tick
     * count. Note that this is the only time the variable is written to
     * explicitly. After this xLastWakeTime is automatically updated within
     * vTaskDelayUntil().
     */
    xLastWakeTime = xTaskGetTickCount();

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /* Print out the name of this task. */
        vPrintLine( pcTaskName );

        /*
         * This task should execute every 250 milliseconds exactly. As per
         * the vTaskDelay() function, time is measured in ticks, and the
         * pdMS_TO_TICKS() macro is used to convert milliseconds into ticks.
         * xLastWakeTime is automatically updated within vTaskDelayUntil(), so
         * is not explicitly updated by the task.
         */
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 250 ) );
    }
}
```

***清单 4.15*** *使用 vTaskDelayUntil() 执行示例任务*

例 4.5 产生的输出与例 4.4 所示的输出完全相同
如图 4.8 所示。

<a name="example4.6" title="Example 4.6 Combining blocking and non-blocking tasks"></a>
---
***示例 4.6*** *结合阻塞和非阻塞任务*

---

前面的示例检查了轮询和阻塞的行为
孤立的任务。这个例子再次强化了我们已经说过的关于
预期的系统行为并演示执行顺序
两种方案结合起来，如下：

1. 以优先级 1 创建两个任务。这些任务除了连续执行之外什么也不做
   打印出一个字符串。

这些任务从不进行 API 函数调用，这可能会导致它们进入
   *阻塞*状态，因此始终处于就绪或*运行*状态。
   这种性质的任务称为“连续处理”任务，因为它们总是
   已工作至 do (albeit rather trivial work, in this case)。
   清单 4.16 显示了连续处理任务的源代码。

1. 然后以优先级 2 创建第三个任务，该任务的优先级高于
   另外两个任务。第三个任务也只是打印出一个字符串，但是这个
   周期性地计时，因此它使用 `vTaskDelayUntil()` API 函数来放置
   在每次打印迭代之间其自身进入*阻塞*状态。

清单 4.17 显示了周期性任务的源代码。


<a name="list4.16" title="Listing 4.16 The continuous processing task used in Example 4.6"></a>


```c
void vContinuousProcessingTask( void * pvParameters )
{
    char * pcTaskName;

    /*
     * The string to print out is passed in via the parameter. Cast this to a
     * character pointer.
     */
    pcTaskName = ( char * ) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /*
         * Print out the name of this task. This task just does this repeatedly
         * without ever blocking or delaying.
         */
        vPrintLine( pcTaskName );
    }
}
```

***清单 4.16*** *示例 4.6 中使用的连续处理任务*


<a name="list4.17" title="Listing 4.17 The periodic task used in Example 4.6"></a>


```c
void vPeriodicTask( void * pvParameters )
{
    TickType_t xLastWakeTime;

    const TickType_t xDelay3ms = pdMS_TO_TICKS( 3 );

    /*
     * The xLastWakeTime variable needs to be initialized with the current tick
     * count. Note that this is the only time the variable is explicitly
     * written to. After this xLastWakeTime is managed automatically by the
     * vTaskDelayUntil() API function.
     */
    xLastWakeTime = xTaskGetTickCount();

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /* Print out the name of this task. */
        vPrintLine( "Periodic task is running" );

        /*
         * The task should execute every 3 milliseconds exactly – see the
         * declaration of xDelay3ms in this function.
         */
        vTaskDelayUntil( &xLastWakeTime, xDelay3ms );
    }
}
```

***清单 4.17*** *示例 4.6 中使用的周期性任务*

图 4.11 显示了例 4.6 产生的输出，并解释了
观察到的行为由图 4.12 所示的执行序列给出。

<a name="fig4.11" title="Figure 4.11 The output produced when Example 4.6 is executed"></a>

***

```console
Continuous task 2 running
Continuous task 2 running
Periodic task is running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Continuous task 2 running
Continuous task 2 running
Continuous task 2 running
Continuous task 2 running
Continuous task 2 running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Continuous task 1 running
Periodic task is running
Continuous task 2 running
Continuous task 2 running
```

***图 4.11*** *执行例 4.6 时产生的输出*
***


<a name="fig4.12" title="Figure 4.12 The execution pattern of Example 4.6"></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.11_example_4.6_execution_pattern.png)
***图4.12*** *例4.6的执行模式*

***

## 4.8 空闲任务和空闲任务挂钩

例 4.4 中创建的任务大部分时间都花在 Blocked 中
状态。在此状态下，它们无法运行，因此无法
由调度程序选择。

必须始终至少有一项任务可以进入运行状态
状态[^6]。为了确保这种情况，调度程序会自动
调用 `vTaskStartScheduler()` 时创建空闲任务。空闲任务
所做的只是坐在循环中，所以，就像第一个中的任务一样
例子，它总是能够运行。

[^6]：即使特殊的低功耗功能也是如此
正在使用 FreeRTOS，在这种情况下，其上的微控制器
如果没有，FreeRTOS 正在执行将被置于低功耗模式
应用程序创建的任务能够执行。

空闲任务具有尽可能低的 priority (priority zero)，以
确保它永远不会阻止更高优先级的应用程序任务
进入*运行*状态。然而，没有什么可以阻止
应用程序设计者从创建任务，因此共享空闲
任务优先级（如果需要）。 `configIDLE_SHOULD_YIELD` 编译时间
`FreeRTOSConfig.h` 中的配置常量可用于防止
空闲任务会消耗处理时间，从而提高工作效率
分配给优先级也为 0 的应用程序任务。 节
4.12，调度算法，描述 `configIDLE_SHOULD_YIELD`。

以最低优先级运行可确保空闲任务转出
一旦更高优先级的任务进入就绪状态，就进入*运行*状态
状态。这可以在图 4.9 中的 **tn** 时刻看到，其中 Idle 任务是
立即换出以允许任务 2 在任务 2 的瞬间执行
离开*阻塞*状态。据说任务2抢占了空闲
任务。抢占会自动发生，并且无需通知
任务被抢占。

> *注意：如果任务使用 `vTaskDelete()` API 函数删除自身
> 那么空闲任务不缺乏处理时间是至关重要的。
> 这是因为Idle任务负责清理内核
> 自行删除的任务使用的资源。*

### 4.8.1 空闲任务挂钩函数

可以将应用程序特定的功能直接添加到
通过使用空闲 hook (or idle callback) 来执行空闲任务
function，是空闲任务自动调用一次的函数
空闲任务循环的每次迭代。

空闲任务挂钩的常见用途包括：

- 执行低优先级、后台或连续处理
  无需创建应用程序任务的 RAM 开销的功能
  为了目的。

- 测量空闲处理能力的量。 （空闲任务
  仅当所有更高优先级的应用程序任务都没有时才会运行
  执行的工作；所以测量处理时间
  分配给空闲任务提供了空闲任务的清晰指示
  处理时间。）

- 将处理器置于低功耗模式，提供简单且
  没有应用程序时自动省电的方法
  要执行的处理（尽管可实现的节能是
  小于无滴答空闲模式所实现的值）。

### 4.8.2 空闲任务钩子函数实现的限制

空闲任务挂钩函数必须遵守以下规则。

- 空闲任务挂钩函数绝不能尝试阻止或挂起自身。

*注意：以任何方式阻止空闲任务都可能导致以下情况
  没有任务可进入*正在运行*状态。*

- 如果应用程序任务使用 `vTaskDelete()` API 函数
  要删除自身，则空闲任务挂钩必须始终返回到其
  来电者在合理的时间内。这是因为空闲
  任务负责清理分配给的内核资源
  删除自身的任务。如果空闲任务永久存在
  在 Idle hook 函数中，则无法进行此清理。

空闲任务挂钩函数必须具有清单 4.18 中所示的名称和原型。


<a name="list4.18" title="Listing 4.18 The idle task hook function name and prototype"></a>

```c
void vApplicationIdleHook( void );
```

***清单 4.18*** *空闲任务钩子函数名称和原型*

<a name="example4.7" title="Example 4.7 Defining an idle task hook function"></a>
---
***示例 4.7*** *定义空闲任务钩子 function</i></h3>

---

示例 4.4 中使用阻塞 `vTaskDelay()` API 调用创建了很多
空闲时间，即空闲任务执行的时间，因为两个应用程序
任务处于*阻塞*状态。例 4.7 利用了这个空闲时间
通过添加一个 Idle 钩子函数，其来源是
如清单 4.19 所示。


<a name="list4.19" title="Listing 4.19 A very simple Idle hook function"></a>

```c
/* Declare a variable that will be incremented by the hook function.  */
volatile unsigned long ulIdleCycleCount = 0UL;

/*
 * Idle hook functions MUST be called vApplicationIdleHook(), take no
 * parameters, and return void.
 */
void vApplicationIdleHook( void )
{
    /* This hook function does nothing but increment a counter. */
    ulIdleCycleCount++;
}
```

***清单 4.19*** *一个非常简单的空闲钩子函数*


FreeRTOSConfig.h 中的 `configUSE_IDLE_HOOK` 必须设置为 1，才能实现空闲挂机功能
被叫。

实现创建任务的函数稍作修改为
打印出 `ulIdleCycleCount` 值，如清单 4.20 所示。


<a name="list4.20" title="Listing 4.20 The source code for the example task now prints out the ulIdleCycleCount value"></a>


```c
void vTaskFunction( void * pvParameters )
{
    char * pcTaskName;
    const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );

    /*
     * The string to print out is passed in via the parameter. Cast this to
     * a character pointer.
     */
    pcTaskName = ( char * ) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        /*
         * Print out the name of this task AND the number of times
         * ulIdleCycleCount has been incremented.
         */
        vPrintLineAndNumber( pcTaskName, ulIdleCycleCount );

        /* Delay for a period of 250 milliseconds. */
        vTaskDelay( xDelay250ms );
    }
}
```

***清单 4.20*** *示例任务的源代码现在打印出
ulIdleCycleCount值*

图 4.13 显示了示例 4.7 生成的输出。可见
空闲任务钩子函数执行大约400万次
应用程序任务的每次迭代之间（
迭代取决于硬件速度）。


<a name="fig4.13" title="Figure 4.13 The output produced when Example 4.7 is executed"></a>

***

```console
C:\Temp>rtosdemo
Task 2 is running
ulIdleCycleCount = 0
Task 1 is running
ulIdleCycleCount = 0
Task 2 is running
ulIdleCycleCount = 3869504
Task 1 is running
ulIdleCycleCount = 3869504
Task 2 is running
ulIdleCycleCount = 8564623
Task 1 is running
ulIdleCycleCount = 8564623
Task 2 is running
ulIdleCycleCount = 13181489
Task 1 is running
ulIdleCycleCount = 13181489
Task 2 is running
ulIdleCycleCount = 17838406
Task 1 is running
ulIdleCycleCount = 17838406
Task 2 is running
```

***图 4.13*** *执行例 4.7 时产生的输出*
***


## 4.9 更改任务的优先级

### 4.9.1 vTaskPrioritySet() API 功能

`vTaskPrioritySet()` API 函数在执行后更改任务的优先级
调度程序已启动。 `vTaskPrioritySet()` API 功能是
仅当 `INCLUDE_vTaskPrioritySet` 设置为 1 时可用
FreeRTOSConfig.h。


<a name="list4.21" title="Listing 4.21 The vTaskPrioritySet() API function prototype"></a>

```c
void vTaskPrioritySet( TaskHandle_t xTask,
                       UBaseType_t uxNewPriority );
```

***清单 4.21*** *vTaskPrioritySet() API 功能原型*

**vTaskPrioritySet() 参数**

- `pxTask`

正在修改优先级的任务的句柄（
  主题任务）。请参阅 `xTaskCreate()` API 的 `pxCreatedTask` 参数
  函数，或 `xTaskCreateStatic()` API 函数的返回值，
  有关获取任务句柄的信息。

任务可以通过传递 NULL 代替
  有效的任务句柄。

- `uxNewPriority`

要设置主题任务的优先级。这是有上限的
  自动为 `(configMAX_PRIORITIES – 1)` 的最大可用优先级，
  其中 `configMAX_PRIORITIES` 是在中设置的编译时间常数
  FreeRTOSConfig.h 头文件。


### 4.9.2 uxTaskPriorityGet() API 功能

`uxTaskPriorityGet()` API 函数返回任务的优先级。的
`uxTaskPriorityGet()` API 功能仅在以下情况下可用
`INCLUDE_uxTaskPriorityGet` 在 FreeRTOSConfig.h 中设置为 1。


<a name="list4.22" title="Listing 4.22 The uxTaskPriorityGet() API function prototype"></a>

```c
UBaseType_t uxTaskPriorityGet( TaskHandle_t xTask );
```

***清单 4.22*** *uxTaskPriorityGet() API 功能原型*

**uxTaskPriorityGet()参数及返回值**

- `pxTask`

正在查询优先级的任务的句柄（
  主题任务）。请参阅 `xTaskCreate()` API 的 `pxCreatedTask` 参数
  函数，或 `xTaskCreateStatic()` API 函数的返回值，
  有关如何获取任务句柄的信息。

任务可以通过传递 NULL 代替有效的优先级来查询自己的优先级
  任务句柄。

- 返回值

当前分配给正在查询的任务的优先级。


<a name="example4.8" title="Example 4.8 Changing task priorities"></a>
---
***示例 4.8*** *更改任务优先级*

---

调度程序始终选择最高*就绪*状态的任务作为要执行的任务
进入*运行*状态。例 4.8 通过使用
`vTaskPrioritySet()` API 函数改变两个任务的优先级
相对于彼此。

例 4.8 创建了两个具有两个不同优先级的任务。没有任务
进行任何可能导致其进入阻止状态的 API 函数调用
状态，因此两者始终处于“就绪”状态或“运行”状态
状态。因此，具有最高相对优先级的任务将
始终是调度程序选择处于*运行*状态的任务。

例 4.8 的行为如下：

1. 任务 1（清单 4.23）是用最高优先级创建的，所以它是
   保证先运行。任务 1 打印出几个字符串
   在将任务 2（清单 4.24）的优先级提高到其自身之上之前
   优先。

1.任务2一旦完成就开始到run (enters the *Running* state)
   最高的相对优先级。运行中只能有一个任务
   状态在任意时刻，因此当任务 2 处于*运行*状态时，任务
   1 处于*就绪*状态。

1. 任务 2 在重新设置其优先级之前打印出一条消息
   低于任务 1。

1. 当任务 2 重新设置其优先级时，任务 1 再次成为
   最高优先级任务，因此任务 1 重新进入*运行*状态，
   强制任务 2 回到“就绪”状态。


<a name="list4.23" title="Listing 4.23 The implementation of Task 1 in Example 4.8"></a>

```c
void vTask1( void * pvParameters )
{
    UBaseType_t uxPriority;

    /*
     * This task will always run before Task 2 as it is created with the higher
     * priority. Neither Task 1 nor Task 2 ever block so both will always be in
     * either the Running or the Ready state.
     */

    /*
     * Query the priority at which this task is running - passing in NULL means
     * "return the calling task's priority".
     */
    uxPriority = uxTaskPriorityGet( NULL );

    for( ;; )
    {
        /* Print out the name of this task. */
        vPrintLine( "Task 1 is running" );

        /*
         * Setting the Task 2 priority above the Task 1 priority will cause
         * Task 2 to immediately start running (as then Task 2 will have the
         * higher priority of the two created tasks). Note the use of the
         * handle to task 2 (xTask2Handle) in the call to vTaskPrioritySet().
         * Listing 4.25 shows how the handle was obtained.
         */
        vPrintLine( "About to raise the Task 2 priority" );
        vTaskPrioritySet( xTask2Handle, ( uxPriority + 1 ) );

        /*
         * Task 1 will only run when it has a priority higher than Task 2.
         * Therefore, for this task to reach this point, Task 2 must already
         * have executed and set its priority back down to below the priority
         * of this task.
         */
    }
}
```

***清单 4.23*** *示例 4.8 中任务 1 的实现*


<a name="list4.24" title="Listing 4.24 The implementation of Task 2 in Example 4.8"></a>


```c
void vTask2( void * pvParameters )
{
    UBaseType_t uxPriority;

    /*
     * Task 1 will always run before this task as Task 1 is created with the
     * higher priority. Neither Task 1 nor Task 2 ever block so will always be
     * in either the Running or the Ready state.
     *
     * Query the priority at which this task is running - passing in NULL means
     * "return the calling task's priority".
     */
    uxPriority = uxTaskPriorityGet( NULL );

    for( ;; )
    {
        /*
         * For this task to reach this point Task 1 must have already run and
         * set the priority of this task higher than its own.
         */

         /* Print out the name of this task. */
        vPrintLine( "Task 2 is running" );

        /*
         * Set the priority of this task back down to its original value.
         * Passing in NULL as the task handle means "change the priority of the
         * calling task". Setting the priority below that of Task 1 will cause
         * Task 1 to immediately start running again – preempting this task.
         */
        vPrintLine( "About to lower the Task 2 priority" );
        vTaskPrioritySet( NULL, ( uxPriority - 2 ) );
    }
}
```

***清单 4.24*** *示例 4.8 中任务 2 的实现*

每个任务都可以通过使用 NULL 查询和设置自己的优先级
有效的任务句柄。仅当任务执行时才需要任务句柄
希望引用除自身之外的任务，例如当任务 1
更改任务 2 的优先级。为了允许任务 1 执行此操作，任务 2
创建任务 2 时获取并保存句柄，如中突出显示的
清单 4.25 中的注释。


<a name="list4.25" title="Listing 4.25 The implementation of main() for Example 4.8"></a>

```c
/* Declare a variable that is used to hold the handle of Task 2. */
TaskHandle_t xTask2Handle = NULL;

int main( void )
{
    /*
     * Create the first task at priority 2. The task parameter is not used
     * and set to NULL. The task handle is also not used so is also set to
     * NULL.
     */
    xTaskCreate( vTask1, "Task 1", 1000, NULL, 2, NULL );
    /* The task is created at priority 2 ______^. */

    /*
     * Create the second task at priority 1 - which is lower than the priority
     * given to Task 1. Again the task parameter is not used so is set to NULL-
     * BUT this time the task handle is required so the address of xTask2Handle
     * is passed in the last parameter.
     */
    xTaskCreate( vTask2, "Task 2", 1000, NULL, 1, &xTask2Handle );
    /* The task handle is the last parameter _____^^^^^^^^^^^^^ */

    /* Start the scheduler so the tasks start executing. */
    vTaskStartScheduler();

    /*
     * If all is well main() will not reach here because the scheduler will
     * now be running the created tasks. If main() does reach here then there
     * was not enough heap memory to create either the idle or timer tasks
     * (described later in this book). Chapter 3 provides more information on
     * heap memory management.
     */
    for( ;; )
    {
    }
}
```

***清单 4.25*** *示例 4.8 的 main() 的实现*

图 4.14 演示了例 4.8 中的任务的顺序
执行，结果输出如图4.15所示。


<a name="fig4.14" title="Figure 4.14 The sequence of task execution when running Example 4.8"></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.14_example_4.8_execution_sequence.png)
***图4.14*** *运行例4.8时任务执行顺序*

***


<a name="fig4.15" title="Figure 4.15 The output produced when Example 4.8 is executed"></a>

***

```console
Task1 is running
About to raise the Task2 priority
Task2 is running
About to lower the Task2 priority
Task1 is running
About to raise the Task2 priority
Task2 is running
About to lower the Task2 priority
Task1 is running
About to raise the Task2 priority
Task2 is running
About to lower the Task2 priority
Task1 is running
```

***图 4.15*** *执行例 4.8 时产生的输出*
***

## 4.10 删除任务

### 4.10.1 vTaskDelete() API 功能

`vTaskDelete()` API 函数删除任务。 `vTaskDelete()` API
该功能仅在 `INCLUDE_vTaskDelete` 设置为 1 时可用
FreeRTOSConfig.h。

在运行时不断创建和删除任务不是一个好习惯
时间，所以考虑其他设计选项，例如重用任务，如果您
发现自己需要这个功能。

删除的任务不再存在，无法再次进入*运行*状态。

如果使用动态内存分配创建的任务稍后删除自身，
Idle任务负责释放分配使用的内存，例如
删除任务的数据结构和堆栈。所以重要的是
应用程序不会完全饿死所有处理的空闲任务
当这种情况发生的时候。

> *注意：只有内核本身分配给任务的内存才会被释放
> 当任务被删除时自动。任何内存或其他资源
> 任务执行期间分配的资源必须显式释放
> 如果不再需要。*


<a name="list4.26" title="Listing 4.26 The vTaskDelete() API function prototype"></a>


```c
void vTaskDelete( TaskHandle_t xTaskToDelete );
```

***清单 4.26*** *vTaskDelete() API 功能原型*

**vTaskDelete() 参数**

- `pxTaskToDelete`

要删除的任务句柄（主题
  任务）。请参阅 `xTaskCreate()` API 函数的 `pxCreatedTask` 参数，
  以及 `xTaskCreateStatic()` API 函数的返回值，对于
  有关获取任务句柄的信息。

任务可以通过传递 NULL 代替有效任务来删除自身
  手柄。


<a name="example4.9" title="Example 4.9 Deleting tasks"></a>
---
***示例 4.9*** *删除任务*

---

这是一个非常简单的示例，其行为如下。

1、任务1由`main()`创建，优先级为1，运行时，
   以优先级 2 创建任务 2。任务 2 现在是最高优先级
   任务，因此它立即开始执行。清单 4.27 显示了
   `main()` 的源代码。清单 4.28 显示了任务 1 的源代码。

1. 任务 2 除了删除自身之外什么也不做。它可以删除自己
   通过将 NULL 传递给 `vTaskDelete()`，而是用于演示
   出于目的，它使用自己的任务句柄。清单 4.29 显示了源代码
   任务 2 的代码。

1. 当任务 2 被删除后，任务 1 再次成为最高优先级
   任务，因此它继续执行 - 此时它调用 `vTaskDelay()`
   短期封锁。

1. 当任务 1 处于阻塞状态时，空闲任务执行，并且
   释放分配给现已删除的任务 2 的内存。

1. 当任务1离开阻塞状态时，它再次变为最高
   优先*就绪*状态任务，因此抢占空闲任务。当它
   进入 *Running* 状态，它再次创建任务 2，如此继续。


<a name="list4.27" title="Listing 4.27 The implementation of main() for Example 4.9"></a>

```c
int main( void )
{
    /* Create the first task at priority 1. */
    xTaskCreate( vTask1, "Task 1", 1000, NULL, 1, NULL );

    /* Start the scheduler so the task starts executing. */
    vTaskStartScheduler();

    /* main() should never reach here as the scheduler has been started. */
    for( ;; )
    {
    }
}
```

***清单 4.27*** *示例 4.9 的 main() 的实现*
***


<a name="list4.28" title="Listing 4.28 The implementation of Task 1 for Example 4.9"></a>

```c
TaskHandle_t xTask2Handle = NULL;

void vTask1( void * pvParameters )
{
    const TickType_t xDelay100ms = pdMS_TO_TICKS( 100UL );

    for( ;; )
    {
        /* Print out the name of this task. */
        vPrintLine( "Task 1 is running" );

        /*
         * Create task 2 at a higher priority.
         * Pass the address of xTask2Handle as the pxCreatedTask parameter so
         * that xTaskCreate write the resulting task handle to that variable.
         */
        xTaskCreate( vTask2, "Task 2", 1000, NULL, 2, &xTask2Handle );

        /*
         * Task 2 has/had the higher priority. For Task 1 to reach here, Task 2
         * must have already executed and deleted itself.
         */
        vTaskDelay( xDelay100ms );
    }
}
```

***清单 4.28*** *示例 4.9 的任务 1 的实现*


<a name="list4.29" title="Listing 4.29 The implementation of Task 2 for Example 4.9"></a>


```c
void vTask2( void * pvParameters )
{
    /*
     * Task 2 immediately deletes itself upon starting.
     * To do this it could call vTaskDelete() using NULL as the parameter.
     * For demonstration purposes, it instead calls vTaskDelete() with its own
     * task handle.
     */
    vPrintLine( "Task 2 is running and about to delete itself" );
    vTaskDelete( xTask2Handle );
}
```

***清单 4.29*** *示例 4.9 的任务 2 的实现*


<a name="fig4.16" title="Figure 4.16 The output produced when Example 4.9 is executed"></a>

***

```console
C:\Temp>rtosdemo
Task1 is running
Task2 is running and about to delete itself
Task1 is running
Task2 is running and about to delete itself
Task1 is running
Task2 is running and about to delete itself
Task1 is running
Task2 is running and about to delete itself
Task1 is running
Task2 is running and about to delete itself
Task1 is running
Task2 is running and about to delete itself
Task1 is running
Task2 is running and about to delete itself
Task1 is running
Task2 is running and about to delete itself
```

***图 4.16*** *执行例 4.9 时产生的输出*
***


<a name="fig4.17" title="Figure 4.17 The execution sequence for Example 4.9"></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.17_example_4.9_execution_sequence.png)
***图 4.17*** *示例 4.9 的执行顺序*

***


## 4.11 线程本地存储和重入

线程本地存储允许应用程序开发人员将任意数据存储在
每个任务的任务控制块。此功能最常用于存储
通常由不可重入函数存储在全局变量中的数据。

可重入函数是可以从多个线程安全运行的函数
无任何副作用。当不可重入函数用于
多线程环境没有线程本地存储，必须特别小心
用于检查这些函数调用的带外结果
关键部分。过度使用关键部分会降低 RTOS 性能，
因此，线程本地存储通常优于临界区的使用。

到目前为止，线程本地存储最常见的用途是全局使用的“`errno`”
在C标准库和POSIX系统使用的ISO C标准中。
```errno``` 全局用于提供扩展结果或错误代码
常见的标准库函数，例如 strtof 和 strtol。

### 4.11.1 C 运行时线程本地存储实现

大多数嵌入式 libc 实现都提供 API 来确保不可重入
函数可以在多线程环境下正常工作。 FreeRTOS 包括
支持两个常用开源库的重入 API：
[newlib](https://sourceware.org/newlib/) 和
[picolibc](https://github.com/picolibc/picolibc)。
这些预构建的 C 运行时线程本地存储实现可以通过以下方式启用
通过在其项目的 FreeRTOSConfig.h 中定义下面列出的相应宏
文件。

- [newlib](https://sourceware.org/newlib/) 的“`configUSE_NEWLIB_REENTRANT`”
- [picolibc](https://github.com/picolibc/picolibc) 的“`configUSE_PICOLIBC_TLS`”

### 4.11.2 自定义 C 运行时线程本地存储

应用程序开发人员可以通过定义以下内容来实现线程本地存储
FreeRTOSConfig.h 文件中的宏：

- 将“`configUSE_C_RUNTIME_TLS_SUPPORT`”定义为 1 以启用 C 运行时线程
本地存储支持。

- 将“`configTLS_BLOCK_TYPE`”定义为用于存储的c类型
C 运行时线程本地存储数据。

- 将“`configINIT_TLS_BLOCK`”定义为初始化时应运行的c代码
C 运行时线程本地存储块。

- 将“`configSET_TLS_BLOCK`”定义为切换时应运行的c代码
在新任务中

- 将“`configDEINIT_TLS_BLOCK`”定义为反初始化时应运行的c代码
C 运行时线程本地存储块。

### 4.11.3 应用程序线程本地存储

除了C运行时线程本地存储之外，应用程序开发人员还可以
定义一组要包含在任务控制中的应用程序特定指针
块。通过设置“`configNUM_THREAD_LOCAL_STORAGE_POINTERS`”启用此功能
为项目 FreeRTOSConfig.h 文件中的非零数字。
```vTaskSetThreadLocalStoragePointer```` and ````pvTaskGetThreadLocalStoragePointer```
清单 4.30 中定义的函数可分别用于设置和获取
运行时每个线程本地存储指针的值。


<a name="list4.30" title="Listing 4.30 Function prototypes of the Thread Local Storage Pointer API functions"></a>

```c
void * pvTaskGetThreadLocalStoragePointer( TaskHandle_t xTaskToQuery,
                                           BaseType_t xIndex )

void vTaskSetThreadLocalStoragePointer( TaskHandle_t xTaskToSet,
                                        BaseType_t xIndex,
                                        void * pvValue );
```

***清单 4.30*** *线程本地存储指针 API 函数的函数原型*


## 4.12 调度算法

### 4.12.1 任务状态和事件回顾

实际上是 running (using processing time) 的任务位于
*运行*状态。在单核处理器上只能有一个任务
任何给定时间的*运行*状态。也可以运行 FreeRTOS
在多个 core (asymmetric multiprocessing, or AMP) 上，或者有
FreeRTOS 跨多个核心调度任务（对称
多处理，或 SMP）。这里没有描述这两种情况。

实际未运行但不在阻塞状态的任务
状态或*挂起*状态，处于*就绪*状态。准备中的任务
状态可供调度程序选择作为要进入的任务
*运行*状态。调度器总是会选择最高优先级
就绪状态任务进入*运行*状态。

任务可以在“阻塞”状态下等待事件，并且它们会自动
事件发生时移回*就绪*状态。时间事件
发生在特定时间，例如，当区块时间到期时，以及
通常用于实现周期性或超时行为。
当任务或中断服务例程发生时同步事件发生
使用任务通知、队列、事件组、消息发送信息
缓冲区、流缓冲区或多种信号量类型之一。他们是
通常用于指示异步活动，例如数据到达
一个外围设备。


### 4.12.2 选择调度算法

调度算法是决定哪个的软件例程
*就绪*状态任务转换为*运行*状态。

到目前为止所有的例子都使用了相同的调度算法，但是
可以使用 `configUSE_PREEMPTION` 更改算法和
`configUSE_TIME_SLICING` 配置常量。两个常数都是
FreeRTOSConfig.h 中定义。

第三个配置常数 `configUSE_TICKLESS_IDLE` 也会影响
调度算法，因为它的使用可能会导致滴答中断
长时间完全关闭。
`configUSE_TICKLESS_IDLE` 是专门为
用于必须最大限度降低功耗的应用。
本节中提供的描述假定为 `configUSE_TICKLESS_IDLE`
设置为 0，如果保留常量，则这是默认设置
未定义。

在所有可能的单核配置中，FreeRTOS 调度程序
依次选择共享优先级的任务。这个“轮流做”
策略通常称为“循环调度”。循环赛
调度算法不保证时间在之间平等共享
同等优先级的任务，仅*就绪*状态任务具有同等优先级
依次进入*运行*状态。

<a name="tbl5" title="Table 5 The FreeRTOSConfig.h settings to configure the kernel scheduling algorithms"></a>

***
|调度算法|优先 | `configUSE_PREEMPTION` | `configUSE_TIME_SLICING` |
|---------------------------------|-------------|------------------------|--------------------------|
|抢占式时间切片 |是的 | 1 | 1 |
|抢占式无时间切片 |是的 | 1 | 0 |
|合作社 |没有 | 0 |任何 |

***表 5*** *FreeRTOSConfig.h 设置来配置内核调度算法*
* * *

### 4.12.3 带时间分片的优先抢占式调度

表 5 中显示的配置将 FreeRTOS 调度程序设置为使用
调度算法称为“固定优先级抢占式调度”
Time Slicing'，这是大多数小型RTOS使用的调度算法
应用程序，以及所有示例所使用的算法
这本书到目前为止。下表提供了术语的描述
用在算法的名称中。

**用于描述调度策略的术语解释：**

- 固定优先级

描述为“固定优先级”的调度算法不会更改优先级
  分配给正在计划的任务，但也不阻止任务
  他们自己不会改变自己的优先级或其他任务的优先级。

- 先发制人

抢占式调度算法将立即“抢占”*运行*状态
  如果有优先级高于 *Running* 状态任务的任务进入
  *就绪*状态。被抢占意味着不由自主地被移出
  *运行*状态并进入*就绪*状态（没有显式产生或
  阻塞）以允许不同的任务进入*运行*状态。任务抢占
  可以在任何时间发生，而不仅仅是在 RTOS 滴答中断中。

- 时间切片

时间分片用于在同等优先级的任务之间共享处理时间，
  即使任务没有明确屈服或进入*阻塞*状态。
  描述为使用“时间切片”的调度算法选择一个新任务来
  如果还有其他*Ready*，则在每个时间片结束时进入*Running*状态
  状态任务与正在运行的任务具有相同优先级。一个时间片是相等的
  两次 RTOS 滴答中断之间的时间。

图 4.18 和图 4.19 演示了当固定任务时如何调度任务
采用优先级抢占式调度和时间分片算法。
图 4.18 显示了选择任务进入任务的顺序
当应用程序中的所有任务都具有唯一的时*运行*状态
优先。图 4.19 显示了选择任务的顺序
当应用程序中的两个任务共享一个任务时，进入*运行*状态
优先。


<a name="fig4.18" title="Figure 4.18 Execution pattern highlighting task prioritization and preemption..."></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.18_preemption_execution_pattern.png)
***图 4.18*** *强调任务优先级和抢占的执行模式
在一个假设的应用程序中，每个任务都被分配了一个唯一的
优先权*

***

参见图4.18：

- 空闲任务

空闲任务以最低优先级运行，因此它会被抢占
  每次更高优先级的任务进入*就绪*状态时，例如，
  时间t3、t5和t9。

- 任务3

任务 3 是一个事件驱动任务，执行速度相对较低
  优先级，但高于空闲优先级。它大部分时间都花在
  *阻塞*状态等待其感兴趣的事件，从
  每次事件发生时，*阻塞*状态都会变为*就绪*状态。全部
  FreeRTOS 任务间通信机制（任务通知、
  队列、信号量、事件组等）可用于发出事件信号并
  以这种方式解锁任务。

事件发生在时间 t3 和 t5，也发生在 t9 和 t12 之间的某个时间。
  立即处理在时间 t3 和 t5 发生的事件，因为
  此时，任务 3 是能够运行的最高优先级任务。
  时间 t9 和 t12 之间发生的事件不是
  直到 t12 才处理，因为在此之前，优先级较高的任务 Task
  1 和任务 2 仍在执行。仅在时间 t12 时，两个任务
  1 和任务 2 处于 *Blocked* 状态，使得任务 3 处于最高状态
  优先*就绪*状态任务。

- 任务2

任务 2 是一个周期性任务，其执行优先级高于优先级
  任务 3 的优先级，但低于任务 1 的优先级。任务的周期间隔
  表示任务 2 希望在时间 t1、t6 和 t9 执行。

在时间 t6，任务 3 处于 *Running* 状态，但任务 2 具有更高的
  相对优先级因此抢占任务 3 并立即开始执行。
  任务 2 完成处理并在某个时间重新进入 *Blocked* 状态
  t7，此时任务 3 可以重新进入 *Running* 状态以完成其任务
  处理。任务 3 本身在时间 t8 处阻塞。

- 任务1

任务1也是一个事件驱动的任务。它以最高的执行
  所有任务的优先级，因此可以抢占系统中的任何其他任务。唯一的
  显示的任务 1 事件发生在时间 t10，此时任务 1 抢占
  任务 2。任务 2 只有在任务 1 完成处理后才能完成其处理。
  在时间 t11 重新进入*阻塞*状态。


<a name="fig4.19" title="Figure 4.19 Execution pattern highlighting task prioritization and time slicing..."></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.19_time_slicing_execution_pattern.png)
***图 4.19*** *执行模式突出显示任务优先级和时间切片
在一个假设的应用程序中，两个任务以相同的优先级运行*

***

参见图4.19：

- 空闲任务和任务 2

Idle任务和Task 2都是连续处理任务，并且都
  优先级为 0（可能的最低优先级）。仅调度程序
  当没有任务时，将处理时间分配给优先级为 0 的任务
  能够运行更高优先级的任务，并共享所占用的时间
  通过时间分片分配给优先级0的任务。新的时间片
  在每个时钟周期中断开始，在图 4.19 中发生在时间 t1、t2、
  t3、t4、t5、t8、t9、t10 和 t11。

Idle任务和Task 2依次进入*Running*状态，可以
  导致两个任务在同一部分中都处于“正在运行”状态
  时间片，如时间 t5 和时间 t8 之间发生的情况。

- 任务1

任务1的优先级高于空闲优先级。任务 1 是一个
  事件驱动任务大部分时间处于“阻塞”状态
  等待其感兴趣的事件，从 *Blocked* 状态转换
  每次事件发生时进入*就绪*状态。

感兴趣的事件发生在时间 t6。在 t6 时，任务 1 变为
  能够运行的最高优先级任务，因此任务 1
  在时间片中抢占空闲任务的一部分。处理的
  事件在时间 t7 完成，此时任务 1 重新进入阻塞状态
  状态。

图4.19显示了Idle任务与一个任务共享处理时间
由应用程序编写者创建。分配那么多的处理时间
如果空闲优先级任务
应用程序编写者创建的任务有工作要做，但空闲任务
没有。 `configIDLE_SHOULD_YIELD` 编译时配置
常量可用于更改空闲任务的调度方式：

- 如果 `configIDLE_SHOULD_YIELD` 设置为 0，则空闲任务保持在
  整个时间片的 *Running* 状态，除非是
  被更高优先级的任务抢占。

- 如果 `configIDLE_SHOULD_YIELD` 设置为 1，则空闲任务 yields
  (voluntarily gives up whatever remains of its allocated time slice)
  在其循环的每次迭代中如果还有其他空闲优先级任务
  处于*就绪*状态。

图 4.19 所示的执行模式是当
`configIDLE_SHOULD_YIELD` 设置为 0。执行模式如图
图 4.20 是在相同场景中观察到的结果：
`configIDLE_SHOULD_YIELD` 设置为 1。


<a name="fig4.20" title="Figure 4.20 The execution pattern for the same scenario as shown in Figure 4.19..."></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.20_time_slicing_with_yield_execution_pattern.png)
***图4.20*** *相同场景的执行模式如图4.19所示，
但这次 `configIDLE_SHOULD_YIELD` 设置为 1*

***

图 4.20 还显示，当 `configIDLE_SHOULD_YIELD` 设置为 1 时，
空闲任务不选择后进入*运行*状态的任务
执行整个时间片，而是执行任意时间片
空闲任务产生的时间片的剩余时间。

### 4.12.4 无时间分片的优先抢占式调度

没有时间切片的优先抢占式调度保持了
与中描述的相同的任务选择和抢占算法
上一节，但不使用时间切片来共享处理时间
相同优先级的任务之间。

表 5 显示了配置 FreeRTOS 的 FreeRTOSConfig.h 设置
调度程序使用优先抢占式调度，无需时间切片。

如图4.19所示，如果使用时间切片，则有
多个处于最高优先级的就绪状态任务能够
run，然后调度程序选择一个新任务进入*Running*状态
在每个 RTOS 滴答中断期间（标记中断的结束
时间片）。如果不使用时间分片，则调度器仅
在以下任一情况下选择一个新任务进入*正在运行*状态：

- 更高优先级的任务进入*就绪*状态。

- 处于*运行*状态的任务进入*阻塞*或*挂起*状态。

不使用时间分片时任务上下文切换次数少于
当使用时间切片时。因此，关闭时间切片结果
减少调度程序的处理开销。然而，转
时间分割还可以导致同等优先级接收任务
处理时间量差异很大，该场景由
图 4.21。因此，运行调度程序时无需进行时间切片
被认为是一种先进技术，只能由以下人员使用
有经验的用户。


<a name="fig4.21" title="Figure 4.21 Execution pattern that demonstrates how tasks of equal priority can..."></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.21_equal_priority_without_time_slicing_execution_pattern.png)
***图 4.21*** *执行模式演示了相同优先级的任务如何执行
不使用时间切片时，收到的处理时间量差异很大*

***

参见图 4.21，假设 `configIDLE_SHOULD_YIELD` 设置为 0：

- 勾选中断

滴答中断发生在时间 t1、t2、t3、t4、t5、t8、t11、t12 和
  t13。

- 任务1

任务 1 是一个高优先级事件驱动任务，它花费了大部分时间
  处于*阻塞*状态等待其感兴趣的事件的时间。任务1
  从 *Blocked* 状态转换到 *Ready* 状态（并且
  随后，由于它是最高优先级的 *Ready* 状态任务，因此进入
  每次事件发生时的*运行*状态。图 4.21 显示任务 1
  在时间 t6 和 t7 之间处理事件，然后在时间之间再次处理
  t9 和 t10。

- 空闲任务和任务 2

Idle任务和Task 2都是连续处理任务，并且都
  优先级为 0（空闲优先级）。连续处理任务做
  不进入*阻止*状态。

未使用时间分片，因此空闲优先级任务位于
  *Running*状态将保持在*Running*状态，直到被抢占
  优先级较高的任务 1。

在图 4.21 中，空闲任务在时间 t1 开始运行，并保持在
  *运行*状态，直到在时间 t6 被任务 1 抢占，这更
  进入*运行*状态后超过四个完整的刻度周期。

任务 2 在时间 t7 开始运行，此时任务 1 重新进入
  *阻塞*状态等待另一个事件。任务 2 仍处于运行状态
  状态，直到它在时间 t9 也被任务 1 抢占，该时间小于
  进入*运行*状态后的一个周期。

在时间 t10 时，空闲任务重新进入 *Running* 状态，尽管
  已经收到比任务 2 多四倍多的处理时间。

### 4.12.5 协同调度

本书重点介绍抢占式调度，但FreeRTOS也可以使用
协同调度。表 5 显示 FreeRTOSConfig.h 设置
将 FreeRTOS 调度程序配置为使用协作调度。

当使用协作调度程序时（因此假设
应用程序提供的中断服务例程没有明确
请求上下文切换）上下文切换仅在运行时发生
状态任务显式进入*阻塞*状态，或*运行*状态任务
yields (manually requests a re-schedule) 通过调用 `taskYIELD()`。任务
永远不会被抢占，因此不能使用时间切片。

图 4.22 演示了协作调度程序的行为。的
图 4.22 中的水平虚线显示任务何时处于就绪状态
状态。


<a name="fig4.22" title="Figure 4.22 Execution pattern demonstrating the behavior of the cooperative scheduler"></a>

***
![](https://cloud.rocketpi.club/cloud/figure_4.22_cooperative_scheduler_execution_pattern.png)
***图 4.22*** *演示协作调度程序行为的执行模式*

***

参见图4.22：

- 任务1

任务 1 具有最高优先级。它从 *Blocked* 状态开始，
  等待信号量。

在时间 t3，中断给出信号量，导致任务 1 离开
  *阻塞*状态并进入*就绪*状态（从
  中断将在第 6 章中介绍）。

在时间 t3，任务 1 是最高优先级的 *Ready* 状态任务，并且如果
  已使用抢占式调度程序 任务 1 将成为正在运行
  状态任务。然而，当使用协作调度程序时，任务
  1 保持在 *Ready* 状态直到时间 t4，此时运行
  状态任务调用 `taskYIELD()`。

- 任务2

任务 2 的优先级介于任务 1 和任务 3 之间。
  处于 *Blocked* 状态，等待 Task 发送给它的消息
  3 在时间t2。

在时间 t2，任务 2 是最高优先级的 *Ready* 状态任务，并且如果
  已使用抢占式调度程序 任务 2 将成为正在运行
  状态任务。然而，当使用协作调度程序时，任务
  2 保持在 *Ready* 状态，直到 *Running* 状态任务
  进入*阻止*状态或调用 `taskYIELD()`。

*Running* 状态任务在时间 t4 调用 `taskYIELD()`，但到那时任务
  1 是最高优先级 *Ready* 状态任务，因此任务 2 不会
  实际上成为 *Running* 状态任务，直到任务 1 重新进入
  在时间 t5 处于*阻塞*状态。

在时间 t6 时，任务 2 重新进入 *Blocked* 状态以等待下一个任务
  消息，此时任务 3 再次成为最高优先级
  *就绪*状态任务。

在多任务应用程序中，应用程序编写者必须小心
资源不会同时被多个任务访问，如
同时访问可能会损坏资源。作为一个例子，考虑
以下场景，其中访问的资源是 UART（串行
端口）。两个任务将字符串写入 UART；任务1写
“abcdefghijklmnop”，任务 2 写入“123456789”：

1. 任务 1 处于 *Running* 状态并开始写入其字符串。它
   将“abcdefg”写入 UART，但之前保留 *Running* 状态
   写入任何其他字符。

1、任务2进入*运行*状态，向UART写入“123456789”，
   在离开*运行*状态之前。

1.任务1重新进入*Running*状态并写入剩余的
   其字符串的字符到 UART。

在这种情况下，实际写入 UART 的内容是
“abcdefg123456789hijklmnop”。任务 1 写入的字符串尚未被
按预期以不间断的顺序写入 UART，但相反
已损坏，因为任务 2 写入 UART 的字符串
出现在其中。

通常使用协作调度器可以更容易地避免
与使用抢占式访问相比，同时访问引起的问题
调度程序[^7]：

[^7]：涵盖了任务之间安全共享资源的方法
本书稍后会介绍。 FreeRTOS本身提供的资源，例如
队列和信号量始终可以安全地在任务之间共享。

- 当您使用抢占式调度程序时，*正在运行*状态任务可以是
  随时被抢占，包括与之共享资源时
  另一个任务处于不一致状态。正如刚才所证明的
  在 UART 示例中，使资源处于不一致状态可以
  导致数据损坏。

- 当您使用协作调度程序时，您可以控制何时切换到
  另一项任务发生。因此，您可以确保切换到另一个
  当资源处于不一致状态时，任务不会发生。

- 在上面的 UART 示例中，您可以确保任务 1 不会离开
  *运行*状态，直到将其整个字符串写入 UART
  并且，这样做可以消除字符串被损坏的可能性
  另一个任务的活动。

如图 4.22 所示，使用协作调度器使得
系统的响应速度比使用抢占式调度程序时要慢：

- 当使用抢占式调度程序时，调度程序开始运行
  当任务变为最高优先级时立即执行任务*就绪*
  状态任务。这在实时系统中通常是至关重要的，必须
  在规定的时间段内响应高优先级事件。

- 使用协作调度程序时，切换到具有
  成为最高优先级的*Ready*状态任务才执行
  *运行*状态任务进入*阻塞*状态或调用
  `taskYIELD()`。

