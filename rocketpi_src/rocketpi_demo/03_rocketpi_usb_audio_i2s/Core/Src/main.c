/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2s.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7789.h"
#include "spectrum.h"
#include "usbd_audio.h"
#include "driver_ws2812b.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WS2812_LED_COUNT           30U
#define WS2812_UPDATE_INTERVAL_MS  20U

#define WS2812_VIS_MODE_SPECTRUM_BAR      0U
#define WS2812_VIS_MODE_ENERGY_SWEEP      1U
#define WS2812_VIS_MODE_RAINBOW_GLOW      2U

#ifndef WS2812_VISUAL_MODE
#define WS2812_VISUAL_MODE WS2812_VIS_MODE_SPECTRUM_BAR
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t ws2812_ready = 0U;
static uint32_t ws2812_last_update_ms = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void Visualizer_UpdateStrip(void);
static void Visualizer_ModeSpectrumBar(uint32_t leds, uint32_t bins, uint16_t max_height);
static void Visualizer_ModeEnergySweep(uint32_t leds, uint32_t bins, uint16_t max_height);
static void Visualizer_ModeRainbowGlow(uint32_t leds, uint32_t bins, uint16_t max_height);
static void Visualizer_Color565ToRGB(uint16_t color565, uint8_t *r, uint8_t *g, uint8_t *b);
static void Visualizer_ColorWheel(uint32_t pos, uint32_t total, uint8_t *r, uint8_t *g, uint8_t *b);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S2_Init();
  MX_USB_DEVICE_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  ST7789_Init();
  Spectrum_Init(USBD_AUDIO_FREQ);
  if (ws2812b_init(WS2812_LED_COUNT))
  {
    ws2812_ready = 1U;
    ws2812b_fill(0U, 0U, 0U);
    (void)ws2812b_refresh();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Spectrum_DrawIfDue();
    Visualizer_UpdateStrip();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void Visualizer_UpdateStrip(void)
{
  if ((ws2812_ready == 0U) || ws2812b_is_busy())
  {
    return;
  }

  uint32_t now = HAL_GetTick();
  if ((now - ws2812_last_update_ms) < WS2812_UPDATE_INTERVAL_MS)
  {
    return;
  }
  ws2812_last_update_ms = now;

  uint32_t leds = ws2812b_get_led_count();
  uint32_t bins = Spectrum_GetBinCount();
  if ((leds == 0U) || (bins == 0U))
  {
    return;
  }

  uint16_t max_height = Spectrum_GetMaxHeight();
  if (max_height == 0U)
  {
    max_height = 1U;
  }

  switch (WS2812_VISUAL_MODE)
  {
    case WS2812_VIS_MODE_SPECTRUM_BAR:
      Visualizer_ModeSpectrumBar(leds, bins, max_height);
      break;
    case WS2812_VIS_MODE_ENERGY_SWEEP:
      Visualizer_ModeEnergySweep(leds, bins, max_height);
      break;
    case WS2812_VIS_MODE_RAINBOW_GLOW:
      Visualizer_ModeRainbowGlow(leds, bins, max_height);
      break;
    default:
      Visualizer_ModeSpectrumBar(leds, bins, max_height);
      break;
  }

  (void)ws2812b_refresh();
}

static void Visualizer_ModeSpectrumBar(uint32_t leds, uint32_t bins, uint16_t max_height)
{
  for (uint32_t led = 0U; led < leds; ++led)
  {
    uint32_t bin = (led * bins) / leds;
    uint16_t level = Spectrum_GetLevel(bin);
    uint16_t color565 = Spectrum_GetBinColor(bin);

    uint8_t r, g, b;
    Visualizer_Color565ToRGB(color565, &r, &g, &b);

    uint32_t scale = ((uint32_t)level * 255U) / max_height;
    r = (uint8_t)((r * scale) / 255U);
    g = (uint8_t)((g * scale) / 255U);
    b = (uint8_t)((b * scale) / 255U);

    (void)ws2812b_set_pixel((uint16_t)led, r, g, b);
  }
}

static void Visualizer_ModeEnergySweep(uint32_t leds, uint32_t bins, uint16_t max_height)
{
  uint32_t sum = 0U;
  for (uint32_t bin = 0U; bin < bins; ++bin)
  {
    sum += Spectrum_GetLevel(bin);
  }
  uint32_t avg = (bins > 0U) ? (sum / bins) : 0U;
  uint32_t active_leds = (avg * leds) / max_height;
  if (active_leds > leds)
  {
    active_leds = leds;
  }

  for (uint32_t led = 0U; led < leds; ++led)
  {
    if (led < active_leds)
    {
      uint32_t bin = (led * bins) / leds;
      uint16_t color565 = Spectrum_GetBinColor(bin);
      uint8_t r, g, b;
      Visualizer_Color565ToRGB(color565, &r, &g, &b);
      (void)ws2812b_set_pixel((uint16_t)led, r, g, b);
    }
    else
    {
      (void)ws2812b_set_pixel((uint16_t)led, 0U, 0U, 0U);
    }
  }
}

static void Visualizer_ModeRainbowGlow(uint32_t leds, uint32_t bins, uint16_t max_height)
{
  if (leds == 0U)
  {
    return;
  }

  for (uint32_t led = 0U; led < leds; ++led)
  {
    uint8_t r, g, b;
    Visualizer_ColorWheel(led, leds, &r, &g, &b);

    uint32_t bin = (led * bins) / leds;
    uint16_t level = Spectrum_GetLevel(bin);
    uint32_t scale = ((uint32_t)level * 255U) / max_height;
    r = (uint8_t)((r * scale) / 255U);
    g = (uint8_t)((g * scale) / 255U);
    b = (uint8_t)((b * scale) / 255U);

    (void)ws2812b_set_pixel((uint16_t)led, r, g, b);
  }
}

static void Visualizer_Color565ToRGB(uint16_t color565, uint8_t *r, uint8_t *g, uint8_t *b)
{
  if (r != NULL)
  {
    *r = (uint8_t)(((color565 >> 11) & 0x1FU) << 3);
  }
  if (g != NULL)
  {
    *g = (uint8_t)(((color565 >> 5) & 0x3FU) << 2);
  }
  if (b != NULL)
  {
    *b = (uint8_t)((color565 & 0x1FU) << 3);
  }
}

static void Visualizer_ColorWheel(uint32_t pos, uint32_t total, uint8_t *r, uint8_t *g, uint8_t *b)
{
  if ((total == 0U) || (r == NULL) || (g == NULL) || (b == NULL))
  {
    return;
  }

  uint32_t angle = (pos * 1536U) / total;
  angle %= 1536U;
  uint32_t region = angle / 256U;
  uint32_t offset = angle % 256U;

  switch (region)
  {
    case 0:
      *r = 255U;
      *g = (uint8_t)offset;
      *b = 0U;
      break;
    case 1:
      *r = (uint8_t)(255U - offset);
      *g = 255U;
      *b = 0U;
      break;
    case 2:
      *r = 0U;
      *g = 255U;
      *b = (uint8_t)offset;
      break;
    case 3:
      *r = 0U;
      *g = (uint8_t)(255U - offset);
      *b = 255U;
      break;
    case 4:
      *r = (uint8_t)offset;
      *g = 0U;
      *b = 255U;
      break;
    default:
      *r = 255U;
      *g = 0U;
      *b = (uint8_t)(255U - offset);
      break;
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
