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
#include "mbedtls.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "mbedtls/aes.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RSA_KEY_BITS        1024U
#define RSA_KEY_BYTES       (RSA_KEY_BITS / 8U)

static const char RSA_N_HEX[] =
    "9292758453063D803DD603D5E777D788"
    "8ED1D5BF35786190FA2F23EBC0848AEA"
    "DDA92CA6C3D80B32C4D109BE0F36D6AE"
    "7130B9CED7ACDF54CFC7555AC14EEBAB"
    "93A89813FBF3C4F8066D2D800F7C38A8"
    "1AE31942917403FF4946B0A83D3D3E05"
    "EE57C6F5F5606FB5D4BC6CD34EE0801A"
    "5E94BB77B07507233A0BC7BAC8F90F79";

static const char RSA_E_HEX[] = "10001";
static const char RSA_D_HEX[] =
    "24BF6185468786FDD303083D25E64EFC"
    "66CA472BC44D253102F8B4A9D3BFA750"
    "91386C0077937FE33FA3252D28855837"
    "AE1B484A8A9A45F7EE8C0C634F99E8CD"
    "DF79C5CE07EE72C7F123142198164234"
    "CABB724CF78B8173B9F880FC86322407"
    "AF1FEDFDDE2BEB674CA15F3E81A1521E"
    "071513A1E85B5DFA031F21ECAE91A34D";
static const char RSA_P_HEX[] =
    "C36D0EB7FCD285223CFB5AABA5BDA3D8"
    "2C01CAD19EA484A87EA4377637E75500"
    "FCB2005C5C7DD6EC4AC023CDA285D796"
    "C3D9E75E1EFC42488BB4F1D13AC30A57";
static const char RSA_Q_HEX[] =
    "C000DF51A7C77AE8D7C7370C1FF55B69"
    "E211C2B9E5DB1ED0BF61D0D9899620F4"
    "910E4168387E3C30AA1E00C339A79508"
    "8452DD96A9A5EA5D9DCA68DA636032AF";

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
static void run_mbedtls_tests(void);
static int run_aes_test(void);
static int run_rsa_test(void);
static int run_sha256_test(void);
static void print_hex(const char *label, const uint8_t *buffer, size_t length);
static void log_status(const char *name, int ret);
static int pseudo_entropy_source(void *ctx, unsigned char *output, size_t len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 以十六进制打印缓冲区内容，方便串口观察数据 */
static void print_hex(const char *label, const uint8_t *buffer, size_t length)
{
  printf("%s", label);
  for (size_t i = 0; i < length; i++)
  {
    printf("%02X", buffer[i]);
  }
  printf("\r\n");
}

/* 统一输出 PASS/FAIL 字样，便于调试与脚本解析 */
static void log_status(const char *name, int ret)
{
  if (ret == 0)
  {
    printf("%s: PASS\r\n", name);
  }
  else
  {
    printf("%s: FAIL (ret=%d)\r\n", name, ret);
  }
}

/* 简单的伪随机发生器，用作 CTR_DRBG 的种子输入 */
static int pseudo_entropy_source(void *ctx, unsigned char *output, size_t len)
{
  uint32_t *seed = (uint32_t *)ctx;
  for (size_t i = 0; i < len; i++)
  {
    *seed = (*seed * 1664525UL) + 1013904223UL;
    output[i] = (uint8_t)(*seed >> 24);
  }
  return 0;
}

/* 使用 NIST 向量完成 AES-128 ECB 加/解密校验 */
static int run_aes_test(void)
{
  int ret = 0;
  const uint8_t key[16] = {
      0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
      0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
  const uint8_t plain[16] = {
      0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
      0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
  const uint8_t expected_cipher[16] = {
      0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
      0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};
  uint8_t cipher[16];
  uint8_t decrypted[16];
  mbedtls_aes_context aes;

  mbedtls_aes_init(&aes);

  if ((ret = mbedtls_aes_setkey_enc(&aes, key, 128)) != 0)
  {
    goto cleanup;
  }

  if ((ret = mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, plain, cipher)) != 0)
  {
    goto cleanup;
  }

  if (memcmp(cipher, expected_cipher, sizeof(cipher)) != 0)
  {
    ret = -1;
    goto cleanup;
  }

  if ((ret = mbedtls_aes_setkey_dec(&aes, key, 128)) != 0)
  {
    goto cleanup;
  }

  if ((ret = mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, cipher, decrypted)) != 0)
  {
    goto cleanup;
  }

  if (memcmp(decrypted, plain, sizeof(plain)) != 0)
  {
    ret = -1;
    goto cleanup;
  }

  print_hex("AES cipher: ", cipher, sizeof(cipher));

cleanup:
  mbedtls_aes_free(&aes);
  return ret;
}

/* 导入固定 RSA-1024 密钥对后执行 PKCS#1 v1.5 加解密 */
static int run_rsa_test(void)
{
  int ret = 0;
  const unsigned char message[] = "RocketPi RSA demo";
  unsigned char ciphertext[RSA_KEY_BYTES];
  unsigned char decrypted[sizeof(message)];
  size_t olen = 0;
  mbedtls_rsa_context rsa;
  mbedtls_mpi value;
  uint32_t seed = 0x12345678UL;

  mbedtls_mpi_init(&value);
  mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);

  do
  {
    if ((ret = mbedtls_mpi_read_string(&value, 16, RSA_N_HEX)) != 0)
    {
      printf("RSA load N failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_import(&rsa, &value, NULL, NULL, NULL, NULL)) != 0)
    {
      printf("RSA import N failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_mpi_read_string(&value, 16, RSA_P_HEX)) != 0)
    {
      printf("RSA load P failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_import(&rsa, NULL, &value, NULL, NULL, NULL)) != 0)
    {
      printf("RSA import P failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_mpi_read_string(&value, 16, RSA_Q_HEX)) != 0)
    {
      printf("RSA load Q failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_import(&rsa, NULL, NULL, &value, NULL, NULL)) != 0)
    {
      printf("RSA import Q failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_mpi_read_string(&value, 16, RSA_D_HEX)) != 0)
    {
      printf("RSA load D failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_import(&rsa, NULL, NULL, NULL, &value, NULL)) != 0)
    {
      printf("RSA import D failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_mpi_read_string(&value, 16, RSA_E_HEX)) != 0)
    {
      printf("RSA load E failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_import(&rsa, NULL, NULL, NULL, NULL, &value)) != 0)
    {
      printf("RSA import E failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_complete(&rsa)) != 0)
    {
      printf("RSA complete failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_check_pubkey(&rsa)) != 0)
    {
      printf("RSA public check failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_check_privkey(&rsa)) != 0)
    {
      printf("RSA private check failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_pkcs1_encrypt(&rsa, pseudo_entropy_source, &seed,
                                         MBEDTLS_RSA_PUBLIC, sizeof(message) - 1U, message, ciphertext)) != 0)
    {
      printf("RSA encrypt failed: %d\r\n", ret);
      break;
    }

    if ((ret = mbedtls_rsa_pkcs1_decrypt(&rsa, pseudo_entropy_source, &seed,
                                         MBEDTLS_RSA_PRIVATE, &olen, ciphertext, decrypted, sizeof(decrypted))) != 0)
    {
      printf("RSA decrypt failed: %d\r\n", ret);
      break;
    }
  } while (0);

  if (ret == 0)
  {
    if ((olen != (sizeof(message) - 1U)) || (memcmp(decrypted, message, olen) != 0))
    {
      ret = -1;
      goto cleanup;
    }

    decrypted[olen] = '\0';
    printf("RSA plaintext: %s\r\n", decrypted);
  }

cleanup:
  mbedtls_mpi_free(&value);
  mbedtls_rsa_free(&rsa);
  return ret;
}

/* 对固定字符串求 SHA-256，验证哈希正确性 */
static int run_sha256_test(void)
{
  int ret = 0;
  const unsigned char message[] = "RocketPi MBEDTLS";
  const uint8_t expected_hash[32] = {
      0xb9, 0x2f, 0x7c, 0xae, 0xa1, 0x4f, 0x3b, 0x71,
      0xfe, 0xbb, 0x57, 0x98, 0x75, 0x65, 0x17, 0x7d,
      0xb1, 0x03, 0x11, 0x61, 0x93, 0xd7, 0x9e, 0x85,
      0x1e, 0x17, 0x6b, 0x13, 0x91, 0x88, 0xb3, 0x63};
  uint8_t hash[32];
  mbedtls_sha256_context ctx;

  mbedtls_sha256_init(&ctx);

  if ((ret = mbedtls_sha256_starts_ret(&ctx, 0)) != 0)
  {
    goto cleanup;
  }

  if ((ret = mbedtls_sha256_update_ret(&ctx, message, sizeof(message) - 1U)) != 0)
  {
    goto cleanup;
  }

  if ((ret = mbedtls_sha256_finish_ret(&ctx, hash)) != 0)
  {
    goto cleanup;
  }

  if (memcmp(hash, expected_hash, sizeof(hash)) != 0)
  {
    ret = -1;
    goto cleanup;
  }

  print_hex("SHA-256: ", hash, sizeof(hash));

cleanup:
  mbedtls_sha256_free(&ctx);
  return ret;
}

/* 串行执行三项测试并输出最终结果 */
static void run_mbedtls_tests(void)
{
  printf("\r\n==== mbed TLS sample tests ====\r\n");
  log_status("AES-128 ECB", run_aes_test());
  log_status("RSA-1024 PKCS#1 v1.5", run_rsa_test());
  log_status("SHA-256 digest", run_sha256_test());
  printf("==== tests finished ====\r\n");
}

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
  MX_MBEDTLS_Init();
  /* USER CODE BEGIN 2 */
  run_mbedtls_tests();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_Delay(1000);
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
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
