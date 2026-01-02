#include "unit_test.h"

#include "multi_button.h"

static volatile unit_test_button_state_t g_button_state = UNIT_TEST_BUTTON_STATE_IDLE;

static uint8_t ButtonPinLevel(uint8_t button_id)
{
  (void)button_id;
  return (uint8_t)HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
}

static void ButtonSingleClickHandler(Button *btn)
{
  (void)btn;
  g_button_state = UNIT_TEST_BUTTON_STATE_SHORT;
  elog_d("unit_test_button", "ButtonSingleClick.");
}

static void ButtonDoubleClickHandler(Button *btn)
{
  (void)btn;
  g_button_state = UNIT_TEST_BUTTON_STATE_DOUBLE;
  elog_d("unit_test_button", "ButtonDoubleClick.");
}

static void ButtonLongPressHandler(Button *btn)
{
  (void)btn;
  g_button_state = UNIT_TEST_BUTTON_STATE_LONG;
  elog_d("unit_test_button", "ButtonLongPress.");
}

#define USER_BUTTON_ACTIVE_LEVEL GPIO_PIN_SET
static Button g_user_button;


void button_Task(void *argument)
{	
	osDelay(500);
	button_init(&g_user_button, ButtonPinLevel, USER_BUTTON_ACTIVE_LEVEL, 0);
  button_attach(&g_user_button, BTN_SINGLE_CLICK, ButtonSingleClickHandler);
  button_attach(&g_user_button, BTN_DOUBLE_CLICK, ButtonDoubleClickHandler);
  button_attach(&g_user_button, BTN_LONG_PRESS_START, ButtonLongPressHandler);
  button_start(&g_user_button);
	
  for(;;)
  {
		static uint32_t s_button_tick_divider = 0U;
			if (++s_button_tick_divider >= TICKS_INTERVAL) {
					s_button_tick_divider = 0U;
					button_ticks();
			}
			
     osDelay(1);
  }
}

unit_test_button_state_t unit_test_button_get_state(void)
{
  unit_test_button_state_t state;
  taskENTER_CRITICAL();
  state = g_button_state;
  g_button_state = UNIT_TEST_BUTTON_STATE_IDLE;
  taskEXIT_CRITICAL();
  return state;
}
