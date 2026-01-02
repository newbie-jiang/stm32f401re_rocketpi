#include "unit_test.h"

#include <limits.h>
#include "portable.h"

#ifndef FREERTOS_RUN_TIME_STATS_HZ
#define FREERTOS_RUN_TIME_STATS_HZ           100000UL
#endif

#define ADMIN_TAG                    "unit_test_admin"
#define ADMIN_STATS_INTERVAL_MS      2000U
#define ADMIN_STATS_BUFFER_LEN       1024U

extern uint32_t SystemCoreClock;

typedef enum
{
    RUN_TIME_SRC_NONE = 0,
    RUN_TIME_SRC_DWT,
    RUN_TIME_SRC_SYSTICK
} run_time_source_t;

typedef struct
{
    TaskHandle_t handle;
    uint32_t runtime;
} task_runtime_snapshot_t;

static uint32_t s_runtimeStatsDivider = 0U;
static volatile run_time_source_t s_runtimeSource = RUN_TIME_SRC_NONE;
static bool s_runtimeSourceLogged = false;
static bool s_runtimeTimerConfigured = false;
static uint32_t s_lastDwtSample = 0U;
static uint64_t s_cycleAccumulator = 0U;
static uint32_t s_runTimeCounter = 0U;
static task_runtime_snapshot_t *s_prevTaskStats = NULL;
static UBaseType_t s_prevTaskStatsCount = 0U;
static char s_statsBuffer[ADMIN_STATS_BUFFER_LEN];

static uint32_t prvDiffWithOverflow(uint32_t current, uint32_t previous);
static void prvLogRunTimeSource(void);
static uint32_t prvFindPreviousRuntime(TaskHandle_t handle);
static void prvStoreTaskSnapshot(const TaskStatus_t *statusArray, UBaseType_t count);

void vConfigureTimerForRunTimeStats(void)
{
    if (s_runtimeTimerConfigured)
    {
        return;
    }

    s_runtimeStatsDivider = SystemCoreClock / FREERTOS_RUN_TIME_STATS_HZ;
    if (s_runtimeStatsDivider == 0U)
    {
        s_runtimeStatsDivider = 1U;
    }

    s_runtimeSource = RUN_TIME_SRC_NONE;
    s_cycleAccumulator = 0U;
    s_runTimeCounter = 0U;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

#if defined(DWT_LAR)
    DWT->LAR = 0xC5ACCE55;
#endif

    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    bool cyccntSupported = true;
#ifdef DWT_CTRL_NOCYCCNT_Msk
    cyccntSupported = ((DWT->CTRL & DWT_CTRL_NOCYCCNT_Msk) == 0U);
#endif

    if (cyccntSupported)
    {
        s_lastDwtSample = DWT->CYCCNT;
        uint32_t start = s_lastDwtSample;
        for (volatile uint32_t i = 0U; i < 128U; ++i)
        {
            __NOP();
        }
        if (DWT->CYCCNT != start)
        {
            s_runtimeSource = RUN_TIME_SRC_DWT;
            s_runtimeTimerConfigured = true;
            return;
        }
    }

    s_runtimeSource = RUN_TIME_SRC_SYSTICK;
    s_runtimeTimerConfigured = true;
}

uint32_t ulGetRunTimeCounterValue(void)
{
    if ((s_runtimeStatsDivider == 0U) || (s_runtimeSource == RUN_TIME_SRC_NONE))
    {
        vConfigureTimerForRunTimeStats();
    }

    if (s_runtimeSource == RUN_TIME_SRC_DWT)
    {
        uint32_t current = DWT->CYCCNT;
        uint32_t delta = current - s_lastDwtSample;
        s_lastDwtSample = current;

        s_cycleAccumulator += delta;
        if (s_cycleAccumulator >= s_runtimeStatsDivider)
        {
            uint32_t increments = (uint32_t)(s_cycleAccumulator / s_runtimeStatsDivider);
            s_cycleAccumulator -= ((uint64_t)increments * s_runtimeStatsDivider);
            s_runTimeCounter += increments;
        }

        return s_runTimeCounter;
    }

    return HAL_GetTick();
}

void admin_Task(void *argument)
{
    (void)argument;

    const TickType_t delayTicks = pdMS_TO_TICKS(ADMIN_STATS_INTERVAL_MS);
    TaskHandle_t idleHandle = xTaskGetIdleTaskHandle();
    uint32_t prevTotalRuntime = 0U;
    uint32_t prevIdleRuntime = 0U;

    for (;;)
    {
        vTaskDelay(delayTicks);

        UBaseType_t taskCount = uxTaskGetNumberOfTasks();
        if (taskCount == 0U)
        {
            continue;
        }

        TaskStatus_t *taskStatus = pvPortMalloc(taskCount * sizeof(TaskStatus_t));
        if (taskStatus == NULL)
        {
            elog_w(ADMIN_TAG, "No heap for %lu task stats entries", (unsigned long)taskCount);
            continue;
        }

        uint32_t totalRunTime = 0U;
        UBaseType_t populated = uxTaskGetSystemState(taskStatus, taskCount, &totalRunTime);

        uint32_t idleRunTime = 0U;
        for (UBaseType_t i = 0; i < populated; ++i)
        {
            if (taskStatus[i].xHandle == idleHandle)
            {
                idleRunTime = taskStatus[i].ulRunTimeCounter;
                break;
            }
        }

        uint32_t totalDelta = prvDiffWithOverflow(totalRunTime, prevTotalRuntime);
        uint32_t idleDelta = prvDiffWithOverflow(idleRunTime, prevIdleRuntime);

        prevTotalRuntime = totalRunTime;
        prevIdleRuntime = idleRunTime;

        if (totalDelta == 0U)
        {
            prvStoreTaskSnapshot(taskStatus, populated);
            vPortFree(taskStatus);
            continue;
        }

        if (!s_runtimeSourceLogged)
        {
            prvLogRunTimeSource();
        }

        float idlePercent = ((float)idleDelta * 100.0f) / (float)totalDelta;
        if (idlePercent > 100.0f)
        {
            idlePercent = 100.0f;
        }
        float cpuPercent = 100.0f - idlePercent;

        size_t freeHeap = xPortGetFreeHeapSize();
        size_t minEverHeap = xPortGetMinimumEverFreeHeapSize();

        elog_i(ADMIN_TAG,
               "CPU %.2f%%  Idle %.2f%% (window=%lu ticks)  Heap free=%lu bytes  MinEver=%lu bytes",
               cpuPercent,
               idlePercent,
               (unsigned long)totalDelta,
               (unsigned long)freeHeap,
               (unsigned long)minEverHeap);
        elog_i(ADMIN_TAG, "Task           DeltaTicks   Window%%   StackFree(bytes)");

        for (UBaseType_t i = 0; i < populated; ++i)
        {
            uint32_t prevTaskRuntime = prvFindPreviousRuntime(taskStatus[i].xHandle);
            uint32_t taskDelta = prvDiffWithOverflow(taskStatus[i].ulRunTimeCounter, prevTaskRuntime);

            float taskPercent = ((float)taskDelta * 100.0f) / (float)totalDelta;
            StackType_t stackHighWater = uxTaskGetStackHighWaterMark(taskStatus[i].xHandle);
            unsigned long stackFreeBytes = (unsigned long)stackHighWater * (unsigned long)sizeof(StackType_t);

            if ((taskPercent >= 1.0f) || (taskDelta == 0U))
            {
                (void)snprintf(s_statsBuffer,
                               sizeof(s_statsBuffer),
                               "%-14s %10lu   %8.2f%%   %8lu",
                               taskStatus[i].pcTaskName,
                               (unsigned long)taskDelta,
                               taskPercent,
                               stackFreeBytes);
            }
            else
            {
                (void)snprintf(s_statsBuffer,
                               sizeof(s_statsBuffer),
                               "%-14s %10lu        <1%%   %8lu",
                               taskStatus[i].pcTaskName,
                               (unsigned long)taskDelta,
                               stackFreeBytes);
            }

            elog_i(ADMIN_TAG, "%s", s_statsBuffer);
        }

        prvStoreTaskSnapshot(taskStatus, populated);
        vPortFree(taskStatus);
    }
}

static uint32_t prvDiffWithOverflow(uint32_t current, uint32_t previous)
{
    return (current >= previous)
               ? (current - previous)
               : ((UINT32_MAX - previous) + 1U + current);
}

static void prvLogRunTimeSource(void)
{
    if (s_runtimeSourceLogged)
    {
        return;
    }

    if (s_runtimeSource == RUN_TIME_SRC_DWT)
    {
        elog_i(ADMIN_TAG, "Run-time stats source: DWT CYCCNT (%lu Hz)", (unsigned long)FREERTOS_RUN_TIME_STATS_HZ);
    }
    else if (s_runtimeSource == RUN_TIME_SRC_SYSTICK)
    {
        elog_w(ADMIN_TAG, "Run-time stats fallback to HAL_GetTick() (1 kHz). Check DWT trace enable if higher resolution is required.");
    }
    else
    {
        elog_w(ADMIN_TAG, "Run-time stats timer not configured.");
    }

    s_runtimeSourceLogged = true;
}

static uint32_t prvFindPreviousRuntime(TaskHandle_t handle)
{
    for (UBaseType_t i = 0; i < s_prevTaskStatsCount; ++i)
    {
        if (s_prevTaskStats[i].handle == handle)
        {
            return s_prevTaskStats[i].runtime;
        }
    }

    return 0U;
}

static void prvStoreTaskSnapshot(const TaskStatus_t *statusArray, UBaseType_t count)
{
    if (count == 0U)
    {
        if (s_prevTaskStats != NULL)
        {
            vPortFree(s_prevTaskStats);
            s_prevTaskStats = NULL;
            s_prevTaskStatsCount = 0U;
        }
        return;
    }

    task_runtime_snapshot_t *newSnapshot = pvPortMalloc(count * sizeof(task_runtime_snapshot_t));
    if (newSnapshot == NULL)
    {
        elog_w(ADMIN_TAG, "No heap to cache task run-time snapshot");
        return;
    }

    for (UBaseType_t i = 0; i < count; ++i)
    {
        newSnapshot[i].handle = statusArray[i].xHandle;
        newSnapshot[i].runtime = statusArray[i].ulRunTimeCounter;
    }

    if (s_prevTaskStats != NULL)
    {
        vPortFree(s_prevTaskStats);
    }

    s_prevTaskStats = newSnapshot;
    s_prevTaskStatsCount = count;
}
