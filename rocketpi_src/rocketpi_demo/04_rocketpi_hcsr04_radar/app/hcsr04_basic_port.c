#include "driver_hcsr04_basic.h"

#include "driver_hcsr04_interface.h"

#include <stdint.h>

#if defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__clang__) || defined(__GNUC__)
#define HCSR04_WEAK __attribute__((weak))
#else
#define HCSR04_WEAK
#endif

static hcsr04_handle_t s_hcsr04_handle;

HCSR04_WEAK uint8_t hcsr04_basic_init(void)
{
    uint8_t res;

    DRIVER_HCSR04_LINK_INIT(&s_hcsr04_handle, hcsr04_handle_t);
    DRIVER_HCSR04_LINK_TRIG_INIT(&s_hcsr04_handle, hcsr04_interface_trig_init);
    DRIVER_HCSR04_LINK_TRIG_DEINIT(&s_hcsr04_handle, hcsr04_interface_trig_deinit);
    DRIVER_HCSR04_LINK_TRIG_WRITE(&s_hcsr04_handle, hcsr04_interface_trig_write);
    DRIVER_HCSR04_LINK_ECHO_INIT(&s_hcsr04_handle, hcsr04_interface_echo_init);
    DRIVER_HCSR04_LINK_ECHO_DEINIT(&s_hcsr04_handle, hcsr04_interface_echo_deinit);
    DRIVER_HCSR04_LINK_ECHO_WRITE(&s_hcsr04_handle, hcsr04_interface_echo_read);
    DRIVER_HCSR04_LINK_TIMESTAMP_READ(&s_hcsr04_handle, hcsr04_interface_timestamp_read);
    DRIVER_HCSR04_LINK_DELAY_MS(&s_hcsr04_handle, hcsr04_interface_delay_ms);
    DRIVER_HCSR04_LINK_DELAY_US(&s_hcsr04_handle, hcsr04_interface_delay_us);
    DRIVER_HCSR04_LINK_DEBUG_PRINT(&s_hcsr04_handle, hcsr04_interface_debug_print);

    res = hcsr04_init(&s_hcsr04_handle);
    if (res != 0U)
    {
        hcsr04_interface_debug_print("hcsr04: init failed.\n");
        return 1U;
    }

    return 0U;
}

HCSR04_WEAK uint8_t hcsr04_basic_read(float *m)
{
    uint32_t time_us;

    if (m == NULL)
    {
        return 1U;
    }

    if (hcsr04_read(&s_hcsr04_handle, &time_us, m) != 0U)
    {
        return 1U;
    }

    return 0U;
}

HCSR04_WEAK uint8_t hcsr04_basic_deinit(void)
{
    if (hcsr04_deinit(&s_hcsr04_handle) != 0U)
    {
        return 1U;
    }

    return 0U;
}
