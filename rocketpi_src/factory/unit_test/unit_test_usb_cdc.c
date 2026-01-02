#include "unit_test.h"

#include "usbd_cdc_if.h"
#include "usbd_def.h"


static uint8_t cdcRxAppBuffer[APP_RX_DATA_SIZE];

extern uint8_t CDC_IsRxReady(void);
extern uint32_t CDC_ReadRxData(uint8_t *Buf, uint32_t BufSize);
extern USBD_HandleTypeDef hUsbDeviceFS;

void usb_cdc_Task(void *argument)
{
  (void)argument;
	 osDelay(2000); /* 等待USB SHELL 稳定 */
	
  for(;;)
  {
    if (CDC_IsRxReady())
    {
      uint32_t received = CDC_ReadRxData(cdcRxAppBuffer, sizeof(cdcRxAppBuffer));
      if (received > 0U)
      {
        shell_push_input_stream(cdcRxAppBuffer, received);
      }
    }
    osDelay(1);
  }
}

bool unit_test_usb_cdc_is_connected(void)
{
    return (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) &&
           (hUsbDeviceFS.pClassData != NULL);
}
