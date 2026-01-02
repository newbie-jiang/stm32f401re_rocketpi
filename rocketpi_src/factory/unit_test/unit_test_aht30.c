#include "unit_test.h"
#include "driver_aht30.h"

#define AHT30_QUEUE_DEPTH 4U

static osMessageQueueId_t s_aht30_queue = NULL;

static void aht30_queue_init_once(void)
{
    if (s_aht30_queue == NULL)
    {
        s_aht30_queue = osMessageQueueNew(AHT30_QUEUE_DEPTH, sizeof(unit_test_aht30_data_t), NULL);
    }
}

void unit_test_aht30_channel_init(void)
{
    aht30_queue_init_once();
}

bool unit_test_aht30_publish(const unit_test_aht30_data_t *data)
{
    if (data == NULL)
    {
        return false;
    }

    aht30_queue_init_once();
    if (s_aht30_queue == NULL)
    {
        return false;
    }

    osStatus_t status = osMessageQueuePut(s_aht30_queue, data, 0U, 0U);
    if (status == osErrorResource)
    {
        unit_test_aht30_data_t dropped;
        (void)osMessageQueueGet(s_aht30_queue, &dropped, NULL, 0U);
        status = osMessageQueuePut(s_aht30_queue, data, 0U, 0U);
    }

    return status == osOK;
}

bool unit_test_aht30_receive(unit_test_aht30_data_t *data, uint32_t timeout_ms)
{
    if (data == NULL)
    {
        return false;
    }

    aht30_queue_init_once();
    if (s_aht30_queue == NULL)
    {
        return false;
    }

    osStatus_t status = osMessageQueueGet(s_aht30_queue, data, NULL, timeout_ms);
    return status == osOK;
}

HAL_StatusTypeDef aht30_test_log_measurement(void)
{
    float temperature = 0.0f;
    float humidity = 0.0f;

    HAL_StatusTypeDef status = aht30_read(&temperature, &humidity);

    if (status == HAL_OK) {
        int16_t  temp10 = (int16_t)(temperature * 10.0f);
        uint16_t hum10  = (uint16_t)(humidity * 10.0f);

//        elog_i("unit_test_aht30", "T=%d.%01dC  RH=%u.%01u%%",
//               (int)(temp10 / 10), (int)abs(temp10 % 10),
//               (unsigned)(hum10 / 10), (unsigned)(hum10 % 10));

        unit_test_aht30_data_t msg = {
            .temperature = temperature,
            .humidity = humidity};
        (void)unit_test_aht30_publish(&msg);

    } else if (status == HAL_BUSY) {
        elog_w("unit_test_aht30","AHT30 measurement busy");
    } else {
            
        elog_e("unit_test_aht30", "error (status=%d)", (int)status);
        
    }

    return status;
}

void aht30_Task(void *argument)
{
    unit_test_aht30_channel_init();
	
	 if (aht30_init() != HAL_OK)
  {
     elog_e("unit_test_aht30", "aht30_init() err.");
  }
	
  for(;;)
  {
		
		aht30_test_log_measurement();
    osDelay(1000);
  }
}
