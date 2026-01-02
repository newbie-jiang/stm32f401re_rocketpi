<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:9356f88eaf17d8cf7b0128f89227f8aa1ae00ecb600c7b4d46860268c3ab45cf -->
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
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7789.h"
#include "test_240x240_RGB_BE.h"
#include "png_gradient_horizontal_240x240_RGB_BE_RGB565.h"
#include "jpg_checkerboard_240x240_RGB_BE_RGB565.h"
#include "font16x24_ascii.h"
#include "fonts.h"
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	extern const unsigned char rgb565_grid_240x240[] ;
	
  ST7789_Init();
	ST7789_Clear(WHITE);
  ST7789_DrawBitmap(0,0,240,240,(uint8_t *)jpg_checkerboard_240x240_RGB_BE_RGB565);
//  ST7789_DrawBitmap(0,0,240,240,(uint8_t *)rgb565_grid_240x240);
//  ST7789_DrawBitmap(0,0,240,240,(uint8_t *)test_240x240_RGB_BE_RGB565);
	
//  ST7789_TestFrameRate(); /* 帧率测试 */
	

//	ST7789_Marquee g_marquee = {0};
//	ST7789_MarqueeInit(&g_marquee,
//                       "rocket pi color test",
//                       &Font16x24,                 // 传指针
//                       120,
//                       YELLOW,
//                       BLUE,
//                       +2,                      // +2: 左->右；-2: 右->左
//                       25,
//                       25); 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		ST7789_MarqueeStep(&g_marquee, HAL_GetTick());
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
<summary>bsp/st7789/st7789.c</summary>

```c
#include "st7789.h"
#include <string.h>
#include <stdio.h>

/*========================= 用户可配置区域 =========================*/

/* 分辨率与旋转 */
#ifndef ST7789_WIDTH
#define ST7789_WIDTH   240
#endif
#ifndef ST7789_HEIGHT
#define ST7789_HEIGHT  240
#endif

#ifndef ST7789_HIGHT
#define ST7789_HIGHT   ST7789_HEIGHT
#endif

/* 旋转角：0/90/180/270 */
#ifndef ST7789_ROTATION
#define ST7789_ROTATION  0
#endif

/* 若屏幕 240x240 模组实为 320 控制窗（常见 Y 偏移 80），可选其一： */
#ifndef X_SHIFT
#define X_SHIFT  0
#endif
#ifndef Y_SHIFT
#define Y_SHIFT  0   /* 如遇图像下移或截断，可尝试改为 80 */
#endif

/* RGB/BGR 颜色顺序：某些面板需要 BGR（=1） */
#ifndef ST7789_BGR
#define ST7789_BGR  0  /* 0=RGB, 1=BGR */
#endif

/* 发送像素的缓冲区大小（字节，需为偶数） */
#ifndef ST7789_BUF_SIZE
#define ST7789_BUF_SIZE  (240 * 2) 
#endif

/*========================= 外部硬件依赖宏 =========================*/
/* 需在 st7789.h 或工程配置里定义：
   - SPI 句柄：ST7789_SPI（SPI_HandleTypeDef*）
   - 控制脚：ST7789_CS_LOW/HIGH(), ST7789_DC_LOW/HIGH(), ST7789_RST_LOW/HIGH(), ST7789_BL_LOW/HIGH()
*/
#ifndef ST7789_SPI
#warning "Please define ST7789_SPI (SPI_HandleTypeDef*)"
#endif

/*========================= 字体结构 =========================*/
//#ifndef __FONT_DEF__
//#define __FONT_DEF__
//typedef struct {
//    uint16_t width;    /* 字符宽（像素） */
//    uint16_t height;   /* 字符高（像素） */
//    const uint16_t *data; /* 字形数据：假设每行 16bit 位图，宽<=16 */
//} FontDef;
//#endif
/*========================= 本地缓冲与工具函数 =========================*/
static uint8_t ST7789_Buf[ST7789_BUF_SIZE];

/* 阻塞发送（命令/小数据使用） */
static inline void st7789_tx_blocking(const uint8_t *buf, size_t len) {
    HAL_SPI_Transmit(ST7789_SPI, (uint8_t*)buf, len, HAL_MAX_DELAY);
}

/* 命令写入 */
static inline void ST7789_WriteCmd(uint8_t cmd) {
    ST7789_CS_LOW();
    ST7789_DC_LOW();
    st7789_tx_blocking(&cmd, 1);
    ST7789_CS_HIGH();
}

/* 数据写入（小块，阻塞） */
static inline void ST7789_WriteData(const uint8_t *data, size_t len) {
    if (len == 0) return;
    ST7789_CS_LOW();
    ST7789_DC_HIGH();
    st7789_tx_blocking(data, len);
    ST7789_CS_HIGH();
}

/* 数据写入（大块，DMA + 轮询结束；若要更高效可改为回调+双缓冲） */
static inline void ST7789_WriteDataDMA(const uint8_t *data, size_t len) {
    if (len == 0) return;
    ST7789_CS_LOW();
    ST7789_DC_HIGH();
    HAL_SPI_Transmit_DMA(ST7789_SPI, (uint8_t*)data, len);
    while (HAL_SPI_GetState(ST7789_SPI) == HAL_SPI_STATE_BUSY_TX) { /* 轮询等待 */ }
    ST7789_CS_HIGH();
}

/* 设置窗口：一次性下发 CASET/RASET/RAMWR，内部处理旋转与偏移 */
static void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t xs, xe, ys, ye;

#if (ST7789_ROTATION == 0)
    xs = x0; xe = x1;
    ys = y0; ye = y1;
#elif (ST7789_ROTATION == 90)
    xs = y0;
    xe = y1;
    ys = (ST7789_WIDTH - 1 - x1);
    ye = (ST7789_WIDTH - 1 - x0);
#elif (ST7789_ROTATION == 180)
    xs = (ST7789_WIDTH  - 1 - x1);
    xe = (ST7789_WIDTH  - 1 - x0);
    ys = (ST7789_HEIGHT - 1 - y1);
    ye = (ST7789_HEIGHT - 1 - y0);
#elif (ST7789_ROTATION == 270)
    xs = (ST7789_HEIGHT - 1 - y1);
    xe = (ST7789_HEIGHT - 1 - y0);
    ys = x0;
    ye = x1;
#else
# error "Invalid ST7789_ROTATION"
#endif

    xs += X_SHIFT; xe += X_SHIFT;
    ys += Y_SHIFT; ye += Y_SHIFT;

    uint8_t caset[4] = { xs >> 8, xs & 0xFF, xe >> 8, xe & 0xFF };
    uint8_t raset[4] = { ys >> 8, ys & 0xFF, ye >> 8, ye & 0xFF };

    ST7789_CS_LOW();

    /* CASET */
    { uint8_t c = 0x2A;
      ST7789_DC_LOW();  st7789_tx_blocking(&c,1);
      ST7789_DC_HIGH(); st7789_tx_blocking(caset,4);
    }

    /* RASET */
    { uint8_t c = 0x2B;
      ST7789_DC_LOW();  st7789_tx_blocking(&c,1);
      ST7789_DC_HIGH(); st7789_tx_blocking(raset,4);
    }

    /* RAMWR */
    { uint8_t c = 0x2C;
      ST7789_DC_LOW();  st7789_tx_blocking(&c,1);
    }

    ST7789_CS_HIGH();
}

/*========================= 公共 API 实现 =========================*/

void ST7789_Init(void)
{
    /* 背光优先点亮（可按需） */
    ST7789_BL_HIGH();
    ST7789_CS_HIGH();

    /* 复位：建议 >=10ms 低电平，>=120ms 出睡眠后延时 */
    ST7789_RST_HIGH();
    HAL_Delay(1);
    ST7789_RST_LOW();
    HAL_Delay(10);
    ST7789_RST_HIGH();
    HAL_Delay(120);

    /* Sleep Out */
    ST7789_WriteCmd(0x11);
    HAL_Delay(120);

    /* 像素格式：16bpp RGB565 */
    ST7789_WriteCmd(0x3A);
    { uint8_t fmt = 0x55; ST7789_WriteData(&fmt,1); }

    /* Porch control */
    ST7789_WriteCmd(0xB2); {
        uint8_t d[] = {0x0C,0x0C,0x00,0x33,0x33};
        ST7789_WriteData(d, sizeof(d));
    }

    /* Gate control */
    ST7789_WriteCmd(0xB7); { uint8_t d=0x35; ST7789_WriteData(&d,1); }

    /* VCOM */
    ST7789_WriteCmd(0xBB); { uint8_t d=0x32; ST7789_WriteData(&d,1); } /* 1.35V */

    /* LCM control */
    ST7789_WriteCmd(0xC0); { uint8_t d=0x2C; ST7789_WriteData(&d,1); } /* 可选 */

    /* VDV/VRH */
    ST7789_WriteCmd(0xC2); { uint8_t d=0x01; ST7789_WriteData(&d,1); }
    ST7789_WriteCmd(0xC3); { uint8_t d=0x19; ST7789_WriteData(&d,1); } /* GVDD=4.8V */
    ST7789_WriteCmd(0xC4); { uint8_t d=0x20; ST7789_WriteData(&d,1); } /* VDV, 0x20:0V */

    /* Frame rate */
    ST7789_WriteCmd(0xC6); { uint8_t d=0x0F; ST7789_WriteData(&d,1); } /* 60Hz */

    /* Power control */
    ST7789_WriteCmd(0xD0); { uint8_t d[2]={0xA4,0xA1}; ST7789_WriteData(d,2); }

    /* 正伽马 */
    ST7789_WriteCmd(0xE0); {
        uint8_t d[] = {0xD0,0x08,0x0E,0x09,0x09,0x05,0x31,0x33,0x48,0x17,0x14,0x15,0x31,0x34};
        ST7789_WriteData(d,sizeof(d));
    }
    /* 负伽马 */
    ST7789_WriteCmd(0xE1); {
        uint8_t d[] = {0xD0,0x08,0x0E,0x09,0x09,0x15,0x31,0x33,0x48,0x17,0x14,0x15,0x31,0x34};
        ST7789_WriteData(d,sizeof(d));
    }

		
    /* 反色 */
    ST7789_WriteCmd(0x21); /* Normal display */

    /* MADCTL：方向 + RGB/BGR */
    {
        uint8_t madctl = 0x00;
    #if (ST7789_ROTATION == 0)
        madctl = 0x00;
    #elif (ST7789_ROTATION == 90)
        madctl = 0x60;
    #elif (ST7789_ROTATION == 180)
        madctl = 0xC0;
    #elif (ST7789_ROTATION == 270)
        madctl = 0xA0;
    #endif
    #if (ST7789_BGR == 1)
        madctl |= 0x08; /* BGR 位 */
    #endif
        ST7789_WriteCmd(0x36);
        ST7789_WriteData(&madctl,1);
    }

    /* 显示开 */
    ST7789_WriteCmd(0x29);
}

/* 清屏（整屏填充 color） */
void ST7789_Clear(uint16_t color)
{
    uint32_t total_bytes = (uint32_t)ST7789_WIDTH * ST7789_HEIGHT * 2;
    /* 预填充缓冲 */
    for (uint32_t i = 0; i < ST7789_BUF_SIZE; i += 2) {
        ST7789_Buf[i]   = color >> 8;
        ST7789_Buf[i+1] = color & 0xFF;
    }

    ST7789_SetWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    /* 启动 RAMWR 后，连续写数据 */
    uint8_t ramwr = 0x2C;
    ST7789_WriteCmd(ramwr);

    /* 按块写入 */
    uint32_t remain = total_bytes;
    while (remain) {
        uint32_t chunk = (remain > ST7789_BUF_SIZE) ? ST7789_BUF_SIZE : remain;
        ST7789_WriteDataDMA(ST7789_Buf, chunk);
        remain -= chunk;
    }
}

/* 画一个像素（不建议频繁调用，演示用） */
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    ST7789_SetWindow(x, y, x, y);
    uint8_t px[2] = { color >> 8, color & 0xFF };
    ST7789_WriteCmd(0x2C);
    ST7789_WriteData(px, 2);
}

/* 横线（一次性打包） */
void ST7789_DrawHLine(uint16_t xs, uint16_t xe, uint16_t y, uint16_t color)
{
    if (y >= ST7789_HEIGHT) return;
    if (xe < xs) { uint16_t t = xs; xs = xe; xe = t; }
    if (xs >= ST7789_WIDTH) return;
    if (xe >= ST7789_WIDTH) xe = ST7789_WIDTH - 1;

    uint16_t w = xe - xs + 1;
    uint32_t bytes = (uint32_t)w * 2;

    /* 准备一行颜色 */
    for (uint32_t i=0; i<bytes; i+=2) {
        ST7789_Buf[i]   = color >> 8;
        ST7789_Buf[i+1] = color & 0xFF;
    }

    ST7789_SetWindow(xs, y, xe, y);
    ST7789_WriteCmd(0x2C);
    ST7789_WriteDataDMA(ST7789_Buf, bytes);
}

/* 竖线（一次性打包后多段写） */
void ST7789_DrawVLine(uint16_t ys, uint16_t ye, uint16_t x, uint16_t color)
{
    if (x >= ST7789_WIDTH) return;
    if (ye < ys) { uint16_t t = ys; ys = ye; ye = t; }
    if (ys >= ST7789_HEIGHT) return;
    if (ye >= ST7789_HEIGHT) ye = ST7789_HEIGHT - 1;

    uint16_t h = ye - ys + 1;
    uint32_t bytes = (uint32_t)h * 2;

    /* 若一整段能装入缓冲，则一次写完，否则分块 */
    if (bytes <= ST7789_BUF_SIZE) {
        for (uint32_t i=0; i<bytes; i+=2) {
            ST7789_Buf[i]   = color >> 8;
            ST7789_Buf[i+1] = color & 0xFF;
        }
        ST7789_SetWindow(x, ys, x, ye);
        ST7789_WriteCmd(0x2C);
        ST7789_WriteDataDMA(ST7789_Buf, bytes);
    } else {
        ST7789_SetWindow(x, ys, x, ye);
        ST7789_WriteCmd(0x2C);
        /* 先填满一次缓冲 */
        for (uint32_t i=0; i<ST7789_BUF_SIZE; i+=2) {
            ST7789_Buf[i]   = color >> 8;
            ST7789_Buf[i+1] = color & 0xFF;
        }
        uint32_t remain = bytes;
        while (remain) {
            uint32_t chunk = (remain > ST7789_BUF_SIZE) ? ST7789_BUF_SIZE : remain;
            ST7789_WriteDataDMA(ST7789_Buf, chunk);
            remain -= chunk;
        }
    }
}

/* 填充矩形（尽量用整块 DMA） */
void ST7789_FillRect(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    if (xe < xs) { uint16_t t=xs; xs=xe; xe=t; }
    if (ye < ys) { uint16_t t=ys; ys=ye; ye=t; }
    if (xs >= ST7789_WIDTH || ys >= ST7789_HEIGHT) return;
    if (xe >= ST7789_WIDTH)  xe = ST7789_WIDTH - 1;
    if (ye >= ST7789_HEIGHT) ye = ST7789_HEIGHT - 1;

    uint16_t w = xe - xs + 1;
    uint16_t h = ye - ys + 1;
    uint32_t total = (uint32_t)w * h * 2;

    /* 预填充缓冲 */
    for (uint32_t i=0; i<ST7789_BUF_SIZE; i+=2) {
        ST7789_Buf[i]   = color >> 8;
        ST7789_Buf[i+1] = color & 0xFF;
    }

    ST7789_SetWindow(xs, ys, xe, ye);
    ST7789_WriteCmd(0x2C);

    /* 按块写入 */
    uint32_t remain = total;
    while (remain) {
        uint32_t chunk = (remain > ST7789_BUF_SIZE) ? ST7789_BUF_SIZE : remain;
        ST7789_WriteDataDMA(ST7789_Buf, chunk);
        remain -= chunk;
    }
}

/**
 * 画一行 16BPP 位图（RGB565，小端存储：p[0]=low, p[1]=high）
 * xsize ∈ [1, ST7789_WIDTH]
 */
void ST7789_DrawBitLine16BPP(uint16_t xs, uint16_t y, const uint8_t *p, uint16_t xsize)
{
    if (y >= ST7789_HEIGHT || xs >= ST7789_WIDTH) return;
    if (xsize == 0) return;
    if (xs + xsize > ST7789_WIDTH) xsize = ST7789_WIDTH - xs;

    uint32_t bytes = (uint32_t)xsize * 2;

    /* 小端 -> 高字节在前（面板需要高字节先出） */
    for (uint32_t i = 0; i < bytes; i += 2) {
        uint8_t lo = p[i];
        uint8_t hi = p[i+1];
        ST7789_Buf[i]   = hi;
        ST7789_Buf[i+1] = lo;
    }

    ST7789_SetWindow(xs, y, xs + xsize - 1, y);
    ST7789_WriteCmd(0x2C);
    ST7789_WriteDataDMA(ST7789_Buf, bytes);
}

/* 画任意矩形位图：逐行送（p 为 RGB565 小端） */
void ST7789_DrawBitmap(uint16_t xs, uint16_t ys, uint16_t xsize, uint16_t ysize, const uint8_t *p)
{
    if (xs >= ST7789_WIDTH || ys >= ST7789_HEIGHT) return;
    if (xsize == 0 || ysize == 0) return;

    uint16_t xmax = (xs + xsize > ST7789_WIDTH)  ? (ST7789_WIDTH  - xs) : xsize;
    uint16_t ymax = (ys + ysize > ST7789_HEIGHT) ? (ST7789_HEIGHT - ys) : ysize;

    for (uint16_t i=0; i<ymax; i++) {
        ST7789_DrawBitLine16BPP(xs, ys + i, p + (uint32_t)i * xsize * 2, xmax);
    }
}

void ST7789_ShowChar(uint16_t x, uint16_t y, uint8_t ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    if (font.width == 0 || font.height == 0) return;

    /* 使用局部变量承接，避免修改 font 本体 */
    uint16_t w = font.width;
    uint16_t h = font.height;

    /* 这版实现假设“每行最多 16 位位图”，超出则按 16 处理 */
    if (w > 16) w = 16;

    /* 越界裁剪 */
    if (x + w - 1 >= ST7789_WIDTH || y + h - 1 >= ST7789_HEIGHT) return;

    ST7789_SetWindow(x, y, x + w - 1, y + h - 1);
    ST7789_WriteCmd(0x2C);

    /* 把字符整块写入，按行拼进缓冲区，满了就发一块 */
    uint32_t wrote = 0;
    for (uint16_t row = 0; row < h; row++) {
        /* 注意：如果你的 fonts.h 定义不是每行 16bit，需要按你的格式改这里取位方式 */
        uint16_t bits = font.data[((uint32_t)(ch - 32) * h) + row];
        for (uint16_t col = 0; col < w; col++) {
            uint16_t c = (bits & (1U << (w - 1 - col))) ? color : bgcolor;

            /* 将像素写入发送缓冲（双字节：高位在前） */
            uint32_t idx = wrote % ST7789_BUF_SIZE;
            ST7789_Buf[idx + 0] = (uint8_t)(c >> 8);
            ST7789_Buf[idx + 1] = (uint8_t)(c & 0xFF);
            wrote += 2;

            if ((wrote % ST7789_BUF_SIZE) == 0) {
                ST7789_WriteDataDMA(ST7789_Buf, ST7789_BUF_SIZE);
            }
        }
    }

    /* 发送尾块 */
    uint32_t remain = wrote % ST7789_BUF_SIZE;
    if (remain) {
        ST7789_WriteDataDMA(ST7789_Buf, remain);
    }
}

void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    if (!str) return;
    /* 同样用局部变量保存宽高 */
    uint16_t fw = font.width;
    uint16_t fh = font.height;
    if (fw == 0 || fh == 0) return;
    if (fw > 16) fw = 16;

    uint16_t cx = x, cy = y;
    while (*str) {
        /* 自动换行 */
        if (cx + fw > ST7789_WIDTH) {
            cx = 0;
            cy += fh;
            if (cy + fh > ST7789_HEIGHT) break;
            if (*str == ' ') { str++; continue; }
        }

        ST7789_ShowChar(cx, cy, (uint8_t)*str, font, color, bgcolor);
        cx += fw;
        str++;
    }
}


/* 帧率测试：60 帧整屏填充不同色 */
void ST7789_TestFrameRate(void)
{
    static const uint16_t test_colors[] = { 0xF800, /*RED*/ 0x07E0, /*GREEN*/ 0x001F, /*BLUE*/
                                            0xFFFF, /*WHITE*/ 0x0000, /*BLACK*/ 0x07FF, /*CYAN*/
                                            0xF81F, /*MAGENTA*/ 0xFFE0 /*YELLOW*/ };
    const uint32_t frame_count = 60U;
    uint32_t tick_start = 0, elapsed = 0;

    /* 清屏 */
    ST7789_Clear(0x0000);

    tick_start = HAL_GetTick();
    for (uint32_t f=0; f<frame_count; f++) {
        uint16_t color = test_colors[f % (sizeof(test_colors)/sizeof(test_colors[0]))];
        ST7789_FillRect(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1, color);
    }
    elapsed = HAL_GetTick() - tick_start;

    double fps = (elapsed > 0) ? ((double)frame_count * 1000.0) / (double)elapsed : 0.0;
    printf("ST7789 frame test: %lu frames in %lums (%.2f FPS)\r\n",
           (unsigned long)frame_count, (unsigned long)elapsed, fps);
}



/* 计算字符串像素宽度（不考虑换行） */
static uint16_t ST7789_TextPixelWidth(const char *s, const FontDef *font)
{
    if (!s || !font) return 0;
    uint32_t n = 0;
    while (*s++) n++;
    return (uint16_t)(n * font->width);
}




/* 像素级：在一行缓冲里合成当前帧，再一次性送出（极顺滑） */
static void ST7789_DrawVisibleText_SmoothRow(int16_t x, uint16_t y,
                                             const char *s,
                                             const FontDef *font,
                                             uint16_t color, uint16_t bgcolor)
{
    if (!s || !font) return;
    const int16_t fw = (int16_t)font->width;
    const int16_t fh = (int16_t)font->height;
    if (y >= ST7789_HEIGHT || (int32_t)y + fh - 1 > (int32_t)ST7789_HEIGHT - 1) return;

    /* 对每一行做合成，然后整行发出去 */
    for (int16_t row = 0; row < fh; row++) {

        /* 1) 先把整行清成背景色（高字节在前） */
        for (uint16_t xi = 0; xi < ST7789_WIDTH; xi++) {
            uint32_t idx = (uint32_t)xi * 2;
            ST7789_Buf[idx + 0] = (uint8_t)(bgcolor >> 8);
            ST7789_Buf[idx + 1] = (uint8_t)(bgcolor & 0xFF);
        }

        /* 2) 把这一行的文字逐“像素”覆写到行缓冲上 */
        int16_t cx = x;                 /* 本字符左上角 X（像素，可为负） */
        const char *p = s;
        while (*p) {
            /* 字形在屏幕上的 X 范围 */
            int16_t gx0 = cx;           /* 包含当前字符的首像素 */
            int16_t gx1 = cx + fw - 1;  /* 包含当前字符的末像素 */

            if (gx1 < 0) {              /* 这个字完全在屏幕左侧之外 */
                cx += fw;                /* 跳到下一个字 */
                p++;
                continue;
            }
            if (gx0 >= (int16_t)ST7789_WIDTH) {
                break;                   /* 右侧之外，后面都不用画了 */
            }

            /* 取该字符此行的位图（每行16bit，MSB是最左像素） */
            uint16_t bits = font->data[((uint32_t)((uint8_t)(*p) - 32) * fh) + row];

            /* 可见区域裁剪到屏幕 */
            int16_t vis0 = (gx0 < 0) ? 0 : gx0;
            int16_t vis1 = (gx1 >= (int16_t)ST7789_WIDTH) ? ((int16_t)ST7789_WIDTH - 1) : gx1;

            /* 把可见列逐像素画进行缓冲 */
            for (int16_t sx = vis0; sx <= vis1; sx++) {
                /* 对应到字形内部的列编号（0..fw-1） */
                int16_t col = sx - cx;              /* col 可能是 0..fw-1 的任意值 */
                /* 测这一位是否为 1（前景像素） */
                if (bits & (1U << (fw - 1 - col))) {
                    uint32_t idx = (uint32_t)sx * 2;
                    ST7789_Buf[idx + 0] = (uint8_t)(color >> 8);
                    ST7789_Buf[idx + 1] = (uint8_t)(color & 0xFF);
                }
            }

            cx += fw;
            p++;
        }

        /* 3) 发送这一行到 (0, y+row) .. (WIDTH-1, y+row) */
        ST7789_SetWindow(0, (uint16_t)(y + row), ST7789_WIDTH - 1, (uint16_t)(y + row));
        ST7789_WriteCmd(0x2C);
        ST7789_WriteDataDMA(ST7789_Buf, (uint32_t)ST7789_WIDTH * 2);
    }
}



/* 初始化（注意 font 传指针） */
void ST7789_MarqueeInit(ST7789_Marquee *m,
                        const char *text,
                        const FontDef *font,
                        uint16_t y,
                        uint16_t color,
                        uint16_t bgcolor,
                        int16_t speed_px,
                        uint16_t gap_px,
                        uint32_t interval_ms)
{
    if (!m) return;
    m->text   = text;
    m->font   = font;
    m->y      = y;
    m->color  = color;
    m->bgcolor= bgcolor;
    m->speed_px = (speed_px == 0) ? 1 : speed_px;           // 避免 0
    m->gap_px   = gap_px;
    m->interval_ms = (interval_ms == 0) ? 16 : interval_ms;
    m->last_ms  = 0;

    int16_t tw = (int16_t)ST7789_TextPixelWidth(text, font);
    // 左->右：从左侧外开始；右->左：从右侧外开始
    m->x = (m->speed_px > 0) ? (int16_t)(-tw) : (int16_t)(ST7789_WIDTH + gap_px);
}

/* 非阻塞步进：按时间刷新、只重绘文字带 */
void ST7789_MarqueeStep(ST7789_Marquee *m, uint32_t now_ms)
{
    if (!m || !m->text || !m->font) return;

    if (m->last_ms && (uint32_t)(now_ms - m->last_ms) < m->interval_ms) return;
    m->last_ms = now_ms;

    uint16_t band_y0 = m->y;
    uint16_t band_y1 = (uint16_t)(m->y + m->font->height - 1);
    if (band_y1 >= ST7789_HEIGHT) band_y1 = ST7789_HEIGHT - 1;

    /* 1) 擦除这一行带 */
    ST7789_FillRect(0, band_y0, ST7789_WIDTH - 1, band_y1, m->bgcolor);

    /* 2) 绘制当前帧 */
	  ST7789_DrawVisibleText_SmoothRow(m->x, m->y, m->text, m->font, m->color, m->bgcolor);

    /* 3) 更新位置 */
    m->x += m->speed_px;

    /* 4) 回绕逻辑 */
    int16_t tw = (int16_t)ST7789_TextPixelWidth(m->text, m->font);
    if (m->speed_px > 0) {
        if (m->x > (int16_t)ST7789_WIDTH) {
            m->x = (int16_t)(-tw - (int16_t)m->gap_px);
        }
    } else { // 右->左
        if ((m->x + tw) < 0) {
            m->x = (int16_t)(ST7789_WIDTH + m->gap_px);
        }
    }
}

/* 阻塞演示（方便快速验证） */
void ST7789_MarqueeRunBlocking(const char *text,
                               const FontDef *font,
                               uint16_t y,
                               uint16_t color,
                               uint16_t bgcolor,
                               int16_t speed_px,
                               uint16_t gap_px,
                               uint32_t interval_ms,
                               uint32_t duration_ms)
{
    ST7789_Marquee m;
    ST7789_MarqueeInit(&m, text, font, y, color, bgcolor, speed_px, gap_px, interval_ms);

    uint32_t t0 = HAL_GetTick();
    while ((uint32_t)(HAL_GetTick() - t0) < duration_ms) {
        ST7789_MarqueeStep(&m, HAL_GetTick());
        HAL_Delay(1);
    }
}
```

</details>

<details>
<summary>bsp/st7789/st7789.h</summary>

```c
#ifndef ST7789_H
#define ST7789_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "fonts.h"     /* 需要提供 FontDef 定义；若与你现有不一致，可告诉我改配 */

/*================= 用户配置（与实际屏幕/连线匹配） =================*/

/* 分辨率 */
#ifndef ST7789_WIDTH
#define ST7789_WIDTH    240
#endif
#ifndef ST7789_HEIGHT
#define ST7789_HEIGHT   240
#endif

/* 兼容你之前代码使用的 HIGHT 宏名 */
#ifndef ST7789_HIGHT
#define ST7789_HIGHT    ST7789_HEIGHT
#endif

/* 旋转角：0/90/180/270 */
#ifndef ST7789_ROTATION
#define ST7789_ROTATION 0
#endif

/* 若你的 240x240 模组实际 IC 窗口为 240x320，需要 Y 方向偏移 80（常见）。
   如发现图像上下移/被裁剪，尝试把 Y_SHIFT 改为 80。 */
#ifndef X_SHIFT
#define X_SHIFT 0
#endif
#ifndef Y_SHIFT
#define Y_SHIFT 0 /* 常见：0 或 80 */
#endif

/* 颜色顺序：有的屏需要 BGR。0=RGB，1=BGR */
#ifndef ST7789_BGR
#define ST7789_BGR 0
#endif

/* SPI 句柄（沿用你的写法） */
#ifndef ST7789_SPI
#define ST7789_SPI (&hspi1)
#endif

/* 发送像素缓冲大小（字节，偶数）。越大整屏填充越快（受 RAM 限制）。 */
#ifndef ST7789_BUF_SIZE
#define ST7789_BUF_SIZE (240 * 2)
#endif

/* 控制引脚（沿用你的宏） */
#define ST7789_RST_LOW()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define ST7789_RST_HIGH()  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)

#define ST7789_CS_LOW()    HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_RESET)
#define ST7789_CS_HIGH()   HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_SET)

#define ST7789_DC_LOW()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_RESET)
#define ST7789_DC_HIGH()   HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_SET)

#define ST7789_BL_LOW()    HAL_GPIO_WritePin(LCD_BL_GPIO_Port,  LCD_BL_Pin,  GPIO_PIN_RESET)
#define ST7789_BL_HIGH()   HAL_GPIO_WritePin(LCD_BL_GPIO_Port,  LCD_BL_Pin,  GPIO_PIN_SET)

/* 颜色常量（沿用你的） */
#define WHITE     0xFFFF
#define BLACK     0x0000
#define BLUE      0x001F
#define BRED      0xF81F
#define GRED      0xFFE0
#define GBLUE     0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define GREEN     0x07E0
#define CYAN      0x7FFF
#define YELLOW    0xFFE0
#define BROWN     0xBC40
#define BRRED     0xFC07
#define GRAY      0x8430


typedef struct {
    const char *text;         // 要滚动的字符串
    const FontDef *font;      // 指向字体（允许是 const FontDef）
    uint16_t y;               // 顶部 Y 坐标
    uint16_t color;           // 字颜色
    uint16_t bgcolor;         // 背景色（用于擦除带）
    int16_t  x;               // 当前起点 X（像素，可为负）
    int16_t  speed_px;        // 每步像素（>0 左->右；<0 右->左）
    uint16_t gap_px;          // 循环间距
    uint32_t interval_ms;     // 帧间隔
    uint32_t last_ms;         // 内部：上次刷新时间戳
} ST7789_Marquee;


/**
 * @brief 跑马灯初始化（非阻塞版本使用）。
 *
 * 将一段字符串设置为在指定 Y 行进行水平滚动显示。配合 ST7789_MarqueeStep()
 * 周期调用实现动画。支持像素级平滑滚动：当文本尚未完全进入屏幕时，
 * 也会绘制可见的“半字符”像素。
 *
 * @param m            跑马灯控制结构体指针（由调用方提供保存状态）。
 * @param text         要滚动的 C 字符串（生命周期需覆盖整个显示期间）。
 * @param font         字体指针（与现有 FontDef 格式一致：每字符 24 行×16 位等）。
 * @param y            文本左上角 Y 坐标（0..HEIGHT-font->height），单行绘制，不换行。
 * @param color        文本前景色（RGB565）。
 * @param bgcolor      背景色（RGB565），用于擦除该文字带（只重绘这条带，避免闪烁）。
 * @param speed_px     每帧移动像素：>0 表示“左→右”，<0 表示“右→左”，=0 将被归一为 1。
 * @param gap_px       每轮滚动结束后重新从屏外进入前的额外留白（像素），用于视觉节奏。
 * @param interval_ms  帧间隔（毫秒），例如 16≈60FPS，33≈30FPS；=0 将默认 16ms。
 *
 * @note  本实现是单行合成+DMA 推送，刷新开销低；建议在主循环/定时器中调用 Step。
 * @note  若需要更改方向，直接传入 speed_px 的符号即可（不需要重新 init）。
 */
void ST7789_MarqueeInit(ST7789_Marquee *m,
                        const char *text,
                        const FontDef *font,
                        uint16_t y,
                        uint16_t color,
                        uint16_t bgcolor,
                        int16_t speed_px,
                        uint16_t gap_px,
                        uint32_t interval_ms);

/**
 * @brief 跑马灯步进（非阻塞）：到时才刷新一帧。
 *
 * 依据 ST7789_MarqueeInit() 设置的参数与上次刷新的时间戳，判断是否需要
 * 刷新；若到时则仅重绘目标文字带的一行行像素（像素级平滑），并推进 x 位置。
 *
 * @param m       跑马灯控制结构体指针。
 * @param now_ms  当前毫秒计数（通常传 HAL_GetTick()）。
 *
 * @note  内部会维护 last_ms，以 interval_ms 为周期刷新；未到时间直接返回。
 * @note  若 speed_px > 0：文本从屏幕左外侧进入，穿过屏幕，越过右边界后从左外侧
 *        重新进入；若 speed_px < 0：方向相反（右入左出再右入）。
 * @note  仅重绘文字所占的 y..y+font->height-1 区域，其余画面不受影响。
 */
void ST7789_MarqueeStep(ST7789_Marquee *m, uint32_t now_ms);

/**
 * @brief 跑马灯阻塞演示：在指定时长内循环播放（便于快速验证效果）。
 *
 * 该函数内部会创建一个临时 ST7789_Marquee，调用 Step 并延时，直到达到
 * 指定 duration_ms。适合作为 Demo/自检；在正式工程中建议使用非阻塞
 * 的 Init + Step 方案。
 *
 * @param text         要滚动的 C 字符串。
 * @param font         字体指针（与 FontDef 格式一致）。
 * @param y            文本左上角 Y 坐标（单行，不换行）。
 * @param color        文本前景色（RGB565）。
 * @param bgcolor      背景色（RGB565），用于擦除该文字带。
 * @param speed_px     每帧移动像素：>0 左→右；<0 右→左；=0 将归一为 1。
 * @param gap_px       每轮滚动之间的额外留白（像素）。
 * @param interval_ms  帧间隔（毫秒），如 16≈60FPS。
 * @param duration_ms  播放总时长（毫秒）。
 *
 * @note  阻塞式实现：内部 while 循环持续到 duration_ms 结束。适合上电演示、
 *        单步调试；不适合作为正式 UI 的主循环方式。
 */
void ST7789_MarqueeRunBlocking(const char *text,
                               const FontDef *font,
                               uint16_t y,
                               uint16_t color,
                               uint16_t bgcolor,
                               int16_t speed_px,
                               uint16_t gap_px,
                               uint32_t interval_ms,
                               uint32_t duration_ms);


/*================= 对外 API =================*/

void ST7789_Init(void);
void ST7789_Clear(uint16_t color);

void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7789_DrawHLine(uint16_t xs, uint16_t xe, uint16_t y, uint16_t color);
void ST7789_DrawVLine(uint16_t ys, uint16_t ye, uint16_t x, uint16_t color);
void ST7789_FillRect(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);

/* 位图（RGB565 小端：低字节在前） */
void ST7789_DrawBitLine16BPP(uint16_t xs, uint16_t y, const uint8_t *p, uint16_t xsize);
void ST7789_DrawBitmap(uint16_t xs, uint16_t ys, uint16_t xsize, uint16_t ysize, const uint8_t *p);

/* 文本（FontDef 要与你的 fonts.h 匹配；若每行不是 16bit 位图，告诉我改位取法） */
void ST7789_ShowChar(uint16_t x, uint16_t y, uint8_t ch, FontDef font, uint16_t color, uint16_t bgcolor);
void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);

/* 简单帧率测试 */
void ST7789_TestFrameRate(void);

/*================= 可选：保留旧类型（如有外部引用就留，否则可删） =================*/
typedef enum {
    ST7789_CMD,
    ST7789_DATA,
} ST7789_DCType;

/* 若你的外部代码还在用 GUI_BITMAP，可保留；否则可删除 */
typedef struct {
    uint16_t XSize;
    uint16_t YSize;
    uint16_t BytesPerLine;
    uint16_t BitsPerPixel;
    const uint8_t *pData;
} GUI_BITMAP;

#ifdef __cplusplus
}
#endif
#endif /* ST7789_H */
```

</details>

<!-- BSP_DRIVERS_END -->
