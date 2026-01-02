#include "app_threadx.h"
#include "gpio.h"

/* Simple LED thread demo configuration. */
#define DEMO_THREAD_STACK_SIZE   512U
#define DEMO_THREAD_PRIORITY     5U

/* Declare the thread objects */
static TX_THREAD led1_thread;
static TX_THREAD led2_thread;
static TX_THREAD led3_thread;

static ULONG led1_thread_stack[DEMO_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG led2_thread_stack[DEMO_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG led3_thread_stack[DEMO_THREAD_STACK_SIZE / sizeof(ULONG)];

static VOID led1_thread_entry(ULONG context);
static VOID led2_thread_entry(ULONG context);
static VOID led3_thread_entry(ULONG context);

VOID tx_application_define(VOID *first_unused_memory)
{
    TX_THREAD_NOT_USED(first_unused_memory);

    UINT status;

    /* Create LED1 thread */
    status = tx_thread_create(&led1_thread,
                              "LED1 Thread",
                              led1_thread_entry,
                              0U,
                              led1_thread_stack,
                              sizeof(led1_thread_stack),
                              DEMO_THREAD_PRIORITY,
                              DEMO_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE,
                              TX_AUTO_START);

    if (status != TX_SUCCESS) { for( ; ; ); }

    /* Create LED2 thread */
    status = tx_thread_create(&led2_thread,
                              "LED2 Thread",
                              led2_thread_entry,
                              0U,
                              led2_thread_stack,
                              sizeof(led2_thread_stack),
                              DEMO_THREAD_PRIORITY,
                              DEMO_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE,
                              TX_AUTO_START);

    if (status != TX_SUCCESS) { for( ; ; ); }

    /* Create LED3 thread */
    status = tx_thread_create(&led3_thread,
                              "LED3 Thread",
                              led3_thread_entry,
                              0U,
                              led3_thread_stack,
                              sizeof(led3_thread_stack),
                              DEMO_THREAD_PRIORITY,
                              DEMO_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE,
                              TX_AUTO_START);

    if (status != TX_SUCCESS) { for( ; ; ); }
}

static VOID led1_thread_entry(ULONG context)
{
    TX_THREAD_NOT_USED(context);

    for (;;)
    {
        HAL_GPIO_TogglePin(LED_P_GPIO_Port, LED_P_Pin);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2U);
    }
}

static VOID led2_thread_entry(ULONG context)
{
    TX_THREAD_NOT_USED(context);

    for (;;)
    {
        HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2U);
    }
}

static VOID led3_thread_entry(ULONG context)
{
    TX_THREAD_NOT_USED(context);

    for (;;)
    {
        HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2U);
    }
}
