# 11 低功耗支持

## 11.1 省电介绍

FreeRTOS 提供了一种通过 IDLE 任务进入低功耗模式的简单方法 
挂钩和无滴答空闲模式。

通常通过使用以下方法来降低运行 FreeRTOS 的微控制器的功耗 
IDLE 任务挂钩将微控制器置于低功耗状态。可以节省的电量 
这种方法所实现的目标受到周期性退出然后重新进入低功耗状态的必要性的限制。 
状态来处理滴答中断。此外，如果节拍中断的频率太高（从 
空闲太频繁），每次进入然后退出低功耗状态所消耗的能量和时间 
对于除了最轻的省电模式之外的所有模式，勾选将超过任何潜在的省电收益。

FreeRTOS 支持低功耗状态，允许微控制器定期进入和退出低功耗状态 
功耗。 FreeRTOS 无滴答空闲模式会在空闲 periods 
(when there are no application tasks that are able to execute) 期间停止周期性滴答中断，从而允许 MCU 保持在深度状态 
省电状态，直到发生中断，或者 RTOS 内核转换一个 
任务进入就绪状态。然后，当 
滴答中断重新启动。 FreeRTOS的tickless模式原理是让MCU进入 
当 MCU 执行空闲任务时，低功耗模式可节省系统功耗。


## 11.2 FreeRTOS 睡眠模式

FreeRTOS 支持三种类型的睡眠模式：

1. eAbortSleep - 此模式表示任务已准备就绪、上下文切换已挂起或打勾 
   中断已经发生，但由于调度程序被挂起而被挂起。它发出 RTOS 信号 
   中止进入睡眠模式。

2. eStandardSleep - 此模式允许进入睡眠模式，持续时间不会超过预期 
   空闲时间。

3. eNoTasksWaitingTimeout - 当没有任务等待超时时进入此模式，因此可以安全地 
   进入睡眠模式，只能通过外部中断或复位退出。


## 11.3 函数和启用内置 Tickless 空闲功能

通过在 FreeRTOSConfig.h 
(for ports that support this feature) 中将 `configUSE_TICKLESS_IDLE` 定义为 1 来启用内置 Tickless Idle 功能。用户定义的无滴答空闲功能可以为任何 
FreeRTOS port (including those that include a built in implementation) 通过定义 `configUSE_TICKLESS_IDLE` 
FreeRTOSConfig.h 中的 2。

当启用无滴答空闲功能时，内核将调用 `portSUPPRESS_TICKS_AND_SLEEP()` 
当满足以下两个条件时宏：

1. 空闲任务是唯一能够运行的任务，因为所有应用程序任务要么处于阻塞状态，要么处于阻塞状态。 
   状态或挂起状态。

2. 在内核转换应用程序之前，至少还会经过 n 个完整的节拍周期 
   任务退出阻塞状态，其中 n 由 `configEXPECTED_IDLE_TIME_BEFORE_SLEEP` 定义设置 
   FreeRTOSConfig.h。


### 11.3.1 portSUPPRESS\_TICKS\_AND\_SLEEP() 宏

<a name="list11.1" title="Listing 11.1 The prototype for the portSUPPRESS\_TICKS\_AND\_SLEEP macro"></a>

```c
portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime )
```
***清单 11.1*** *portSUPPRESS\_TICKS\_AND\_SLEEP 宏的原型*

`portSUPPRESS_TICKS_AND_SLEEP()`中`xExpectedIdleTime`参数的值等于总数 
任务进入就绪状态之前的时钟周期数。因此参数值是 
微控制器可以安全地保持在深度睡眠状态的时间，并抑制滴答中断。


### 11.3.2 vPortSuppressTicksAndSleep 函数

FreeRTOS 中定义了 `vPortSuppressTicksAndSleep()` 函数，可用于实现 
无滴答模式。该函数在 FreeRTOS Cortex-M 端口层中弱定义，可以被覆盖 
由应用程序编写者。

<a name="list11.2" title="Listing 11.2 The vPortSuppressTicksAndSleep API function prototype"></a>

```c
void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime );
```
***清单 11.2*** *vPortSuppressTicksAndSleep API 函数原型*


### 11.3.3 eTaskConfirmSleepModeStatus 函数

API *eTaskConfirmSleepModeStatus* 返回睡眠模式状态以确定是否可以继续 
与睡眠有关，以及是否可以无限期地睡眠。此功能仅在 `configUSE_TICKLESS_IDLE` 时可用 
设置为 1。

<a name="list11.3" title="Listing 11.3 The eTaskConfirmSleepModeStatus API function prototype"></a>

```c
eSleepModeStatus eTaskConfirmSleepModeStatus( void );
```
***清单 11.3*** *eTaskConfirmSleepModeStatus API 函数原型*


如果 `eTaskConfirmSleepModeStatus()` 在调用时返回 `eNoTasksWaitingTimeout` 
在`portSUPPRESS_TICKS_AND_SLEEP()`内，那么微控制器可以保持在深度睡眠状态 
无限期地。 `eTaskConfirmSleepModeStatus()` 仅在以下情况下返回 `eNoTasksWaitingTimeout`： 
以下条件为真：

+ 软件定时器没有被使用，所以调度程序不会执行定时器回调函数 
  未来的任何时候。

+ 所有应用程序任务要么处于挂起状态，要么处于具有超时值的阻塞状态 
  `portMAX_DELAY`，因此调度程序不会在任何固定时间将任务从阻塞状态转换出来 
  未来的时间。

为了避免竞争条件，FreeRTOS 调度程序在 `portSUPPRESS_TICKS_AND_SLEEP()` 之前被暂停 
调用，并在 `portSUPPRESS_TICKS_AND_SLEEP()` 完成时恢复。这确保应用程序任务不能 
在退出低功耗状态的微控制器和 `portSUPPRESS_TICKS_AND_SLEEP()` 之间执行 
完成其执行。此外，`portSUPPRESS_TICKS_AND_SLEEP()` 函数需要 
在计时器停止和进入睡眠模式之间创建一个小的关键部分，以确保 
进入睡眠模式就可以了。 `eTaskConfirmSleepModeStatus()` 应该从此关键调用 
部分。

此外，FreeRTOS还为用户提供了FreeRTOSConfig.h中定义的另外两个接口函数。这些 
宏允许应用程序编写者在将 MCU 放入低电平之前和之后添加额外的步骤 
分别为电源状态。


### 11.3.4 configPRE\_SLEEP\_PROCESSING 配置

<a name="list11.4" title="Listing 11.4 The prototype for the configPRE\_SLEEP\_PROCESSING macro"></a>

```c
configPRE_SLEEP_PROCESSING( xExpectedIdleTime )
```
***清单 11.4*** *configPRE\_SLEEP\_PROCESSING 宏的原型*


在用户使 MCU 进入低功耗模式之前，必须调用 `configPRE_SLEEP_PROCESSING()` 
配置系统参数以降低系统功耗，例如关闭其他外设 
时钟，降低系统频率。


### 11.3.5 configPOST\_SLEEP\_PROCESSING 配置

<a name="list11.5" title="Listing 11.5 The prototype for the configPOST\_SLEEP\_PROCESSING macro"></a>

```c
configPOST_SLEEP_PROCESSING( xExpectedIdleTime )
```
***清单 11.5*** *configPOST\_SLEEP\_PROCESSING 宏的原型*


退出低功耗模式后，用户应调用`configPOST_SLEEP_PROCESSING()`函数 
恢复系统主频及外围功能。


## 11.4 实现 portSUPPRESS\_TICKS\_AND\_SLEEP() 宏

如果使用的 FreeRTOS 端口不提供 `portSUPPRESS_TICKS_AND_SLEEP()` 的默认实现， 
那么应用程序编写者可以通过定义 `portSUPPRESS_TICKS_AND_SLEEP()` 来提供自己的实现 
FreeRTOSConfig.h。如果使用的 FreeRTOS 端口确实提供了 `portSUPPRESS_TICKS_AND_SLEEP()` 的默认实现， 
那么应用程序编写者可以通过定义 `portSUPPRESS_TICKS_AND_SLEEP()` 来覆盖默认实现 
FreeRTOSConfig.h。

以下源代码是 `portSUPPRESS_TICKS_AND_SLEEP()` 如何由 
应用程序作家。该示例是基本的，并且将在由 
内核和日历时间。在示例中显示的函数调用中，只有 `vTaskStepTick()` 
和 `eTaskConfirmSleepModeStatus()` 是 FreeRTOS API 的一部分。其他功能特定于 
正在使用的硬件上可用的时钟和省电模式，因此必须由 
应用程序作家。

<a name="list11.6" title="Listing 11.6 An example of a user defined implementation of portSUPPRESS\_TICKS\_AND\_SLEEP()"></a>

```c
/* First define the portSUPPRESS_TICKS_AND_SLEEP() macro.  The parameter is the
   time, in ticks, until the kernel next needs to execute. */

#define portSUPPRESS_TICKS_AND_SLEEP( xIdleTime ) vApplicationSleep( xIdleTime )

/* Define the function that is called by portSUPPRESS_TICKS_AND_SLEEP(). */
void vApplicationSleep( TickType_t xExpectedIdleTime )
{
    unsigned long ulLowPowerTimeBeforeSleep, ulLowPowerTimeAfterSleep;

    eSleepModeStatus eSleepStatus;

    /* Read the current time from a time source that will remain operational
       while the microcontroller is in a low power state. */
    ulLowPowerTimeBeforeSleep = ulGetExternalTime();

    /* Stop the timer that is generating the tick interrupt. */
    prvStopTickInterruptTimer();

    /* Enter a critical section that will not effect interrupts bringing the MCU
       out of sleep mode. */
    disable_interrupts();

    /* Ensure it is still ok to enter the sleep mode. */
    eSleepStatus = eTaskConfirmSleepModeStatus();

    if( eSleepStatus == eAbortSleep )
    {
        /* A task has been moved out of the Blocked state since this macro was
           executed, or a context siwth is being held pending.  Do not enter a
           sleep state.  Restart the tick and exit the critical section. */
        prvStartTickInterruptTimer();
        enable_interrupts();
    }
    else
    {
        if( eSleepStatus == eNoTasksWaitingTimeout )
        {
            /* It is not necessary to configure an interrupt to bring the
               microcontroller out of its low power state at a fixed time in 
               the future. */
            prvSleep();
        }
        else
        {
            /* Configure an interrupt to bring the microcontroller out of its low
               power state at the time the kernel next needs to execute.  The
               interrupt must be generated from a source that remains operational
               when the microcontroller is in a low power state. */
            vSetWakeTimeInterrupt( xExpectedIdleTime );

            /* Enter the low power state. */
            prvSleep();

            /* Determine how long the microcontroller was actually in a low power
               state for, which will be less than xExpectedIdleTime if the
               microcontroller was brought out of low power mode by an interrupt
               other than that configured by the vSetWakeTimeInterrupt() call.
               Note that the scheduler is suspended before
               portSUPPRESS_TICKS_AND_SLEEP() is called, and resumed when
               portSUPPRESS_TICKS_AND_SLEEP() returns.  Therefore no other tasks will
               execute until this function completes. */
            ulLowPowerTimeAfterSleep = ulGetExternalTime();

            /* Correct the kernels tick count to account for the time the
               microcontroller spent in its low power state. */
            vTaskStepTick( ulLowPowerTimeAfterSleep - ulLowPowerTimeBeforeSleep );
        }

        /* Exit the critical section - it might be possible to do this immediately
           after the prvSleep() calls. */
        enable_interrupts();

        /* Restart the timer that is generating the tick interrupt. */
        prvStartTickInterruptTimer();
    }
}
```
***清单 11.6*** *用户定义的 portSUPPRESS\_TICKS\_AND\_SLEEP() 实现的示例*


## 11.5 空闲任务钩子函数

空闲任务可以选择调用应用程序定义的 hook (or callback) 函数 - 空闲钩子。 
空闲任务以最低优先级运行，因此这样的空闲钩子函数只有在以下情况下才会被执行： 
没有可以运行的更高优先级的任务。这使得 Idle hook 功能成为理想的选择 
将处理器置于低功耗状态的位置 - 随时提供自动省电功能 
是没有要执行的处理。仅当设置 `configUSE_IDLE_HOOK` 时才会调用 Idle hook 
FreeRTOSConfig.h 内为 1。

<a name="list11.7" title="Listing 11.7 The vApplicationIdleHook API function prototype"></a>

```c
void vApplicationIdleHook( void );
```
***清单 11.7*** *vApplicationIdleHook API 函数原型*


只要空闲任务正在运行，空闲钩子就会被重复调用。最重要的是闲置 
挂钩函数不会调用任何可能导致其阻塞的 API 函数。另外，如果应用 
使用 `vTaskDelete()` API 函数，则必须允许空闲任务挂钩定期 
返回，因为空闲任务负责清理 RTOS 分配的资源 
kernel 到已删除的任务。
