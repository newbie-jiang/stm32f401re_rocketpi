#ifndef ROCKETPI_APP_H
#define ROCKETPI_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the RocketPi smart-home demo logic.
 */
void App_Init(void);

/**
 * @brief Periodic application task. Call inside the main loop.
 */
void App_Loop(void);

#ifdef __cplusplus
}
#endif

#endif /* ROCKETPI_APP_H */
