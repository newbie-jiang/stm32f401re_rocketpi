/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_JOYSTICK_KEY_Pin GPIO_PIN_13
#define ADC_JOYSTICK_KEY_GPIO_Port GPIOC
#define AT24CXX_SCL_Pin GPIO_PIN_14
#define AT24CXX_SCL_GPIO_Port GPIOC
#define AT24CXX_SDA_Pin GPIO_PIN_15
#define AT24CXX_SDA_GPIO_Port GPIOC
#define SDIO_CARD_CD_Pin GPIO_PIN_0
#define SDIO_CARD_CD_GPIO_Port GPIOC
#define LCD_RST_Pin GPIO_PIN_1
#define LCD_RST_GPIO_Port GPIOC
#define LCD_DC_Pin GPIO_PIN_3
#define LCD_DC_GPIO_Port GPIOC
#define KEY_Pin GPIO_PIN_0
#define KEY_GPIO_Port GPIOA
#define LED_B_Pin GPIO_PIN_1
#define LED_B_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_4
#define LCD_CS_GPIO_Port GPIOA
#define ADC_JOYSTICK_X_Pin GPIO_PIN_4
#define ADC_JOYSTICK_X_GPIO_Port GPIOC
#define ADC_JOYSTICK_Y_Pin GPIO_PIN_5
#define ADC_JOYSTICK_Y_GPIO_Port GPIOC
#define BUZZER_Pin GPIO_PIN_0
#define BUZZER_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_10
#define LED_G_GPIO_Port GPIOB
#define LED_P_Pin GPIO_PIN_14
#define LED_P_GPIO_Port GPIOB
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define IRDA_IO_Pin GPIO_PIN_15
#define IRDA_IO_GPIO_Port GPIOA
#define IRDA_IO_EXTI_IRQn EXTI15_10_IRQn
#define LCD_BL_Pin GPIO_PIN_5
#define LCD_BL_GPIO_Port GPIOB
#define AHT30_SCL_Pin GPIO_PIN_8
#define AHT30_SCL_GPIO_Port GPIOB
#define AHT30_SDA_Pin GPIO_PIN_9
#define AHT30_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
