/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LEDP_TASK */
osThreadId_t LEDP_TASKHandle;
const osThreadAttr_t LEDP_TASK_attributes = {
  .name = "LEDP_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LEDG_TASK */
osThreadId_t LEDG_TASKHandle;
const osThreadAttr_t LEDG_TASK_attributes = {
  .name = "LEDG_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LEDB_TASK */
osThreadId_t LEDB_TASKHandle;
const osThreadAttr_t LEDB_TASK_attributes = {
  .name = "LEDB_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void ledp_task(void *argument);
void ledg_task(void *argument);
void ledb_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of LEDP_TASK */
  LEDP_TASKHandle = osThreadNew(ledp_task, NULL, &LEDP_TASK_attributes);

  /* creation of LEDG_TASK */
  LEDG_TASKHandle = osThreadNew(ledg_task, NULL, &LEDG_TASK_attributes);

  /* creation of LEDB_TASK */
  LEDB_TASKHandle = osThreadNew(ledb_task, NULL, &LEDB_TASK_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_ledp_task */
/**
* @brief Function implementing the LEDP_TASK thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ledp_task */
void ledp_task(void *argument)
{
  /* USER CODE BEGIN ledp_task */
  /* Infinite loop */
  for(;;)
  {
		HAL_GPIO_TogglePin(LED_P_GPIO_Port,LED_P_Pin);
    osDelay(500);
  }
  /* USER CODE END ledp_task */
}

/* USER CODE BEGIN Header_ledg_task */
/**
* @brief Function implementing the LEDG_TASK thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ledg_task */
void ledg_task(void *argument)
{
  /* USER CODE BEGIN ledg_task */
  /* Infinite loop */
  for(;;)
  {
		HAL_GPIO_TogglePin(LED_G_GPIO_Port,LED_G_Pin);
    osDelay(500);
  }
  /* USER CODE END ledg_task */
}

/* USER CODE BEGIN Header_ledb_task */
/**
* @brief Function implementing the LEDB_TASK thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ledb_task */
void ledb_task(void *argument)
{
  /* USER CODE BEGIN ledb_task */
  /* Infinite loop */
  for(;;)
  {
		HAL_GPIO_TogglePin(LED_B_GPIO_Port,LED_B_Pin);
    osDelay(500);
  }
  /* USER CODE END ledb_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

