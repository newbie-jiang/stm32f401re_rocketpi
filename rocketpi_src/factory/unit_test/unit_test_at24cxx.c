#include "unit_test.h"
#include "driver_at24cxx_read_test.h"

static volatile unit_test_eeprom_status_t g_eeprom_status = UNIT_TEST_EEPROM_STATUS_NOT_TESTED;

void at24cxx_Task(void *argument)
{
	  osDelay(1200);
    g_eeprom_status = UNIT_TEST_EEPROM_STATUS_TESTING;

    if (at24cxx_read_test(AT24C02, AT24CXX_ADDRESS_A000) == 0U)
    {
        g_eeprom_status = UNIT_TEST_EEPROM_STATUS_PASS;
    }
    else
    {
        g_eeprom_status = UNIT_TEST_EEPROM_STATUS_FAIL;
    }
	
  for(;;)
  {
    osDelay(500);
		osThreadExit(); // 等价于 vTaskDelete(NULL)
  }
}

unit_test_eeprom_status_t unit_test_eeprom_get_status(void)
{
    return g_eeprom_status;
}
