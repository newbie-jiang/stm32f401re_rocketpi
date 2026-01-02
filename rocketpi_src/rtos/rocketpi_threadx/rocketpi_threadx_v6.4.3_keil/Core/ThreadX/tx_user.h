#ifndef TX_USER_H
#define TX_USER_H

/* 1ms ThreadX tick. */
#define TX_TIMER_TICKS_PER_SECOND     1000

/* Reduce memory footprint for this microcontroller. */
#define TX_MAX_PRIORITIES             32
#define TX_MINIMUM_STACK              256

/* Enable stack checking to help catch configuration issues early. */
#define TX_ENABLE_STACK_CHECKING

#endif /* TX_USER_H */
