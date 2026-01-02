/*
 * FreeRTOS Kernel V11.1.0
 * 版权所有 (C) 2021 Amazon.com, Inc. 或其关联公司。
 *
 * SPDX-License-Identifier: MIT
 *
 * 允许任何获得本软件及相关文档的人免费使用本软件，
 * 可不受限制地使用、复制、修改、合并、发布、分发、再许可和/或出售，
 * 并允许被提供软件的人同样处理本软件，但必须满足以下条件：
 *
 * 上述版权声明和本许可声明必须包含在本软件的所有副本或主要部分中。
 *
 * 本软件按“原样”提供，不附带任何明示或暗示的担保，
 * 包括但不限于适销性、特定用途适用性及非侵权的担保。
 * 无论是在合同、侵权或其他行为中，作者或版权持有人均不对
 * 因本软件引起或与本软件有关的任何索赔、损害或其他责任负责。
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 */

/*******************************************************************************
 * 本文件提供一个 FreeRTOSConfig.h 示例，并为配置项给出简要中文说明。
 * 更详细的资料请参阅线上文档：https://www.freertos.org/a00110.html
 *
 * 方括号（'[' 和 ']'）内的常量需要根据实际平台填写。
 *
 * 若移植包已提供专用的 FreeRTOSConfig.h，应优先使用对应文件。
 ******************************************************************************/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/******************************************************************************/
/* 硬件描述相关配置 ***********************************************************/
/******************************************************************************/

/* configCPU_CLOCK_HZ：通常应与产生系统节拍中断的时钟频率一致。 */
#define configCPU_CLOCK_HZ    ( ( unsigned long ) 84000000 )

/* configSYSTICK_CLOCK_HZ：Cortex-M 端口可选项，若 SysTick 时钟不同于核心时钟，需单独指定。 */

/*
 #define configSYSTICK_CLOCK_HZ                  [Platform specific]
 */

/******************************************************************************/
/* 调度行为相关配置 ***********************************************************/
/******************************************************************************/

/* configTICK_RATE_HZ：节拍中断频率（Hz），通常由内核时钟推算。 */
#define configTICK_RATE_HZ                         1000

/* configUSE_PREEMPTION：1 表示抢占式调度，0 表示协作式调度。 */
#define configUSE_PREEMPTION                       1

/* configUSE_TIME_SLICING：1 表示同优先级任务在每次节拍中断时轮转。 */
#define configUSE_TIME_SLICING                     1

/* configUSE_PORT_OPTIMISED_TASK_SELECTION：1 采用端口优化的任务选择算法。 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    1

/* configUSE_TICKLESS_IDLE：1 开启免节拍低功耗模式，0 表示始终保持节拍中断。 */
#define configUSE_TICKLESS_IDLE                    0

/* configMAX_PRIORITIES：可用任务优先级数量，范围 0~(configMAX_PRIORITIES-1)。 */
#define configMAX_PRIORITIES                       5

/* configMINIMAL_STACK_SIZE：空闲任务使用的栈大小（以字为单位）。 */
#define configMINIMAL_STACK_SIZE                   128

/* configMAX_TASK_NAME_LEN：任务名最大字符长度，包含结尾的空字符。 */
#define configMAX_TASK_NAME_LEN                    16

/* TickType_t 保存自内核启动以来的节拍计数，configTICK_TYPE_WIDTH_IN_BITS 用于选择 TickType_t 的位宽。 */
#define configTICK_TYPE_WIDTH_IN_BITS              TICK_TYPE_WIDTH_32_BITS

/* configIDLE_SHOULD_YIELD：1 表示若有同优先级任务就绪则空闲任务主动让出 CPU。 */
#define configIDLE_SHOULD_YIELD                    1

/* configTASK_NOTIFICATION_ARRAY_ENTRIES：每个任务的通知槽数量。 */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES      1

/* configQUEUE_REGISTRY_SIZE：队列注册表允许登记的队列与信号量数量。 */
#define configQUEUE_REGISTRY_SIZE                  0

/* configENABLE_BACKWARD_COMPATIBILITY：1 表示使能旧版 API 名称兼容层。 */
#define configENABLE_BACKWARD_COMPATIBILITY        0

/* configNUM_THREAD_LOCAL_STORAGE_POINTERS：每个任务可用的线程本地存储指针数量。 */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS    0

/* configUSE_MINI_LIST_ITEM：1 表示启用节省 RAM 的精简链表节点结构。 */
#define configUSE_MINI_LIST_ITEM                   1

/* configSTACK_DEPTH_TYPE：用于描述任务栈深度的类型。 */
#define configSTACK_DEPTH_TYPE                     size_t

/* configMESSAGE_BUFFER_LENGTH_TYPE：消息缓冲区中用于记录消息长度的类型。 */
#define configMESSAGE_BUFFER_LENGTH_TYPE           size_t

/* configHEAP_CLEAR_MEMORY_ON_FREE：1 表示释放堆内存时自动清零。 */
#define configHEAP_CLEAR_MEMORY_ON_FREE            1

/* configSTATS_BUFFER_MAX_LENGTH：vTaskList 等 API 期望的统计缓冲区长度上限。 */
#define configSTATS_BUFFER_MAX_LENGTH              0xFFFF

/* configUSE_NEWLIB_REENTRANT：1 表示为每个任务创建 newlib 重入结构体。 */
#define configUSE_NEWLIB_REENTRANT                 0

/******************************************************************************/
/* 软件定时器相关配置 *********************************************************/
/******************************************************************************/

/* configUSE_TIMERS：1 表示启用软件定时器功能。 */
#define configUSE_TIMERS                1

/* configTIMER_TASK_PRIORITY：定时器服务任务的优先级。 */
#define configTIMER_TASK_PRIORITY       ( configMAX_PRIORITIES - 1 )

/* configTIMER_TASK_STACK_DEPTH：定时器服务任务的栈深度（以字为单位）。 */
#define configTIMER_TASK_STACK_DEPTH    configMINIMAL_STACK_SIZE

/* configTIMER_QUEUE_LENGTH：定时器命令队列的长度。 */
#define configTIMER_QUEUE_LENGTH        10

/******************************************************************************/
/* 事件组相关配置 *************************************************************/
/******************************************************************************/

/* configUSE_EVENT_GROUPS：1 表示启用事件组组件。 */

#define configUSE_EVENT_GROUPS    1

/******************************************************************************/
/* 流缓冲区相关配置 ***********************************************************/
/******************************************************************************/

/* configUSE_STREAM_BUFFERS：1 表示启用流缓冲区组件。 */

#define configUSE_STREAM_BUFFERS    1

/******************************************************************************/
/* 内存分配相关配置 ***********************************************************/
/******************************************************************************/

/* configSUPPORT_STATIC_ALLOCATION：1 表示允许通过静态内存创建对象。 */
#define configSUPPORT_STATIC_ALLOCATION              1

/* configSUPPORT_DYNAMIC_ALLOCATION：1 表示允许通过动态内存创建对象。 */
#define configSUPPORT_DYNAMIC_ALLOCATION             1

/* configTOTAL_HEAP_SIZE：使用 heap_1/2/4 时的堆大小（字节）。 */
#define configTOTAL_HEAP_SIZE                        4096

/* configAPPLICATION_ALLOCATED_HEAP：1 表示堆数组由应用自行分配。 */
#define configAPPLICATION_ALLOCATED_HEAP             0

/* configSTACK_ALLOCATION_FROM_SEPARATE_HEAP：1 表示任务栈来自独立堆。 */
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP    0

/* configENABLE_HEAP_PROTECTOR：1 表示启用 heap_4/heap_5 的堆块保护功能。 */
#define configENABLE_HEAP_PROTECTOR                  0

/******************************************************************************/
/* Interrupt nesting behaviour configuration. *********************************/
/******************************************************************************/

/* STM32F401 具有 4 位 NVIC 优先级（0~15），数值越小优先级越高。 */
#define configPRIO_BITS                             4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY     15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* configKERNEL_INTERRUPT_PRIORITY：节拍与上下文切换中断的优先级设置。 */
#define configKERNEL_INTERRUPT_PRIORITY    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* configMAX_SYSCALL_INTERRUPT_PRIORITY：高于该优先级的中断不可调用 FreeRTOS API。 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* configMAX_API_CALL_INTERRUPT_PRIORITY：与 configMAX_SYSCALL_INTERRUPT_PRIORITY 含义一致。 */
#define configMAX_API_CALL_INTERRUPT_PRIORITY    configMAX_SYSCALL_INTERRUPT_PRIORITY

/******************************************************************************/
/* 钩子与回调相关配置 **********************************************************/
/******************************************************************************/

/* 以下 configUSE_* 宏用于控制对应钩子函数是否启用，启用后需自行实现回调。 */
#define configUSE_IDLE_HOOK                   0
#define configUSE_TICK_HOOK                   0
#define configUSE_MALLOC_FAILED_HOOK          0
#define configUSE_DAEMON_TASK_STARTUP_HOOK    0

/* configUSE_SB_COMPLETED_CALLBACK：1 表示为每个流缓冲区/消息缓冲区启用发送接收完成回调。 */
#define configUSE_SB_COMPLETED_CALLBACK       0

/* configCHECK_FOR_STACK_OVERFLOW：0 关闭，1 或 2 表示采用不同等级的栈溢出检查。 */
#define configCHECK_FOR_STACK_OVERFLOW        0

/******************************************************************************/
/* 运行时间与任务统计相关配置 **************************************************/
/******************************************************************************/

/* configGENERATE_RUN_TIME_STATS：1 表示采集每个任务的运行时间统计。 */
#define configGENERATE_RUN_TIME_STATS           0

/* configUSE_TRACE_FACILITY：1 表示在任务控制块中添加额外字段用于追踪。 */
#define configUSE_TRACE_FACILITY                0

/* configUSE_STATS_FORMATTING_FUNCTIONS：1 表示编译 vTaskList() 与 vTaskGetRunTimeStats()。 */
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/******************************************************************************/
/* 协程相关配置 ****************************************************************/
/******************************************************************************/

/* configUSE_CO_ROUTINES：1 表示启用协程功能。 */
#define configUSE_CO_ROUTINES              0

/* configMAX_CO_ROUTINE_PRIORITIES：协程可用的优先级数量。 */
#define configMAX_CO_ROUTINE_PRIORITIES    1

/******************************************************************************/
/* 调试辅助 ********************************************************************/
/******************************************************************************/

/* configASSERT：行为与标准 C assert() 类似，可自定义断言失败时的处理。 */
#define configASSERT( x )         \
    if( ( x ) == 0 )              \
    {                             \
        taskDISABLE_INTERRUPTS(); \
        for( ; ; )                \
        ;                         \
    }

/******************************************************************************/
/* FreeRTOS MPU 相关配置 ******************************************************/
/******************************************************************************/

/* configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS：1 表示允许应用自定义特权函数。 */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS    0

/* configTOTAL_MPU_REGIONS：目标硬件实现的 MPU 区域数量。 */
#define configTOTAL_MPU_REGIONS                                   8

/* configTEX_S_C_B_FLASH：覆盖用于 Flash 的 MPU TEX/S/C/B 位默认值。 */
#define configTEX_S_C_B_FLASH                                     0x07UL

/* configTEX_S_C_B_SRAM：覆盖用于 SRAM 的 MPU TEX/S/C/B 位默认值。 */
#define configTEX_S_C_B_SRAM                                      0x07UL

/* configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY：1 表示仅内核代码可触发特权提升。 */
#define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY               1

/* configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS：1 表示允许非特权任务进入临界区。 */
#define configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS                0

/* configUSE_MPU_WRAPPERS_V1：0 使用 v2 MPU 包装器，1 使用旧版 v1 包装器。 */
#define configUSE_MPU_WRAPPERS_V1                                 0

/* configPROTECTED_KERNEL_OBJECT_POOL_SIZE：v2 MPU 包装器跟踪的内核对象总数量上限。 */
#define configPROTECTED_KERNEL_OBJECT_POOL_SIZE                   10

/* configSYSTEM_CALL_STACK_SIZE：v2 MPU 包装器下系统调用栈的大小（按字计算）。 */
#define configSYSTEM_CALL_STACK_SIZE                              128

/* configENABLE_ACCESS_CONTROL_LIST：v2 MPU 包装器下启用访问控制列表功能。 */
#define configENABLE_ACCESS_CONTROL_LIST                          1

/******************************************************************************/
/* 对称多处理（SMP）相关配置 ***************************************************/
/******************************************************************************/

/* configNUMBER_OF_CORES：可用处理器内核数量。 */

/*
 #define configNUMBER_OF_CORES                     [Num of available cores]
 */

/* configRUN_MULTIPLE_PRIORITIES：0 表示不同优先级任务不会同时在多核运行。 */
#define configRUN_MULTIPLE_PRIORITIES             0

/* configUSE_CORE_AFFINITY：1 表示启用核心亲和性功能。 */
#define configUSE_CORE_AFFINITY                   0

/* configTASK_DEFAULT_CORE_AFFINITY：未指定亲和掩码的任务默认运行的核心集合。 */
#define configTASK_DEFAULT_CORE_AFFINITY          tskNO_AFFINITY

/* configUSE_TASK_PREEMPTION_DISABLE：1 表示支持单个任务关闭抢占。 */
#define configUSE_TASK_PREEMPTION_DISABLE         0

/* configUSE_PASSIVE_IDLE_HOOK：1 表示启用被动空闲钩子以添加后台逻辑。 */
#define configUSE_PASSIVE_IDLE_HOOK               0

/* configTIMER_SERVICE_TASK_CORE_AFFINITY：定时器/守护任务的核心亲和性设置。 */
#define configTIMER_SERVICE_TASK_CORE_AFFINITY    tskNO_AFFINITY


/******************************************************************************/
/* ARMv8-M 安全侧相关配置 *****************************************************/
/******************************************************************************/

/* secureconfigMAX_SECURE_CONTEXTS：可进入安全世界的任务数量上限。 */
#define secureconfigMAX_SECURE_CONTEXTS        5

/* configKERNEL_PROVIDED_STATIC_MEMORY：1 表示使用内核默认的空闲/定时器任务静态内存。 */
#define configKERNEL_PROVIDED_STATIC_MEMORY    1

/******************************************************************************/
/* ARMv8-M 端口特定配置 *******************************************************/
/******************************************************************************/

/* configENABLE_TRUSTZONE：1 表示在非安全侧启用 TrustZone 支持。 */
#define configENABLE_TRUSTZONE            0

/* configRUN_FREERTOS_SECURE_ONLY：1 表示内核整体运行在安全侧。 */
#define configRUN_FREERTOS_SECURE_ONLY    0

/* configENABLE_MPU：1 表示启用内存保护单元。 */
#define configENABLE_MPU                  0

/* configENABLE_FPU：1 表示启用浮点运算单元。 */
#define configENABLE_FPU                  0

/* configENABLE_MVE：1 表示启用 M-Profile 向量扩展，仅适用于支持该特性的内核。 */
#define configENABLE_MVE                  0

/******************************************************************************/
/* ARMv7-M / ARMv8-M 通用配置 **************************************************/
/******************************************************************************/

/* configCHECK_HANDLER_INSTALLATION：1 表示检查应用是否正确安装 FreeRTOS 中断处理函数。 */
#define configCHECK_HANDLER_INSTALLATION    1

/******************************************************************************/
/* 功能裁剪相关定义 ***********************************************************/
/******************************************************************************/

/* 下面的 configUSE_* 宏用于控制各项功能是否编译进入。 */
#define configUSE_TASK_NOTIFICATIONS           1
#define configUSE_MUTEXES                      1
#define configUSE_RECURSIVE_MUTEXES            1
#define configUSE_COUNTING_SEMAPHORES          1
#define configUSE_QUEUE_SETS                   0
#define configUSE_APPLICATION_TASK_TAG         0

/* 下面的 INCLUDE_* 宏用于控制对应 API 是否可用。 */
#define INCLUDE_vTaskPrioritySet               1
#define INCLUDE_uxTaskPriorityGet              1
#define INCLUDE_vTaskDelete                    1
#define INCLUDE_vTaskSuspend                   1
#define INCLUDE_xResumeFromISR                 1
#define INCLUDE_vTaskDelayUntil                1
#define INCLUDE_vTaskDelay                     1
#define INCLUDE_xTaskGetSchedulerState         1
#define INCLUDE_xTaskGetCurrentTaskHandle      1
#define INCLUDE_uxTaskGetStackHighWaterMark    0
#define INCLUDE_xTaskGetIdleTaskHandle         0
#define INCLUDE_eTaskGetState                  0
#define INCLUDE_xEventGroupSetBitFromISR       1
#define INCLUDE_xTimerPendFunctionCall         0
#define INCLUDE_xTaskAbortDelay                0
#define INCLUDE_xTaskGetHandle                 0
#define INCLUDE_xTaskResumeFromISR             1


#define vPortSVCHandler SVC_Handler          
#define xPortPendSVHandler PendSV_Handler       
#define xPortSysTickHandler SysTick_Handler      


#endif 
