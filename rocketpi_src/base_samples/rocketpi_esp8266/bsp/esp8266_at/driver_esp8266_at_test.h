/**
 * @file driver_esp8266_at_test.h
 * @brief Smoke tests for the ESP8266 AT driver.
 */
#ifndef DRIVER_ESP8266_AT_TEST_H
#define DRIVER_ESP8266_AT_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

void esp8266_at_test_run(void);
void esp8266_at_test_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_ESP8266_AT_TEST_H */
