/**
 * @file retarget.c
 * @brief Bare-metal stubs required when semihosting is disabled.
 */

#include <stdint.h>

void _ttywrch(int ch)
{
    (void)ch;
}
