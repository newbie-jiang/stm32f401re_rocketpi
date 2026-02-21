# 8 资源管理

## 8.1 章简介和范围

在多任务系统中，如果启动一项任务，则可能会出现错误
访问资源，但在被访问之前未完成其访问
退出运行状态。如果任务离开资源
处于不一致状态，然后由任何其他资源访问同一资源
任务或中断可能导致数据损坏或其他类似的情况
问题。

以下是一些示例：

* 访问外设

考虑以下场景，其中两个任务尝试写入
  液晶Display (LCD)。

1.任务A执行并开始将字符串“Hello world”写入
     LCD。

2.任务A刚输出开头就被任务B抢占
     字符串的“Hello w”。

3. 任务 B 写入“中止、重试、失败？”进入之前先到 LCD
     封锁状态。

4. 任务 A 从被抢占的位置继续执行，并且
     完成输出其字符串的剩余字符——“orld”。

LCD 现在显示损坏的字符串“Hello wAbort，重试，失败？orld”。

* 读、修改、写操作

清单 8.1 显示了一行 C 代码，以及 C 代码如何执行的示例
  通常会被翻译成汇编代码。可见
  首先将PORTA的值从内存读入寄存器，并修改
  存入寄存器，然后写回内存。这被称为
  读、修改、写操作。


   <a name="list8.1" title="Listing 8.1 An example read, modify, write sequence"></a>

   ```c
   /* The C code being compiled. */
   PORTA |= 0x01;

   /* The assembly code produced when the C code is compiled. */
   LOAD  R1,[#PORTA] ; Read a value from PORTA into R1
   MOVE  R2,#0x01    ; Move the absolute constant 1 into R2
   OR    R1,R2       ; Bitwise OR R1 (PORTA) with R2 (constant 1)
   STORE R1,[#PORTA] ; Store the new value back to PORTA
   ```
***清单 8.1*** *读取、修改、写入序列示例*

这是一种“非原子”操作，因为它需要多个操作
   指令完成，并且可以被中断。考虑以下几点
   两个任务尝试更新内存映射寄存器的场景
   称为 PORTA。

1. 任务 A 将 PORTA 的值加载到寄存器中，即 PORTA 的读取部分
      操作。

2.任务A在完成修改之前被任务B抢占，
      写入同一操作的部分。

3. 任务B更新PORTA的值，然后进入Blocked状态。

4. 任务 A 从被抢占的位置继续执行。它
      修改它已保存在 a 中的 PORTA 值的副本
      寄存器，然后将更新后的值写回 PORTA。

在这种情况下，任务 A 更新并写回过期值
   PORTA。任务 A 获取 PORTA 的副本后，任务 B 修改 PORTA
   值，并且在任务 A 将修改后的值写回 PORTA 之前
   注册。当任务A写入PORTA时，它会覆盖修改
   任务 B 已经执行过，有效地破坏了
   PORTA 寄存器值。

本例使用外设寄存器，但原理相同
   对变量执行读取、修改、写入操作时。

- 对变量的非原子访问

更新结构体的多个成员，或者更新一个变量
  大于架构的自然字大小（例如，
  在 16 位机器上更新 32 位变量），是以下示例
  非原子操作。如果它们被中断，它们可能会导致数据
  损失或腐败。

- 函数重入

如果从 more 调用该函数是安全的，则该函数是“可重入”的
  多于一项任务，或同时来自任务和中断。可重入函数
  据说是“线程安全”的，因为它们可以从多个地方访问
  一个执行线程，没有数据或逻辑操作的风险
  变得腐败。

每个任务维护自己的堆栈和自己的一组 processor
  (hardware) 寄存器值。如果函数不访问其他任何数据
  比存储在堆栈上或保存在寄存器中的数据，则该函数
  是可重入的，并且是线程安全的。清单 8.2 是一个可重入的例子
  功能。清单 8.3 是不可重入函数的示例。

如果应用程序使用 newlib C 库，则必须将 `configUSE_NEWLIB_REENTRANT` 设置为 1
  在 FreeRTOSConfig.h 中，以确保 newlib 所需的线程本地存储是
  分配正确。

如果应用程序使用 picolibc C 库，则必须将 `configUSE_PICOLIBC_TLS` 设置为 1
  FreeRTOSConfig.h 确保 picolibc 所需的线程本地存储是
  分配正确。

如果应用程序使用任何其他 C 库并且需要线程本地 Storage (TLS)，则它
  必须在 FreeRTOSConfig.h 中将 `configUSE_C_RUNTIME_TLS_SUPPORT` 设置为 1，并且必须实施
  以下宏-
  - `configTLS_BLOCK_TYPE` - 每个任务 TLS 块的类型。
  - `configINIT_TLS_BLOCK` - 初始化每个任务 TLS 块。
  - `configSET_TLS_BLOCK` - 更新当前的 TLS 块。在上下文切换期间调用以确保
    使用正确的 TLS 块。
  - `configDEINIT_TLS_BLOCK` - 释放 TLS 块。


  <a name="list8.2" title="Listing 8.2 An example of a reentrant function"></a>

  ```c
  /* A parameter is passed into the function. This will either be passed on the 
     stack, or in a processor register. Either way is safe as each task or 
     interrupt that calls the function maintains its own stack and its own set 
     of register values, so each task or interrupt that calls the function will 
     have its own copy of lVar1. */
  long lAddOneHundred( long lVar1 )
  {
      /* This function scope variable will also be allocated to the stack or a 
         register, depending on the compiler and optimization level. Each task
         or interrupt that calls this function will have its own copy of lVar2. */
      long lVar2;

      lVar2 = lVar1 + 100;
      return lVar2;
  }
  ```
***清单 8.2*** *可重入函数的示例*


  <a name="list8.3" title="Listing 8.3 An example of a function that is not reentrant"></a>

  ```c
  /* In this case lVar1 is a global variable, so every task that calls
     lNonsenseFunction will access the same single copy of the variable. */
  long lVar1;

  long lNonsenseFunction( void )
  {
      /* lState is static, so is not allocated on the stack. Each task that
         calls this function will access the same single copy of the variable. */
      static long lState = 0;
      long lReturn;

      switch( lState )
      {
          case 0 : lReturn = lVar1 + 10;
                   lState = 1;
                   break;

          case 1 : lReturn = lVar1 + 20;
                   lState = 0;
                   break;
      }
  }
  ```
***清单 8.3*** *不可重入函数的示例*


### 8.1.1 相互排斥

为了确保始终保持数据一致性，访问
任务之间共享的资源，或者任务和中断之间共享的资源，
必须使用“互斥”技术进行管理。目标是
确保一旦任务开始访问共享资源，
可重入且非线程安全，同一任务具有独占访问权
直到资源返回到一致状态。

FreeRTOS 提供了多种可用于实现交互的功能
排除，但最好的互斥方法是（每当
可能，因为这通常不切实际）以这样的方式设计应用程序
资源不共享，每个资源只能访问
来自单个任务。


### 8.1.2 范围

本章内容包括：

- 何时以及为什么需要资源管理和控制。
- 什么是关键部分。
- 相互排斥是什么意思。
- 暂停调度程序意味着什么。
- 如何使用互斥锁。
- 如何创建和使用网守任务。
- 什么是优先级反转，以及优先级继承如何影响 reduce
  (but not remove)。


## 8.2 关键部分和暂停调度程序

### 8.2.1 基本关键部分

基本临界区是被调用包围的代码区域
到宏 `taskENTER_CRITICAL()` 和 `taskEXIT_CRITICAL()`，
分别。关键部分也称为关键区域。

`taskENTER_CRITICAL()`和`taskEXIT_CRITICAL()`不带任何参数，
或返回一个值[^23]。它们的使用如清单 8.4 所示。

[^23]：类似函数的宏并不真正在
与实函数的做法相同。本书应用该术语
当最简单地想到宏时，向宏“返回一个值”
就好像它是一个函数一样。


<a name="list8.4" title="Listing 8.4 Using a critical section to guard access to a register"></a>

```c
/* Ensure access to the PORTA register cannot be interrupted by placing
   it within a critical section. Enter the critical section. */
taskENTER_CRITICAL();

/* A switch to another task cannot occur between the call to
   taskENTER_CRITICAL() and the call to taskEXIT_CRITICAL(). Interrupts may
   still execute on FreeRTOS ports that allow interrupt nesting, but only
   interrupts whose logical priority is above the value assigned to the
   configMAX_SYSCALL_INTERRUPT_PRIORITY constant – and those interrupts are
   not permitted to call FreeRTOS API functions. */
PORTA |= 0x01;

/* Access to PORTA has finished, so it is safe to exit the critical section. */
taskEXIT_CRITICAL();
```
***清单 8.4*** *使用临界区来保护对寄存器的访问*


本书附带的示例项目使用了一个名为
`vPrintString()` 将字符串写入标准输出，这是终端
使用 FreeRTOS Windows 端口时的窗口。 `vPrintString()` 称为
来自许多不同的任务；因此，从理论上讲，它的实施可以
使用关键部分保护对标准输出的访问，如下所示
清单 8.5。


<a name="list8.5" title="Listing 8.5 A possible implementation of vPrintString()"></a>

```c
void vPrintString( const char *pcString )
{
    /* Write the string to stdout, using a critical section as a crude method of
       mutual exclusion. */
    taskENTER_CRITICAL();
    {
        printf( "%s", pcString );
        fflush( stdout );
    }
    taskEXIT_CRITICAL();
}
```
***清单 8.5*** *vPrintString() 的可能实现*


以这种方式实现的临界区是一种非常粗暴的方法
提供互斥。它们通过禁用中断来工作
完全，或达到由设置的中断优先级
`configMAX_SYSCALL_INTERRUPT_PRIORITY`，取决于 FreeRTOS 端口
被使用。抢占式上下文切换只能发生在
中断，因此，只要中断保持禁用状态，任务
称为 `taskENTER_CRITICAL()` 保证保持在运行状态
直到退出临界区。

基本关键部分必须保持非常短，否则它们将
对中断响应时间产生不利影响。每次致电
`taskENTER_CRITICAL()` 必须与调用紧密配对
`taskEXIT_CRITICAL()`。因此，标准输出（stdout，或
计算机写入输出数据的流）不应受到保护
使用关键的 section (as shown in Listing 8.5)，因为写入
终端操作可能会比较长。本节中的例子
本章探讨替代解决方案。

关键部分嵌套是安全的，因为内核
记录嵌套深度。临界区将被退出
仅当嵌套深度返回到零时，即一次调用
`taskEXIT_CRITICAL()` 已为之前的每个调用执行
`taskENTER_CRITICAL()`。

调用 `taskENTER_CRITICAL()` 和 `taskEXIT_CRITICAL()` 是唯一的
任务改变中断使能状态的合法方式
运行 FreeRTOS 的处理器。改变中断使能
通过任何其他方式更改状态都会使宏的嵌套计数无效。

`taskENTER_CRITICAL()` 和 `taskEXIT_CRITICAL()` 不以“FromISR”结尾，因此
不得从中断服务程序中调用。
`taskENTER_CRITICAL_FROM_ISR()` 是中断安全版本
`taskENTER_CRITICAL()`和`taskEXIT_CRITICAL_FROM_ISR()`是中断
`taskEXIT_CRITICAL()` 的安全版本。中断安全版本是
仅提供给允许中断嵌套的 FreeRTOS 端口——它们
在不允许中断嵌套的端口中将被废弃。

`taskENTER_CRITICAL_FROM_ISR()` 返回一个必须传入的值
对 `taskEXIT_CRITICAL_FROM_ISR()` 的匹配调用。这被证明
清单 8.6 中。


<a name="list8.6" title="Listing 8.6 Using a critical section in an interrupt service routine"></a>

```c
void vAnInterruptServiceRoutine( void )
{
    /* Declare a variable in which the return value from 
       taskENTER_CRITICAL_FROM_ISR() will be saved. */
    UBaseType_t uxSavedInterruptStatus;

    /* This part of the ISR can be interrupted by any higher priority 
       interrupt. */

    /* Use taskENTER_CRITICAL_FROM_ISR() to protect a region of this ISR.
       Save the value returned from taskENTER_CRITICAL_FROM_ISR() so it can 
       be passed into the matching call to taskEXIT_CRITICAL_FROM_ISR(). */
    uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();

    /* This part of the ISR is between the call to 
       taskENTER_CRITICAL_FROM_ISR() and taskEXIT_CRITICAL_FROM_ISR(), so can 
       only be interrupted by interrupts that have a priority above that set 
       by the configMAX_SYSCALL_INTERRUPT_PRIORITY constant. */

    /* Exit the critical section again by calling taskEXIT_CRITICAL_FROM_ISR(),
       passing in the value returned by the matching call to 
       taskENTER_CRITICAL_FROM_ISR(). */
    taskEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus );

    /* This part of the ISR can be interrupted by any higher priority 
       interrupt. */
}
```
***清单 8.6*** *在中断服务程序中使用临界区*


使用更多的处理时间来执行代码是浪费的
进入临界区，然后退出，然后执行
代码实际上受到关键部分的保护。基础
临界区进入、退出都非常快，而且总是
确定性，当代码区域被确定时，它们的使用非常理想
受保护的时间很短。


### 8.2.2 Suspending (or Locking) 调度程序

还可以通过暂停调度程序来创建关键部分。
挂起调度程序有时也称为“锁定”
调度程序。

基本临界区保护代码区域不被其他人访问
任务和中断，但关键部分是通过挂起实现的
调度程序仅保护代码区域不被其他任务访问，
因为中断保持启用状态。

关键部分太长，无法简单地实现
相反，可以通过暂停来实现禁用中断
调度程序。但是，在调度程序挂起时中断活动
可以使resuming (or 'un-suspending')调度器比较长
操作，因此必须考虑哪种方法是最好的
在每种情况下使用。


### 8.2.3 vTaskSuspendAll() API 功能


<a name="list8.7" title="Listing 8.7 The vTaskSuspendAll() API function prototype"></a>

```c
void vTaskSuspendAll( void );
```
***清单 8.7*** *vTaskSuspendAll() API 函数原型*


通过调用 `vTaskSuspendAll()` 暂停调度程序。暂停
调度程序会阻止上下文切换的发生，但会留下
中断使能。如果中断请求上下文切换，而
调度程序被挂起，然后请求被挂起，并且
仅当调度程序为 resumed (un-suspended) 时执行。

当调度程序挂起时，不得调用 FreeRTOS API 函数。


### 8.2.4 xTaskResumeAll() API 功能


<a name="list8.8" title="Listing 8.8 The xTaskResumeAll() API function prototype"></a>

```c
BaseType_t xTaskResumeAll( void );
```
***清单 8.8*** *xTaskResumeAll() API 函数原型*


通过调用 `xTaskResumeAll()`，调度器为 resumed (un-suspended)。

**xTaskResumeAll() 返回值**

- 返回值

调度程序挂起时请求的上下文切换将被挂起并执行 
  仅当调度程序正在恢复时。如果在 `xTaskResumeAll()` 之前执行了挂起的上下文切换 
  返回，则返回 `pdTRUE`。否则返回 `pdFALSE`。

对 `vTaskSuspendAll()` 和 `xTaskResumeAll()` 的调用可以安全地变为
嵌套，因为内核保留了嵌套深度的计数。的
仅当嵌套深度返回到时调度程序才会恢复
零——即对 `xTaskResumeAll()` 的一次调用已执行了
之前对 `vTaskSuspendAll()` 的每次调用。

清单 8.9 显示了 `vPrintString()` 的实际实现，其中
挂起调度程序以保护对终端输出的访问。


<a name="list8.9" title="Listing 8.9 The implementation of vPrintString()"></a>

```c
void vPrintString( const char *pcString )
{
    /* Write the string to stdout, suspending the scheduler as a method of 
       mutual exclusion. */
    vTaskSuspendScheduler();
    {
        printf( "%s", pcString );
        fflush( stdout );
    }
    xTaskResumeScheduler();
}
```
***清单 8.9*** *vPrintString() 的实施*


## 8.3 Mutexes (and Binary Semaphores)

互斥体是一种特殊类型的二进制信号量，用于控制
访问两个或多个任务之间共享的资源。这个词
MUTEX 源自“相互排斥”。必须设置 `configUSE_MUTEXES`
FreeRTOSConfig.h 中的 1 以使互斥体可用。

当在互斥场景中使用时，互斥体可以被认为是
与共享资源关联的令牌。对于一个任务
合法地访问资源，首先必须成功“获取”
token (be the token holder)。当代币持有者完成后
资源，它必须“返还”令牌。仅当令牌已被
返回是否可以让另一个任务成功拿走token，然后安全地
访问相同的共享资源。不允许任务访问
共享资源，除非它持有令牌。该机制显示在
图 8.1。

尽管互斥体和二进制信号量有许多共同特征，
图 8.1 所示的场景（其中互斥量用于相互
排除）与图 7.6 所示完全不同（其中
二进制信号量用于同步）。主要区别是
获得信号量后会发生什么：

- 用于互斥的信号量必须始终是
  回来了。
- 用于同步的信号量通常会被丢弃
  并且没有返回。


<a name="fig8.1" title="Figure 8.1 Mutual exclusion implemented using a mutex"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image63.png)   
***图 8.1*** *使用互斥体实现互斥*

* * *

该机制纯粹通过应用程序的规则来工作
作家。任务没有理由无法随时访问资源
时间，但每个任务“同意”不这样做，除非它能够成为
互斥锁持有者。


### 8.3.1 xSemaphoreCreateMutex() API 功能

FreeRTOS 还包括 `xSemaphoreCreateMutexStatic()`
函数，它分配创建互斥体所需的内存
在编译时静态：互斥锁是信号量的一种。处理至
所有各种类型的 FreeRTOS 信号量都存储在变量中
型号 `SemaphoreHandle_t`。

在使用互斥体之前，必须先创建它。创建互斥体类型
信号量，使用 `xSemaphoreCreateMutex()` API 函数。


<a name="list8.10" title="Listing 8.10 The xSemaphoreCreateMutex() API function prototype"></a>

```c
SemaphoreHandle_t xSemaphoreCreateMutex( void );
```
***清单 8.10*** *xSemaphoreCreateMutex() API 功能原型*


**xSemaphoreCreateMutex() 返回值**

- 返回值

如果返回 NULL，则无法创建互斥体，因为
  FreeRTOS 没有足够的堆内存来分配
  互斥数据结构。第 3 章提供有关堆的更多信息
  内存管理。

非 NULL 返回值表示互斥体已创建
  成功。返回值应存储为句柄
  创建了互斥体。


<a name="example8.1" title="Example 8.1 Rewriting vPrintString() to use a semaphore"></a>
---
***示例 8.1*** *重写 vPrintString() 以使用信号量*

---

此示例创建 `vPrintString()` 的新版本，名为
`prvNewPrintString()`，然后从多个任务调用新函数。
`prvNewPrintString()` 在功能上与 `vPrintString()` 相同，但是
使用互斥锁控制对标准输出的访问，而不是通过锁定
调度程序。 `prvNewPrintString()` 的实现如图所示
清单 8.11。


<a name="list8.11" title="Listing 8.11 The implementation of prvNewPrintString()"></a>

```c
static void prvNewPrintString( const char *pcString )
{
    /* The mutex is created before the scheduler is started, so already exists
       by the time this task executes.

       Attempt to take the mutex, blocking indefinitely to wait for the mutex
       if it is not available straight away. The call to xSemaphoreTake() will
       only return when the mutex has been successfully obtained, so there is 
       no need to check the function return value. If any other delay period 
       was used then the code must check that xSemaphoreTake() returns pdTRUE 
       before accessing the shared resource (which in this case is standard 
       out). As noted earlier in this book, indefinite time outs are not 
       recommended for production code. */
    xSemaphoreTake( xMutex, portMAX_DELAY );
    {
        /* The following line will only execute once the mutex has been 
           successfully obtained. Standard out can be accessed freely now as 
           only one task can have the mutex at any one time. */
        printf( "%s", pcString );
        fflush( stdout );

        /* The mutex MUST be given back! */
    }
    xSemaphoreGive( xMutex );
}
```
***清单 8.11*** *prvNewPrintString() 的实现*


`prvNewPrintString()` 被任务的两个实例重复调用
由 `prvPrintTask()` 实施。每个之间使用随机延迟时间
打电话。任务参数用于将唯一的字符串传递给每个任务
任务的实例。 `prvPrintTask()`的实现如图所示
清单 8.12。


<a name="list8.12" title="Listing 8.12 The implementation of prvPrintTask() for Example 8.1"></a>

```c
static void prvPrintTask( void *pvParameters )
{
    char *pcStringToPrint;
    const TickType_t xMaxBlockTimeTicks = 0x20;

    /* Two instances of this task are created. The string printed by the task 
       is passed into the task using the task's parameter. The parameter is 
       cast to the required type. */
    pcStringToPrint = ( char * ) pvParameters;

    for( ;; )
    {
        /* Print out the string using the newly defined function. */
        prvNewPrintString( pcStringToPrint );

        /* Wait a pseudo random time. Note that rand() is not necessarily
           reentrant, but in this case it does not really matter as the code 
           does not care what value is returned. In a more secure application 
           a version of rand() that is known to be reentrant should be used - 
           or calls to rand() should be protected using a critical section. */
        vTaskDelay( ( rand() % xMaxBlockTimeTicks ) );
    }
}
```
***清单 8.12*** *示例 8.1 的 prvPrintTask() 的实现*


正常情况下，`main()` 只是创建互斥体，创建任务，然后
启动调度程序。其实现如清单 8.13 所示。

`prvPrintTask()` 的两个实例以不同的优先级创建，
因此较低优先级的任务有时会被较高优先级的任务抢占
优先任务。由于互斥体用于确保每个任务相互获取
对终端的独占访问，即使发生抢占，
显示的字符串将是正确的并且绝不会损坏。的
可以通过减少最大时间来增加抢占频率
任务处于阻塞状态，该状态由
`xMaxBlockTimeTicks` 常数。

将示例 8.1 与 FreeRTOS Windows 端口一起使用的具体注意事项：

- 调用 `printf()` 会生成 Windows 系统调用。 Windows系统
  调用不受 FreeRTOS 的控制，并且可能会引入
  不稳定。

- Windows系统调用的执行方式意味着很少见
  即使未使用互斥体，也会出现损坏的字符串。


<a name="list8.13" title="Listing 8.13 The implementation of main() for Example 8.1"></a>

```c
int main( void )
{
    /* Before a semaphore is used it must be explicitly created. In this
       example a mutex type semaphore is created. */
    xMutex = xSemaphoreCreateMutex();

    /* Check the semaphore was created successfully before creating the
       tasks. */
    if( xMutex != NULL )
    {
        /* Create two instances of the tasks that write to stdout. The string
           they write is passed in to the task as the task's parameter. The 
           tasks are created at different priorities so some pre-emption will 
           occur. */
        xTaskCreate( prvPrintTask, "Print1", 1000,
                     "Task 1 ***************************************\r\n",
                     1, NULL );

        xTaskCreate( prvPrintTask, "Print2", 1000,
                     "Task 2 ---------------------------------------\r\n", 
                     2, NULL );

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    }

    /* If all is well then main() will never reach here as the scheduler will
       now be running the tasks. If main() does reach here then it is likely 
       that there was insufficient heap memory available for the idle task to 
       be created.  Chapter 3 provides more information on heap memory 
       management. */
    for( ;; );
}
```
***清单 8.13*** *示例 8.1 的 main() 的实现*


执行例 8.1 时产生的输出如图 8.2 所示。一个
可能的执行顺序如图 8.3 所示。

<a name="fig8.2" title="Figure 8.2 The output produced when Example 8.1 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image64.jpg)   
***图 8.2*** *执行例 8.1 时产生的输出*

* * *

图 8.2 显示，正如预期的那样，字符串中没有损坏
终端上显示的内容。随机排序的结果是
任务使用的随机延迟周期。


<a name="fig8.3" title="Figure 8.3 A possible sequence of execution for Example 8.1"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image65.png)   
***图 8.3*** *示例 8.1 的可能执行顺序*

* * *


### 8.3.2 优先级反转

图 8.3 演示了使用互斥体的潜在缺陷之一
提供互斥。所描述的执行顺序显示了
较高优先级的任务 2 必须等待较低优先级的任务 1
放弃对互斥体的控制。更高优先级的任务被延迟
以这种方式处理较低优先级任务的方式称为“优先级反转”。这个
如果中等优先级，不良行为将会被进一步夸大
当高优先级任务正在等待时，任务开始执行
信号量——结果将是一个高优先级任务等待一个低优先级任务
优先级任务——低优先级任务甚至无法执行。
这通常被称为 _无界优先级 inversion_，因为
中优先级任务可能会阻塞低优先级和高优先级任务
无限期地。
最坏的情况如图 8.4 所示。


<a name="fig8.4" title="Figure 8.4 A worst case priority inversion scenario"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image66.png)   
***图 8.4*** *最坏情况优先级反转场景*

* * *

优先级反转可能是一个严重的问题，但在小型嵌入式中
系统通常可以在系统设计时通过考虑来避免
如何访问资源。


### 8.3.3 优先级继承

FreeRTOS 互斥体和二进制信号量非常相似——区别
因为互斥体包括基本的“优先级继承”机制，
而二进制信号量则不然。优先继承是一种方案
最大限度地减少优先级反转的负面影响。它不会“修复”
优先级倒置，但仅仅通过确保
反转总是有时间限制的。但优先继承
使系统时序分析变得复杂，并且依赖于
正确的系统操作。

优先级继承的工作原理是暂时提高优先级
互斥持有者的优先级为最高优先级任务的优先级
尝试获取相同的互斥锁。持有的低优先级任务
互斥锁“继承”等待互斥锁的任务的优先级。
图 8.5 展示了这一点。互斥持有者的优先级是
当它返回互斥锁时，会自动重置为其原始值。


<a name="fig8.5" title="Figure 8.5 Priority inheritance minimizing the effect of priority inversion"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image67.png)   
***图 8.5*** *优先级继承最小化优先级反转的影响*

* * *

正如刚才所看到的，优先级继承功能会影响
使用互斥体的任务。因此，互斥体不能
从中断服务程序中使用。

FreeRTOS 实现了基本的优先级继承机制
设计时考虑到优化空间和执行周期。一个完整的
优先级继承机制需要更多的数据和
处理器周期随时确定继承的优先级，
特别是当一项任务一次持有多个互斥锁时。

要记住的优先级继承机制的具体行为：
* 任务可以进一步提高其继承的优先级，如果
  获取互斥体而不首先释放它已经持有的互斥体。
* 任务保持其最高继承优先级直到
  它已经释放了它持有的所有互斥体。这是无论
  互斥锁的释放顺序。
* 如果有多个任务，则该任务将保持最高继承优先级
  无论任务等待任何持有的互斥体，互斥体都会被持有
  互斥体完成其 wait (timing out)。


### 8.3.4 Deadlock (or Deadly Embrace)

“死锁”是使用互斥体进行互斥的另一个潜在陷阱
排除。死锁有时也有一个更戏剧化的名字
‘致命的拥抱’。

当两个任务都无法继续执行时，就会发生死锁
等待对方持有的资源。考虑以下几点
任务 A 和任务 B 都需要获取互斥体 X *和* 的场景
互斥量 Y 以便执行操作：

1. 任务A执行并成功获取互斥体X。

2. 任务A被任务B抢占。

3. 任务 B 在尝试获取互斥体 Y 之前成功获取互斥体 Y
   互斥体 X — 但互斥体 X 由任务 A 持有，因此任务 B 无法使用。
   任务B选择进入阻塞状态，等待互斥体X被
   释放。

4. 任务A 继续执行。它尝试获取互斥量 Y，但是互斥量 Y
   由任务 B 持有，因此对任务 A 不可用。任务 A 选择
   进入阻塞状态等待互斥量Y被释放。

在这个场景结束时，任务 A 正在等待任务持有的互斥体
B，任务 B 正在等待任务 A 持有的互斥体。死锁
发生的原因是这两项任务都无法继续进行。

与优先级反转一样，避免死锁的最佳方法是
在设计时考虑其潜力，并设计系统以确保
死锁不会发生。特别是，正如前面所述
在本书中，任务等待 indefinitely
(without a time out) 获取互斥体通常是不好的做法。相反，使用超时时间
比预计需要等待的最长时间稍长一些
互斥体——那么在这段时间内未能获得互斥体将是
设计错误的症状，可能是死锁。

实际上，死锁在小型嵌入式系统中并不是一个大问题，
因为系统设计者可以对整个系统有一个很好的了解
应用程序，因此可以识别并删除可能存在的区域
发生。


### 8.3.5 递归互斥体

任务也有可能与自身发生死锁。这将会发生
如果一个任务尝试多次获取相同的互斥锁，而没有首先
返回互斥体。考虑以下场景：

1. 任务成功获取互斥锁。

2. 在持有互斥体的同时，任务调用库函数。

3.库函数的实现尝试采取相同的
   mutex，并进入Blocked状态，等待互斥体变为
   可用。

在此场景结束时，任务处于阻塞状态以等待
要返回的互斥锁，但任务已经是互斥锁持有者。一个
由于任务处于Blocked状态等待而发生死锁
为了自己。

通过使用递归互斥体可以避免这种类型的死锁
标准互斥锁。递归互斥体可以被多次“获取”
相同的任务，并且仅在一次调用“give”后才会返回
递归互斥锁已在之前的每次调用“take”时执行
递归互斥体。

标准互斥体和递归互斥体的创建和使用方式类似
方式：

- 标准互斥体是使用 `xSemaphoreCreateMutex()` 创建的。
  递归互斥体是使用创建的
  `xSemaphoreCreateRecursiveMutex()`。两个 API 函数具有
  相同的原型。

- 使用 `xSemaphoreTake()`“获取”标准互斥体。递归
  互斥体是使用 `xSemaphoreTakeRecursive()`“获取”的。两个 API
  函数具有相同的原型。

- 标准互斥体是使用 `xSemaphoreGive()`“给出”的。递归
  互斥体是使用 `xSemaphoreGiveRecursive()`“给出”的。两个 API
  函数具有相同的原型。

清单 8.14 演示了如何创建和使用递归互斥体。


<a name="list8.14" title="Listing 8.14 Creating and using a recursive mutex"></a>

```c
/* Recursive mutexes are variables of type SemaphoreHandle_t. */
SemaphoreHandle_t xRecursiveMutex;

/* The implementation of a task that creates and uses a recursive mutex. */
void vTaskFunction( void *pvParameters )
{
    const TickType_t xMaxBlock20ms = pdMS_TO_TICKS( 20 );

    /* Before a recursive mutex is used it must be explicitly created. */
    xRecursiveMutex = xSemaphoreCreateRecursiveMutex();

    /* Check the semaphore was created successfully. configASSERT() is 
       described in section 11.2. */
    configASSERT( xRecursiveMutex );

    /* As per most tasks, this task is implemented as an infinite loop. */
    for( ;; )
    {
        /* ... */

        /* Take the recursive mutex. */
        if( xSemaphoreTakeRecursive( xRecursiveMutex, xMaxBlock20ms ) == pdPASS )
        {
            /* The recursive mutex was successfully obtained. The task can now
               access the resource the mutex is protecting. At this point the 
               recursive call count (which is the number of nested calls to 
               xSemaphoreTakeRecursive()) is 1, as the recursive mutex has 
               only been taken once. */

            /* While it already holds the recursive mutex, the task takes the 
               mutex again. In a real application, this is only likely to occur
               inside a sub-function called by this task, as there is no 
               practical reason to knowingly take the same mutex more than 
               once. The calling task is already the mutex holder, so the 
               second call to xSemaphoreTakeRecursive() does nothing more than
               increment the recursive call count to 2. */
            xSemaphoreTakeRecursive( xRecursiveMutex, xMaxBlock20ms );

            /* ... */

            /* The task returns the mutex after it has finished accessing the
               resource the mutex is protecting. At this point the recursive 
               call count is 2, so the first call to xSemaphoreGiveRecursive()
               does not return the mutex. Instead, it simply decrements the 
               recursive call count back to 1. */
            xSemaphoreGiveRecursive( xRecursiveMutex );

            /* The next call to xSemaphoreGiveRecursive() decrements the 
               recursive call count to 0, so this time the recursive mutex is 
               returned. */
            xSemaphoreGiveRecursive( xRecursiveMutex );

            /* Now one call to xSemaphoreGiveRecursive() has been executed for
               every proceeding call to xSemaphoreTakeRecursive(), so the task
               is no longer the mutex holder. */
        }
    }
}
```
***清单 8.14*** *创建和使用递归互斥体*


### 8.3.6 互斥体和任务调度

如果两个不同优先级的任务使用相同的互斥锁，那么 FreeRTOS
调度策略使任务执行的顺序清晰；
能够运行的最高优先级任务将被选择为
进入运行状态的任务。例如，如果一个高优先级任务
处于阻塞状态以等待由低电平持有的互斥体
优先级任务，则高优先级任务将抢占低优先级任务
一旦低优先级任务返回互斥锁，优先级任务就会被执行。的
高优先级任务将成为互斥锁持有者。这个场景有
已经在图 8.5 中看到。

然而，对顺序做出错误的假设是很常见的。
当任务具有相同优先级时，任务将执行。如果
任务1和任务2优先级相同，任务1处于Blocked状态
状态等待任务 2 持有的互斥锁，那么任务 1 将不会
当任务 2“给出”互斥体时抢占任务 2。相反，任务 2 将
保持在运行状态，并且任务 1 将简单地从
阻塞状态转为就绪状态。这种情况如图8.6所示，
其中垂直线标记刻度中断的时间
发生。


<a name="fig8.6" title="Figure 8.6 A possible sequence of execution when tasks that have the same priority use the same mutex"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image68.png)   
***图 8.6*** *当具有相同优先级的任务使用相同的互斥体时可能的执行顺序*

* * *

在图 8.6 所示的场景中，FreeRTOS 调度程序*不*
一旦互斥锁可用，任务 1 就成为运行状态任务
因为：

- 任务 1 和任务 2 具有相同的优先级，因此除非任务 2 进入
  在阻塞状态下，不应切换到任务 1，直到
  下一个时钟周期中断（假设 `configUSE_TIME_SLICING` 设置为 1）
  FreeRTOSConfig.h）。

- 如果任务在紧密循环中使用互斥体以及上下文切换
  每次任务“给出”互斥锁时都会发生，然后任务将
  只会在短时间内保持运行状态。如果两个或
  更多任务在紧密循环中使用相同的互斥体，然后处理时间
  任务之间的快速切换会造成浪费。

如果一个互斥锁被多个任务在紧密循环中使用，并且这些任务
使用互斥体的优先级相同，那么必须注意
确保任务获得大致相等的处理量
时间。任务可能未收到同等数量的原因
图 8.7 展示了处理时间，其中显示了一系列
如果列表中显示的任务有两个实例，则可能发生执行
8.15 以相同的优先级创建。


<a name="list8.15" title="Listing 8.15 A task that uses a mutex in a tight loop"></a>

```c
/* The implementation of a task that uses a mutex in a tight loop. The task 
   creates a text string in a local buffer, then writes the string to a display.
   Access to the display is protected by a mutex. */

void vATask( void *pvParameter )
{
    extern SemaphoreHandle_t xMutex;
    char cTextBuffer[ 128 ];

    for( ;; )
    {
        /* Generate the text string – this is a fast operation. */
        vGenerateTextInALocalBuffer( cTextBuffer );

        /* Obtain the mutex that is protecting access to the display. */
        xSemaphoreTake( xMutex, portMAX_DELAY );

        /* Write the generated text to the display–this is a slow operation. */
        vCopyTextToFrameBuffer( cTextBuffer );

        /* The text has been written to the display, so return the mutex. */
        xSemaphoreGive( xMutex );
    }
}
```
***清单 8.15*** *在紧密循环中使用互斥锁的任务*


清单 8.15 中的注释指出创建字符串非常快
操作，并且更新显示是一个缓慢的操作。因此，作为
更新显示时保持互斥锁，任务将保持
在其大部分运行时间内互斥。

在图 8.7 中，垂直线标记了刻度的时间
发生中断。


<a name="fig8.7" title="Figure 8.7 A sequence of execution that could occur if two instances of the task shown by Listing 8.15 are created at the same priority"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image69.png)   
***图 8.7*** *如果清单 8.15 所示的任务的两个实例以相同的优先级创建，则可能发生的执行顺序*

* * *

图 8.7 中的步骤 7 显示任务 1 重新进入阻塞状态——即
发生在 `xSemaphoreTake()` API 函数内部。

图 8.7 表明任务 1 将无法获得
互斥，直到时间片的开始与短时间片之一重合
任务 2 不是互斥锁持有者的时期。

通过添加对以下内容的调用可以避免图 8.7 中所示的情况
调用 `xSemaphoreGive()` 后的 `taskYIELD()`。这体现在
清单 8.16，其中如果滴答计数发生变化，则调用 `taskYIELD()`
该任务持有互斥锁。


<a name="list8.16" title="Listing 8.16 Ensuring tasks that use a mutex in a loop receive a more equal amount of processing time..."></a>

```c
void vFunction( void *pvParameter )
{
    extern SemaphoreHandle_t xMutex;
    char cTextBuffer[ 128 ];
    TickType_t xTimeAtWhichMutexWasTaken;

    for( ;; )
    {
        /* Generate the text string – this is a fast operation. */
        vGenerateTextInALocalBuffer( cTextBuffer );

        /* Obtain the mutex that is protecting access to the display. */
        xSemaphoreTake( xMutex, portMAX_DELAY );

        /* Record the time at which the mutex was taken. */
        xTimeAtWhichMutexWasTaken = xTaskGetTickCount();

        /* Write the generated text to the display–this is a slow operation. */
        vCopyTextToFrameBuffer( cTextBuffer );

        /* The text has been written to the display, so return the mutex. */
        xSemaphoreGive( xMutex );

        /* If taskYIELD() was called on each iteration then this task would
           only ever remain in the Running state for a short period of time, 
           and processing time would be wasted by rapidly switching between 
           tasks. Therefore, only call taskYIELD() if the tick count changed 
           while the mutex was held. */
        if( xTaskGetTickCount() != xTimeAtWhichMutexWasTaken )
        {
            taskYIELD();
        }
    }
}
```
***清单 8.16*** *确保在循环中使用互斥体的任务接收更相等的处理时间，同时还确保处理时间不会因任务之间切换太快而浪费*


## 8.4 网守任务

看门人任务提供了一种实现互斥的干净方法
没有优先级倒置或死锁的风险。

看门人任务是拥有资源唯一所有权的任务。仅
允许网守任务直接访问资源——任何其他任务
需要访问资源的任务只能通过使用间接访问
看门人的服务。


### 8.4.1 重写 vPrintString() 以使用网守任务

例 8.2 提供了另一种替代实现
`vPrintString()`。这次，使用网守任务来管理对
标准输出。当任务想要将消息写入标准输出时，它
不直接调用打印函数，而是发送消息
给看门人。

看门人任务使用 FreeRTOS 队列来序列化访问
标准输出。任务的内部执行不必
考虑互斥，因为这是唯一允许的任务
直接访问标准输出。

Gatekeeper任务大部分时间都处于Blocked状态，
等待消息到达队列。当消息到达时，
网守在返回之前只需将消息写入标准输出
进入阻塞状态等待下一条消息。实施
看门人任务如清单 8.18 所示。

中断可以发送到队列，因此中断服务程序也可以
安全地使用网守的服务将消息写入
终端。在这个例子中，一个勾号钩子函数用于写出一个
每 200 个刻度消息一次。

刻度 hook (or tick callback) 是一个由
内核在每个tick中断期间。要使用勾号钩子函数：

1. 将 FreeRTOSConfig.h 中的 `configUSE_TICK_HOOK` 设置为 1。

2.提供钩子函数的实现，使用准确
   函数名称和原型如清单 8.17 所示。


<a name="list8.17" title="Listing 8.17 The name and prototype for a tick hook function"></a>

```c
void vApplicationTickHook( void );
```
***清单 8.17*** *tick hook 函数的名称和原型*


滴答钩子函数在滴答中断的上下文中执行，
因此必须保持非常短，必须仅使用适量的堆栈
空格，并且不得调用任何不以 API 结尾的 FreeRTOS 函数
“FromISR()”。

调度程序总是在勾选钩子之后立即执行
函数，因此从tick调用中断安全FreeRTOS API函数
hook 不需要使用它们的 `pxHigherPriorityTaskWoken` 参数，并且
该参数可设置为 NULL。


<a name="list8.18" title="Listing 8.18 The gatekeeper task"></a>

```c
static void prvStdioGatekeeperTask( void *pvParameters )
{
    char *pcMessageToPrint;

    /* This is the only task that is allowed to write to standard out. Any
       other task wanting to write a string to the output does not access 
       standard out directly, but instead sends the string to this task. As 
       only this task accesses standard out there are no mutual exclusion or 
       serialization issues to consider within the implementation of the task 
       itself. */
    for( ;; )
    {
        /* Wait for a message to arrive. An indefinite block time is specified
           so there is no need to check the return value – the function will 
           only return when a message has been successfully received. */
        xQueueReceive( xPrintQueue, &pcMessageToPrint, portMAX_DELAY );

        /* Output the received string. */
        printf( "%s", pcMessageToPrint );
        fflush( stdout );

        /* Loop back to wait for the next message. */
    }
}
```
***清单 8.18*** *看门人任务*


<a name="example8.2" title="Example 8.2 The alternative implementation for print task"></a>
---
***示例 8.2*** *打印任务的替代实现*

---

写入队列的任务如清单 8.19 所示。和以前一样，
创建任务的两个单独实例，并创建任务字符串
使用任务参数将写入队列的数据传递到任务中。

<a name="list8.19" title="The print task implementation for Example 8.2"></a>


```c
static void prvPrintTask( void *pvParameters )
{
    int iIndexToString;
    const TickType_t xMaxBlockTimeTicks = 0x20;

    /* Two instances of this task are created. The task parameter is used to 
       pass an index into an array of strings into the task. Cast this to the
       required type. */
    iIndexToString = ( int ) pvParameters;

    for( ;; )
    {
        /* Print out the string, not directly, but instead by passing a pointer
           to the string to the gatekeeper task via a queue. The queue is 
           created before the scheduler is started so will already exist by the
           time this task executes for the first time. A block time is not 
           specified because there should always be space in the queue. */
        xQueueSendToBack( xPrintQueue, &( pcStringsToPrint[ iIndexToString ]), 0 );

        /* Wait a pseudo random time. Note that rand() is not necessarily
           reentrant, but in this case it does not really matter as the code 
           does not care what value is returned. In a more secure application 
           a version of rand() that is known to be reentrant should be used - 
           or calls to rand() should be protected using a critical section. */
        vTaskDelay( ( rand() % xMaxBlockTimeTicks ) );
    }
}
```

***清单 8.19*** *示例 8.2 的打印任务实现*


tick钩子函数统计调用次数，发送
每次计数达到 200 时，它向网守任务发送消息。
仅用于演示目的，勾选钩写入
队列，任务写入队列的末尾。蜱钩
实现如清单 8.20 所示。


<a name="list8.20" title="Listing 8.20 The tick hook implementation"></a>

```c
void vApplicationTickHook( void )
{
    static int iCount = 0;

    /* Print out a message every 200 ticks. The message is not written out
       directly, but sent to the gatekeeper task. */
    iCount++;

    if( iCount >= 200 )
    {
        /* As xQueueSendToFrontFromISR() is being called from the tick hook, it
           is not necessary to use the xHigherPriorityTaskWoken parameter (the 
           third parameter), and the parameter is set to NULL. */
        xQueueSendToFrontFromISR( xPrintQueue, 
                                  &( pcStringsToPrint[ 2 ] ), 
                                  NULL );

        /* Reset the count ready to print out the string again in 200 ticks
           time. */
        iCount = 0;
    }
}
```
***清单 8.20*** *tick hook 实现*


正常情况下，`main()` 创建运行所需的队列和任务
例如，然后启动调度程序。 `main()`的实现是
如清单 8.21 所示。

```c
/* Define the strings that the tasks and interrupt will print out via the
   gatekeeper. */
static char *pcStringsToPrint[] =
{
    "Task 1 ****************************************************\r\n",
    "Task 2 ----------------------------------------------------\r\n",
    "Message printed from the tick hook interrupt ##############\r\n"
};

/*-----------------------------------------------------------*/

/* Declare a variable of type QueueHandle_t. The queue is used to send messages
   from the print tasks and the tick interrupt to the gatekeeper task. */
QueueHandle_t xPrintQueue;

/*-----------------------------------------------------------*/

int main( void )
{
    /* Before a queue is used it must be explicitly created. The queue is 
       created to hold a maximum of 5 character pointers. */
    xPrintQueue = xQueueCreate( 5, sizeof( char * ) );

    /* Check the queue was created successfully. */
    if( xPrintQueue != NULL )
    {
        /* Create two instances of the tasks that send messages to the 
           gatekeeper. The index to the string the task uses is passed to the 
           task via the task parameter (the 4th parameter to xTaskCreate()). 
           The tasks are created at different priorities so the higher priority
           task will occasionally preempt the lower priority task. */
        xTaskCreate( prvPrintTask, "Print1", 1000, ( void * ) 0, 1, NULL );
        xTaskCreate( prvPrintTask, "Print2", 1000, ( void * ) 1, 2, NULL );

        /* Create the gatekeeper task. This is the only task that is permitted
           to directly access standard out. */
        xTaskCreate( prvStdioGatekeeperTask, "Gatekeeper", 1000, NULL, 0, NULL );

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    }

    /* If all is well then main() will never reach here as the scheduler will 
       now be running the tasks. If main() does reach here then it is likely 
       that there was insufficient heap memory available for the idle task to 
       be created. Chapter 3 provides more information on heap memory 
       management. */
    for( ;; );
}
```
<a name="list8.21" title="Listing 8.21 The implementation of main() for Example 8.2"></a>

***清单 8.21*** *示例 8.2 的 main() 的实现*


执行例 8.2 时产生的输出如图 8.8 所示。
可以看出，源自任务的字符串和来自任务的字符串
源自中断，所有打印都正确无误
腐败。


<a name="fig8.8" title="Figure 8.8 The output produced when Example 8.2 is executed"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image70.jpg)   
***图 8.8*** *执行例 8.2 时产生的输出*
* * *

为网守任务分配的优先级低于打印任务，因此
发送到网守的消息保留在队列中，直到两者都打印出来
任务处于阻塞状态。在某些情况下，这将是
适当地为网守分配更高的优先级，以便消息得到
立即处理，但这样做的代价是
网守延迟较低优先级的任务直到其完成
访问受保护的资源。
