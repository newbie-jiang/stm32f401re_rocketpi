## 效果展示

![image-20251119030904123](https://cloud.rocketpi.club/cloud/image-20251119030904123.png)

## 功能说明

使用内部的flash后两个扇区  做littlefs文件系统

Sector 6 0x0804 0000 - 0x0805 FFFF 128 Kbytes

Sector 7 0x0806 0000 - 0x0807 FFFF 128 Kbytes

支持特性

- 128 位宽数据读取
- 字节、半字、字和双字写入
- 扇区和整片擦除

内部flash适合做不太频写的增量记录，比如日志，擦写次数有限，littlefs均衡磨损优于fatfs

littlefs src   https://github.com/littlefs-project/littlefs/releases/tag/v2.11.2

![image-20251119022932746](https://cloud.rocketpi.club/cloud/image-20251119022932746.png)

## littlefs  vs FatFs

## （针对 MCU / SPI NOR 裸 Flash）

- **littlefs**
  - 设计目标就是：**直接跑在 NOR Flash 这类擦写受限的介质上**。
  - 内部做了 **日志式结构 + 动态/静态磨损均衡 + 断电一致性**。
  - 同一块数据反复更新时，不会总落在同一个物理块上，而是不断在不同块之间轮转。
- **FatFs**
  - FatFs 本身只是 FAT 文件系统的实现，它假设下面是一个**“块设备”**：扇区读写随便搞，没有擦写限制。
  - **它自己不做磨损均衡**，所有写在哪个扇区由 FAT 逻辑决定，**经常改的目录区/FAT 表区会被疯狂写爆**。
  - 如果下面是“带 FTL 的设备”（SD 卡、U 盘、eMMC），**磨损均衡是控制器在做，不是 FatFs 在做**。

👉 所以：

- **裸 NOR / MCU 内部 Flash**：
  - littlefs = 专门为这种介质设计，自带磨损均衡。
  - FatFs = 不会帮你做磨损均衡，用不好是“找死”的玩法。
- **SD 卡、U 盘这类带控制器的设备**：
  - 磨损均衡主要看存储芯片的控制器质量，
  - 用 littlefs 还是 FatFs，磨损均衡差异就没那么关键了。



## CubeMX配置

只需要配置usart即可

![image-20251119031201158](https://cloud.rocketpi.club/cloud/image-20251119031201158.png)

堆栈设置为0x1000

![image-20251119031229195](https://cloud.rocketpi.club/cloud/image-20251119031229195.png)

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:f8adc9914c010fe06555956166b305be70af0fbed51e606ea71626c1c706fe87 -->
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>

#include "lfs.h"
#include "lfs_port.h"
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
static lfs_t g_lfs;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void LittleFS_Test(void);
static void LittleFS_Log(const char *msg);
static void LittleFS_LogError(const char *msg, int err);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void LittleFS_Log(const char *msg)
{
  if (msg == NULL) {
    return;
  }

  HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

static void LittleFS_LogError(const char *msg, int err)
{
  char buffer[96];
  int len = snprintf(buffer, sizeof(buffer),
                     "%s (err=%d)\r\n", msg, err);
  if ((len > 0) && (len < (int)sizeof(buffer))) {
    LittleFS_Log(buffer);
  }
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
  /* USER CODE BEGIN 2 */
  LittleFS_Test();

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
static void LittleFS_Test(void)
{
  struct lfs_file file;
  char read_buffer[64] = {0};
  const char *path = "flash.txt";
  const char *payload = "LittleFS STM32F401 Flash Demo\r\n";

  int err = lfs_port_mount(&g_lfs);
  if (err != LFS_ERR_OK) {
    LittleFS_LogError("LittleFS: initial mount failed", err);
    LittleFS_Log("LittleFS: formatting flash...\r\n");
    err = lfs_port_format(&g_lfs);
    if (err == LFS_ERR_OK) {
      LittleFS_Log("LittleFS: format done\r\n");
      err = lfs_port_mount(&g_lfs);
    } else {
      LittleFS_LogError("LittleFS: format failed", err);
    }
  }

  if (err != LFS_ERR_OK) {
    LittleFS_LogError("LittleFS: mount failed", err);
    return;
  }

  err = lfs_file_open(&g_lfs, &file, path,
                      LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
  if (err < 0) {
    LittleFS_LogError("LittleFS: open for write failed", err);
    lfs_port_unmount(&g_lfs);
    return;
  }

  lfs_ssize_t written = lfs_file_write(&g_lfs, &file, payload, strlen(payload));
  lfs_file_close(&g_lfs, &file);

  if (written < 0) {
    LittleFS_LogError("LittleFS: write failed", (int)written);
    lfs_port_unmount(&g_lfs);
    return;
  }

  err = lfs_file_open(&g_lfs, &file, path, LFS_O_RDONLY);
  if (err < 0) {
    LittleFS_LogError("LittleFS: open for read failed", err);
    lfs_port_unmount(&g_lfs);
    return;
  }

  lfs_ssize_t read = lfs_file_read(&g_lfs, &file, read_buffer,
                                   sizeof(read_buffer) - 1U);
  lfs_file_close(&g_lfs, &file);

  if (read < 0) {
    LittleFS_LogError("LittleFS: read failed", (int)read);
    lfs_port_unmount(&g_lfs);
    return;
  }

  read_buffer[read] = '\0';

  char log_buffer[128];
  int len = snprintf(log_buffer, sizeof(log_buffer),
                     "LittleFS OK (%ld bytes): %s\r\n",
                     (long)read, read_buffer);
  if ((len > 0) && (len < (int)sizeof(log_buffer))) {
    LittleFS_Log(log_buffer);
  }

  lfs_port_unmount(&g_lfs);
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
```

</details>

<details>
<summary>Core/Src/retarget.c</summary>

```c
/**
 * @file retarget.c
 * @brief Bare-metal stubs required when semihosting is disabled.
 */

#include <stdint.h>

void _ttywrch(int ch)
{
    (void)ch;
}
```

</details>

<details>
<summary>bsp/debug/debug_driver.c</summary>

```c
#include "debug_driver.h"

#include "usart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/**
 * @file debug_driver.c
 * @brief Retarget low-level stdio functions to USART2 for debugging output/input.
 * @author rocket
 * @copyright Copyright (c) 2025 rocket. Authorized use only.
 */

#ifdef __GNUC__  // GCC
/**
 * @brief Retarget newlib write syscall to the USART debug port.
 * @param file Logical file descriptor supplied by the C library (unused).
 * @param ptr Buffer containing the data to be transmitted.
 * @param len Number of bytes to be transmitted.
 * @return Number of bytes written.
 */
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

/**
 * @brief Retarget newlib read syscall to the USART debug port.
 * @param file Logical file descriptor supplied by the C library (unused).
 * @param ptr Destination buffer.
 * @param len Number of bytes to be read.
 * @return Number of bytes read.
 */
int _read(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Receive(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}
#elif defined(__ARMCC_VERSION)  // Keil
/**
 * @brief Retarget fputc to the USART debug port for Arm Compiler/Keil.
 * @param ch Character to transmit.
 * @param f Ignored file handle.
 * @return The transmitted character.
 */
int fputc(int ch, FILE *f)
{
    (void)f;
    uint8_t data = (uint8_t)ch;
    HAL_UART_Transmit(&huart2, &data, 1U, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief Retarget fgetc to the USART debug port for Arm Compiler/Keil.
 * @param f Ignored file handle.
 * @return The received character.
 */
int fgetc(FILE *f)
{
    (void)f;
    uint8_t ch;
    HAL_UART_Receive(&huart2, &ch, 1U, HAL_MAX_DELAY);
    return (int)ch;
}

#ifndef __MICROLIB
  /* Disable semihosting when using the standard C library (non-Microlib). */
  #pragma import(__use_no_semihosting)

  struct __FILE
  {
      int handle;
  };

  FILE __stdout;
  FILE __stdin;

  /**
   * @brief Stub exit handler so Keil does not fall back to semihosting.
   * @param x Exit code (ignored).
   */
  void _sys_exit(int x)
  {
      (void)x;
  }
#endif
#else
    #error "Unsupported compiler"
#endif

/* ========= Configuration section ========= */
#ifndef UART_LOG_INSTANCE
#define UART_LOG_INSTANCE  huart2      // Replace with the UART handle you want to use
#endif

#ifndef UART_LOG_TIMEOUT
#define UART_LOG_TIMEOUT   1000        // Send timeout (ms)
#endif

#ifndef UART_LOG_BUF_SIZE
#define UART_LOG_BUF_SIZE  256         // Size of the formatting buffer
#endif
/* ======================================== */

/**
 * @brief Blocking UART transmit helper (call outside of ISR context).
 */
static inline void uart_write_blocking(const uint8_t *data, size_t len)
{
    HAL_UART_Transmit(&UART_LOG_INSTANCE, (uint8_t *)data, (uint16_t)len, UART_LOG_TIMEOUT);
}

/**
 * @brief Normalize newlines to CRLF before sending to the terminal.
 */
static void uart_write_with_crlf(const char *s, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (c == '\n') {
            const char crlf[2] = {'\r','\n'};
            uart_write_blocking((const uint8_t*)crlf, 2);
        } else {
            uart_write_blocking((const uint8_t*)&c, 1);
        }
    }
}

/**
 * @brief Output a zero-terminated string (with automatic CRLF normalization).
 */
void uart_puts(const char *s)
{
    if (s == NULL) {
        return;
    }

    uart_write_with_crlf(s, strlen(s));
}

/**
 * @brief printf-style helper built on top of the debug UART.
 * @return Number of characters that would have been written, or a negative error.
 */
int uart_printf(const char *fmt, ...)
{
    char buf[UART_LOG_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n < 0) return n;  
    size_t out_len = (n < (int)sizeof(buf)) ? (size_t)n : (size_t)sizeof(buf) - 1;
    uart_write_with_crlf(buf, out_len);

    /* 
    if (n >= (int)sizeof(buf)) {
        uart_puts("...[truncated]\n");
    }
    */
    return n;
}

/**
 * @brief Hex dump helper for quick binary inspection.
 */
void uart_hexdump(const void *data, size_t len, const char *title)
{
    const uint8_t *p = (const uint8_t*)data;
    if (title) uart_printf("%s (len=%u):\n", title, (unsigned)len);

    char line[80];
    for (size_t i = 0; i < len; i += 16) {
        int pos = 0;
        pos += snprintf(line + pos, sizeof(line) - pos, "%08X  ", (unsigned)i);

        /* hex */
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < len) pos += snprintf(line + pos, sizeof(line) - pos, "%02X ", p[i + j]);
            else              pos += snprintf(line + pos, sizeof(line) - pos, "   ");
            if (j == 7) pos += snprintf(line + pos, sizeof(line) - pos, " ");
        }

        /* ascii */
        pos += snprintf(line + pos, sizeof(line) - pos, " |");
        for (size_t j = 0; j < 16 && i + j < len; ++j) {
            uint8_t c = p[i + j];
            pos += snprintf(line + pos, sizeof(line) - pos, "%c", (c >= 32 && c <= 126) ? c : '.');
        }
        pos += snprintf(line + pos, sizeof(line) - pos, "|\n");

        uart_puts(line);
    }
}
```

</details>

<details>
<summary>bsp/debug/debug_driver.h</summary>

```c
#pragma once

#include <stddef.h>
#include <stdio.h>

/**
 * @file debug_driver.h
 * @brief Interfaces for redirecting stdio calls to the debug UART and lightweight log helpers.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
/**
 * @brief Retarget newlib write syscall to the configured debug UART.
 * @param file Logical file descriptor supplied by the C library.
 * @param ptr Pointer to the transmit buffer.
 * @param len Number of bytes to be written.
 * @return Number of bytes reported as written.
 */
int _write(int file, char *ptr, int len);

/**
 * @brief Retarget newlib read syscall to the configured debug UART.
 * @param file Logical file descriptor supplied by the C library.
 * @param ptr Pointer to the receive buffer.
 * @param len Number of bytes requested.
 * @return Number of bytes actually read.
 */
int _read(int file, char *ptr, int len);
#elif defined(__ARMCC_VERSION)
/**
 * @brief Retarget fputc calls to the configured debug UART.
 * @param ch Character to emit.
 * @param f Ignored file handle.
 * @return The character that was transmitted.
 */
int fputc(int ch, FILE *f);

/**
 * @brief Retarget fgetc calls to the configured debug UART.
 * @param f Ignored file handle.
 * @return Received character cast to int.
 */
int fgetc(FILE *f);

/**
 * @brief Disable the semihosting exit hook when using the standard C library.
 * @param x Exit code (unused).
 */
void _sys_exit(int x);
#endif

/**
 * @brief Send a zero-terminated string over the debug UART with CRLF normalization.
 * @param s String to transmit (may be NULL).
 */
void uart_puts(const char *s);

/**
 * @brief printf-style helper forwarding formatted text to the debug UART.
 * @param fmt Format string describing the output.
 * @return Number of characters that would have been written, or negative on error.
 */
int uart_printf(const char *fmt, ...);

/**
 * @brief Hex dump utility for visualizing binary buffers.
 * @param data Buffer start pointer.
 * @param len Number of bytes to dump.
 * @param title Optional title string, may be NULL.
 */
void uart_hexdump(const void *data, size_t len, const char *title);

#ifdef __cplusplus
}
#endif
```

</details>

<details>
<summary>component/littlefs-2.11.2/lfs_port.c</summary>

```c

#include "lfs_port.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash_ex.h"

#include <stdbool.h>
#include <string.h>

#define LFS_PORT_PROG_SIZE          (4U)      /* Word programming */
#define LFS_PORT_READ_SIZE          (16U)     /* 128-bit reads */
#define LFS_PORT_CACHE_SIZE         (256U)
#define LFS_PORT_LOOKAHEAD_SIZE     (16U)

#define LFS_PORT_FIRST_SECTOR       FLASH_SECTOR_6

#if defined(FLASH_FLAG_RDERR)
#define LFS_PORT_FLASH_ERROR_FLAGS (FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |     \
                                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |    \
                                    FLASH_FLAG_PGSERR | FLASH_FLAG_RDERR)
#else
#define LFS_PORT_FLASH_ERROR_FLAGS (FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |     \
                                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |    \
                                    FLASH_FLAG_PGSERR)
#endif


static int lfs_port_read(const struct lfs_config *cfg, lfs_block_t block,
                         lfs_off_t off, void *buffer, lfs_size_t size);
static int lfs_port_prog(const struct lfs_config *cfg, lfs_block_t block,
                         lfs_off_t off, const void *buffer, lfs_size_t size);
static int lfs_port_erase(const struct lfs_config *cfg, lfs_block_t block);
static int lfs_port_sync(const struct lfs_config *cfg);
static void lfs_port_clear_flash_flags(void);

const struct lfs_config lfs_port_cfg = {
    .context = NULL,
    .read = lfs_port_read,
    .prog = lfs_port_prog,
    .erase = lfs_port_erase,
    .sync = lfs_port_sync,
    .read_size = LFS_PORT_READ_SIZE,
    .prog_size = LFS_PORT_PROG_SIZE,
    .block_size = LFS_PORT_FLASH_BLOCK_SIZE,
    .block_count = LFS_PORT_FLASH_BLOCK_COUNT,
    .cache_size = LFS_PORT_CACHE_SIZE,
    .lookahead_size = LFS_PORT_LOOKAHEAD_SIZE,
    .block_cycles = 100,
};

static inline uint32_t lfs_port_block_address(lfs_block_t block)
{
    return LFS_PORT_FLASH_START_ADDR + (block * LFS_PORT_FLASH_BLOCK_SIZE);
}

static bool lfs_port_range_valid(lfs_block_t block, lfs_off_t off, lfs_size_t size)
{
    if (block >= LFS_PORT_FLASH_BLOCK_COUNT) {
        return false;
    }

    if (off + size > LFS_PORT_FLASH_BLOCK_SIZE) {
        return false;
    }

    const uint32_t start = lfs_port_block_address(block) + off;
    const uint32_t end = start + size;

    return (start >= LFS_PORT_FLASH_START_ADDR) &&
           (end <= LFS_PORT_FLASH_END_ADDR);
}

static void lfs_port_clear_flash_flags(void)
{
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | LFS_PORT_FLASH_ERROR_FLAGS);
}

static int lfs_port_read(const struct lfs_config *cfg, lfs_block_t block,
                         lfs_off_t off, void *buffer, lfs_size_t size)
{
    (void)cfg;

    if (!lfs_port_range_valid(block, off, size)) {
        return LFS_ERR_CORRUPT;
    }

    const void *src = (const void *)(lfs_port_block_address(block) + off);
    memcpy(buffer, src, size);
    return LFS_ERR_OK;
}

static HAL_StatusTypeDef lfs_port_flash_program(uint32_t address,
                                                const uint8_t *data,
                                                size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;

    while ((address % sizeof(uint32_t)) && size && (status == HAL_OK)) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, *data);
        address += 1U;
        data += 1U;
        size -= 1U;
    }

    while ((size >= sizeof(uint32_t)) && (status == HAL_OK)) {
        uint32_t word32;
        memcpy(&word32, data, sizeof(word32));
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, word32);
        address += sizeof(uint32_t);
        data += sizeof(uint32_t);
        size -= sizeof(uint32_t);
    }

    while ((size >= sizeof(uint16_t)) && (status == HAL_OK)) {
        uint16_t halfword;
        memcpy(&halfword, data, sizeof(halfword));
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, halfword);
        address += sizeof(uint16_t);
        data += sizeof(uint16_t);
        size -= sizeof(uint16_t);
    }

    while (size && (status == HAL_OK)) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, *data);
        address += 1U;
        data += 1U;
        size -= 1U;
    }

    return status;
}

static int lfs_port_prog(const struct lfs_config *cfg, lfs_block_t block,
                         lfs_off_t off, const void *buffer, lfs_size_t size)
{
    (void)cfg;

    if (!lfs_port_range_valid(block, off, size)) {
        return LFS_ERR_CORRUPT;
    }

    uint32_t address = lfs_port_block_address(block) + off;

    HAL_FLASH_Unlock();
    lfs_port_clear_flash_flags();
    HAL_StatusTypeDef status = lfs_port_flash_program(address,
                                                      (const uint8_t *)buffer,
                                                      size);
    HAL_FLASH_Lock();

    if (status != HAL_OK) {
        return LFS_ERR_IO;
    }

    return LFS_ERR_OK;
}

static int lfs_port_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    (void)cfg;

    if (block >= LFS_PORT_FLASH_BLOCK_COUNT) {
        return LFS_ERR_CORRUPT;
    }

    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Banks = FLASH_BANK_1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3,
        .Sector = LFS_PORT_FIRST_SECTOR + block,
        .NbSectors = 1,
    };

    uint32_t sector_error = 0U;

    HAL_FLASH_Unlock();
    lfs_port_clear_flash_flags();
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &sector_error);
    HAL_FLASH_Lock();

    if ((status != HAL_OK) || (sector_error != 0xFFFFFFFFU)) {
        return LFS_ERR_IO;
    }

    return LFS_ERR_OK;
}

static int lfs_port_sync(const struct lfs_config *cfg)
{
    (void)cfg;
    return LFS_ERR_OK;
}

int lfs_port_format(lfs_t *lfs)
{
    return lfs_format(lfs, &lfs_port_cfg);
}

int lfs_port_mount(lfs_t *lfs)
{
    return lfs_mount(lfs, &lfs_port_cfg);
}

int lfs_port_unmount(lfs_t *lfs)
{
    return lfs_unmount(lfs);
}
```

</details>

<details>
<summary>component/littlefs-2.11.2/lfs_port.h</summary>

```c

/**
 * @file lfs_port.h
 * @brief STM32F401 flash-backed block device glue for littlefs.
 */

#ifndef LFS_PORT_H
#define LFS_PORT_H

#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LFS_PORT_FLASH_START_ADDR   (0x08040000UL)  /* Sector 6 base */
#define LFS_PORT_FLASH_END_ADDR     (0x08080000UL)  /* One past sector 7 */
#define LFS_PORT_FLASH_BLOCK_SIZE   (0x20000UL)     /* 128 KiB sectors */
#define LFS_PORT_FLASH_BLOCK_COUNT  (2U)

extern const struct lfs_config lfs_port_cfg;

int lfs_port_format(lfs_t *lfs);
int lfs_port_mount(lfs_t *lfs);
int lfs_port_unmount(lfs_t *lfs);

#ifdef __cplusplus
}
#endif

#endif /* LFS_PORT_H */
```

</details>

<!-- BSP_DRIVERS_END -->
