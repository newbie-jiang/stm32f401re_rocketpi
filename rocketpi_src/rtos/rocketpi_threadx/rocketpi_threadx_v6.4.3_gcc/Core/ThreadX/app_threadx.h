#ifndef APP_THREADX_H
#define APP_THREADX_H

#include "tx_api.h"

/* Entry function that Azure RTOS calls during kernel start-up. */
VOID tx_application_define(VOID *first_unused_memory);

#endif /* APP_THREADX_H */
