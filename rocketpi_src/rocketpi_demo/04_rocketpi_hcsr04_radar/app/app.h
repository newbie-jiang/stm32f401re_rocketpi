#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise the radar demo application.
 */
void App_Init(void);

/**
 * @brief Perform one radar update step.
 *
 * Call this from the main loop after App_Init() to keep the
 * servo, sensor, and display synchronised.
 */
void App_Update(void);

/**
 * @brief Convenience helper that calls App_Init() and then
 *        repeatedly executes App_Update() forever.
 */
void App_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
