

按 Winbond 资料，早期的 W25Q16 系列（如 W25Q16BV／BL）确实没有实现 SFDP；只有较新的 W25Qxx 系列（例如 W25Q16JV 及之后的 JV/LM、FW 系列）才在状态寄存器里新增了 SFDP 支持。所以如果你的器件是常见的 W25Q16BV/BL，那读取 SFDP 得到全 0xFF 是正常现象，它本身就没有这块表。

W25Q16BV 是 Winbond 早期的 16 Mbit 器件，不带 SFDP 表，因此读 SFDP 得到全 0xFF 是正常现象（芯片压根没有实现 JESD216）。这类器件也只支持标准 SPI，虽然能进入 Dual/Quad 模式，但很多高版本驱动才用得到。

```
w25qxx: chip is Winbond W25QXX.
w25qxx: manufacturer is Winbond.
w25qxx: interface is SPI QSPI.
w25qxx: driver version is 1.0.
w25qxx: min supply voltage is 2.7V.
w25qxx: max supply voltage is 3.6V.
w25qxx: max current is 25.00mA.
w25qxx: max temperature is 85.0C.
w25qxx: min temperature is -40.0C.
w25qxx: start register test.
w25qxx: w25qxx_set_type/w25qxx_get_type test.
w25qxx: set type W25Q80.
w25qxx: check chip type ok.
w25qxx: set type W25Q16.
w25qxx: check chip type ok.
w25qxx: set type W25Q32.
w25qxx: check chip type ok.
w25qxx: set type W25Q64.
w25qxx: check chip type ok.
w25qxx: set type W25Q128.
w25qxx: check chip type ok.
w25qxx: set type W25Q256.
w25qxx: check chip type ok.
w25qxx: w25qxx_set_interface/w25qxx_get_interface test.
w25qxx: set interface SPI.
w25qxx: check chip interface ok.
w25qxx: set interface QSPI.
w25qxx: check chip interface ok.
w25qxx: w25qxx_get_manufacturer_device_id test.
w25qxx: manufacturer is 0xEF, device id is 0x14.
w25qxx: w25qxx_get_jedec_id test.
w25qxx: manufacturer is 0xEF, device id is 0x40 0x15.
w25qxx: w25qxx_get_unique_id test.
w25qxx: unique id 0xC4 0x69 0xC4 0x32 0xCF 0x25 0x33 0x33.
w25qxx: w25qxx_set_status1/w25qxx_get_status1 test.
w25qxx: status1 is 0x00.
w25qxx: w25qxx_set_status2/w25qxx_get_status2 test.
w25qxx: status2 is 0x00.
w25qxx: w25qxx_set_status3/w25qxx_get_status3 test.
w25qxx: status3 is 0x00.
w25qxx: w25qxx_enable_write test.
w25qxx: check enable write ok.
w25qxx: w25qxx_disable_write test.
w25qxx: check disable write ok.
w25qxx: w25qxx_enable_write test.
w25qxx: check enable sr write ok.
w25qxx: w25qxx_erase_program_suspend test.
w25qxx: check erase program suspend ok.
w25qxx: w25qxx_erase_program_suspend test.
w25qxx: check erase program resume ok.
w25qxx: w25qxx_global_block_lock test.
w25qxx: check global block lock ok.
w25qxx: w25qxx_global_block_unlock test.
w25qxx: check global block unlock ok.
w25qxx: w25qxx_individual_block_lock test.
w25qxx: check individual block lock ok.
w25qxx: w25qxx_read_block_lock test.
w25qxx: check read block lock ok with 0.
w25qxx: w25qxx_individual_block_unlock test.
w25qxx: check individual block unlock ok.
w25qxx: w25qxx_set_burst_with_wrap test.
w25qxx: check set burst with wrap ok.
w25qxx: w25qxx_power_down test.
w25qxx: w25qxx_release_power_down test.
w25qxx: w25qxx_enable_reset test.
w25qxx: w25qxx_reset_device test.
w25qxx: finish register test.
w25qxx: chip is Winbond W25QXX.
w25qxx: manufacturer is Winbond.
w25qxx: interface is SPI QSPI.
w25qxx: driver version is 1.0.
w25qxx: min supply voltage is 2.7V.
w25qxx: max supply voltage is 3.6V.
w25qxx: max current is 25.00mA.
w25qxx: max temperature is 85.0C.
w25qxx: min temperature is -40.0C.
w25qxx: start read test.
w25qxx: w25qxx_write/w25qxx_read test.
w25qxx: 0x00000000/0x00200000 successful.
w25qxx: 0x00020000/0x00200000 successful.
w25qxx: 0x00040000/0x00200000 successful.
w25qxx: 0x00060000/0x00200000 successful.
w25qxx: 0x00080000/0x00200000 successful.
w25qxx: 0x000A0000/0x00200000 successful.
w25qxx: 0x000C0000/0x00200000 successful.
w25qxx: 0x000E0000/0x00200000 successful.
w25qxx: 0x00100000/0x00200000 successful.
w25qxx: 0x00120000/0x00200000 successful.
w25qxx: 0x00140000/0x00200000 successful.
w25qxx: 0x00160000/0x00200000 successful.
w25qxx: 0x00180000/0x00200000 successful.
w25qxx: 0x001A0000/0x00200000 successful.
w25qxx: 0x001C0000/0x00200000 successful.
w25qxx: 0x001E0000/0x00200000 successful.
w25qxx: w25qxx_sector_erase_4k test with address 0x4000.
w25qxx: only spi read test passed.
w25qxx: fast read test passed.
w25qxx: w25qxx_block_erase_32k test with address 0x18000.
w25qxx: only spi read test passed.
w25qxx: fast read test passed.
w25qxx: w25qxx_block_erase_64k test with address 0x80000.
w25qxx: only spi read test passed.
w25qxx: fast read test passed.
w25qxx: get sfdp.
w25qxx: sdfp[0-7] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[8-15] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[16-23] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[24-31] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[32-39] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[40-47] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[48-55] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[56-63] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[64-71] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[72-79] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[80-87] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[88-95] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[96-103] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[104-111] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[112-119] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[120-127] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[128-135] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[136-143] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[144-151] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[152-159] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[160-167] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[168-175] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[176-183] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[184-191] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[192-199] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[200-207] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[208-215] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[216-223] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[224-231] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[232-239] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[240-247] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: sdfp[248-255] is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
w25qxx: security register1 write and read test.
w25qxx: write read check failed.

```

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:2f187e145bcd517c63a4d07b0872eb7a28436a2e1d5ba8671f9bd53d43ebe3c3 -->
## 驱动以及测试代码

<details>
<summary>Core/Src/main.c</summary>

```c
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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "driver_w25qxx_read_test.h"
#include "driver_w25qxx_register_test.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
	w25qxx_register_test(W25Q64,W25QXX_INTERFACE_SPI,W25QXX_BOOL_FALSE);
  w25qxx_read_test(W25Q64,W25QXX_INTERFACE_SPI,W25QXX_BOOL_FALSE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
```

</details>

<details>
<summary>Core/Src/stm32f4xx_it.c</summary>

```c
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
```

</details>

<!-- BSP_DRIVERS_END -->
