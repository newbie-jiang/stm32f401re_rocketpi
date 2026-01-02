#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void app_init(void);
void app_poll(void);
void app_on_exti(uint16_t gpio_pin);

#ifdef __cplusplus
}
#endif

#endif
