#include "tx_api.h"

#include <stdint.h>

#include "stm32f4xx_hal.h"

extern VOID *_tx_initialize_unused_memory;
extern VOID *_tx_thread_system_stack_ptr;

#if defined(__ARMCC_VERSION) || defined(__CC_ARM)
extern ULONG Image$$RW_IRAM1$$ZI$$Limit;
extern ULONG __Vectors[];
#else
extern ULONG _estack;
extern ULONG __bss_end__;
extern VOID (*g_pfnVectors[])(void);
#endif

VOID _tx_initialize_low_level(VOID)
{
    __disable_irq();

#if defined(__ARMCC_VERSION) || defined(__CC_ARM)
    ULONG unused_memory = (ULONG)&Image$$RW_IRAM1$$ZI$$Limit;
#else
    ULONG unused_memory = (ULONG)&__bss_end__;
#endif

    /* Ensure the first unused address is word aligned. */
    unused_memory = (unused_memory + sizeof(ULONG) - 1U) & ~(sizeof(ULONG) - 1U);
    _tx_initialize_unused_memory = (VOID *)unused_memory;

#if defined(__ARMCC_VERSION) || defined(__CC_ARM)
    _tx_thread_system_stack_ptr = (VOID *)(uintptr_t)__Vectors[0];
    SCB->VTOR = (uint32_t)__Vectors;
#else
    _tx_thread_system_stack_ptr = (VOID *)&_estack;
    SCB->VTOR = (uint32_t)g_pfnVectors;
#endif
    __DSB();
    __ISB();

    NVIC_SetPriority(SVCall_IRQn, 0xFFU);
    NVIC_SetPriority(PendSV_IRQn, 0xFFU);
    NVIC_SetPriority(SysTick_IRQn, 0x40U);

    SysTick->LOAD = (SystemCoreClock / TX_TIMER_TICKS_PER_SECOND) - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk |
                    SysTick_CTRL_TICKINT_Msk;
}

VOID __tx_SVCallHandler(VOID)
{
    while (1)
    {
        __NOP();
    }
}
