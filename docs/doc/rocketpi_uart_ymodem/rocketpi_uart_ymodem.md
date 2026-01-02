# YMODEM 收发说明

本工程在 `component/ymodem` 目录下提供了通用的 YMODEM 协议实现（纯协议逻辑在 `ymodem.c/.h`，平台相关的串口/存储适配放在 `ymodem_port.c`）。

## 功能概述
- **接收**：通过 `ymodem_recv(NULL)` 将上位机发送的文件写入片内 Flash 扇区 6、7（地址 0x08040000-0x0807FFFF）。可以使用 PC 端任意 YMODEM 工具验证。
- **发送**：`ymodem_send()` 会发送固件内置的 `rocketpi_demo.txt` 文本内容，上位机可直接保存并打开。

## 使用方式
1. 在 `Core/Src/main.c` 中通过宏开关选择模式：
   ```c
   // 定义则为发送模式，不定义则进入接收模式
   #define YMODEM_MODE_SEND
   ```
   - 发送模式：复位后自动500 ms，随后调用 `ymodem_send()`，完成后输出返回码。
   - 接收模式：复位后等待500 ms，调用 `ymodem_recv(NULL)`，Flash 内将依次写入收到的文件。
2. 编译并点火，再使用上位机串口工具进行 YMODEM 收发验证。

## 目录结构
- `component/ymodem/ymodem.c/.h`：协议核心，纯平台无关代码。
- `component/ymodem/ymodem_port.c`：STM32 平台适配，包含串口操作、Flash 存储、虚拟发送文件等。
- `Core/Src/main.c`：示例入口，通过宏开关调用发送或接收函数。

## 注意事项
- Flash 扇区抹除与写入在接收前自动完成，请确保其他代码未占用 0x08040000-0x0807FFFF 区域。
- 发送/接收只能单方向工作，YMODEM 协议不支持同时双向；若需动态切换，可在应用层添加命令后再调用对应函数。
- 需要将 `component/ymodem/ymodem_port.c` 加入工程编译。

如需移植到其他平台，只需替换 `ymodem_port.c` 中的串口、计时器、存储回调即可。

<!-- BSP_DRIVERS_START -->
<!-- BSP_DRIVERS_HASH:c53eb162a3ffff9d60f2faad149c775fda69a9a62cd82c0a7b1e74e9042b4f7b -->
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
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "ymodem.h"
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
/* USER CODE BEGIN 2 */

#define YMODEM_MODE_SEND

#if defined(YMODEM_MODE_SEND)
  printf("YMODEM transmit mode\r\n");
  HAL_Delay(200);
  const char* files[] = { "rocketpi_demo.txt" };
  int tx_rc = ymodem_send(files, 1);
  printf("[YMODEM] send rc=%d\r\n", tx_rc);
#else
  printf("YMODEM receive mode\r\n");
  HAL_Delay(200);
  ymodem_recv(NULL);
#endif
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
<summary>component/ymodem/ymodem.c</summary>

```c
/* ymodem_stm32_flash.c  --  STM32 HAL UART + Flash 存储版本
 *
 * 依赖:
 *   - STM32 HAL: UART 已初始化（例如 huart2）
 *   - 片内 Flash: 扇区 6/7 用作缓存
 *
 * 提供接口:
 *   void ymodem_recv(const char* out_dir);               // 接收文件写入 Flash
 *   int  ymodem_send(const char* const* files, int n);   // 发送文件列表
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "ymodem.h"

static int64_t g_store_file_size_hint = -1;

void ymodem_store_set_file_size_hint(int64_t sz){
    g_store_file_size_hint = sz;
}

int64_t ymodem_store_get_file_size_hint(void){
    return g_store_file_size_hint;
}

// ================= CRC / Utils =================
static uint16_t crc16_ccitt(const uint8_t* q, int len) {
    uint16_t crc = 0;
    while (len-- > 0) {
        crc ^= (uint16_t)(*q++) << 8;
        for (int i=0;i<8;i++)
            crc = (crc & 0x8000) ? (crc<<1) ^ 0x1021 : (crc<<1);
    }
    return crc;
}

static inline void ylog(const YContext* ctx, const char* s){
    if (ctx->hooks.on_log) ctx->hooks.on_log(s);
}
static inline void yprog(const YContext* ctx, const char* tag, const char* name, uint64_t done, int64_t total){
    if (ctx->hooks.on_progress) ctx->hooks.on_progress(tag, name, done, total);
}

static int y_putc (const YContext* c, uint8_t b){ return c->port->putc(c->port, b); }
static int y_getc (const YContext* c, uint8_t* b, int ms){ return c->port->getc(c->port, b, ms); }
static int y_write(const YContext* c, const void* p, int n){ return c->port->write(c->port, p, n); }
static int y_readx(const YContext* c, void* p, int n, int ms){ return c->port->read_exact(c->port, p, n, ms); }

/* 立刻把串口里可能残留的 'C' / ACK 等读干净（不阻塞） */
static void drain_input(YContext* ctx){
    uint8_t ch;
    while (y_getc(ctx, &ch, 1) == 1) { /* 1ms 轮询直到没有可读数据 */ }
}

// ================= 进度打印（可选） =================
static void human_bytes(double v, char* out, size_t n){
    const char* u[]={"B","KB","MB","GB"}; int i=0;
    while (v>=1024.0 && i<3){ v/=1024.0; i++; }
    snprintf(out, n, (v>=100)? "%.0f %s" : (v>=10? "%.1f %s":"%.2f %s"), v, u[i]);
}
typedef struct {
    const char* prefix; const char* name;
    long long total; long long done;
    uint32_t t0_ms; uint32_t last_ms;
    size_t last_print_len;
    const YTimer* timer;
} Prog;

static void prog_init(Prog* p, const YTimer* timer, const char* prefix, const char* name, long long total){
    p->prefix=prefix; p->name=name; p->total=total; p->done=0;
    p->timer=timer;
    if (timer && timer->now_ms){
        uint32_t now = timer->now_ms();
        p->t0_ms=now; p->last_ms=now;
    } else {
        p->t0_ms=0; p->last_ms=0;
    }
    p->last_print_len=0;
}
static void prog_tick(Prog* p, bool force){
    if (!p->timer || !p->timer->now_ms) return;
    uint32_t t=p->timer->now_ms();
    if(!force && (t-p->last_ms)<200) return;

    if (p->total > 0 && p->done == 0) {
        if (!force) return;
        return;
    }

    p->last_ms=t;
    double elapsed=(t-p->t0_ms)/1000.0; if(elapsed<=0) elapsed=0.001;
    double sp=(p->done>0)? (p->done/elapsed) : 0.0; /* B/s */
    char done_s[32], sp_s[32], line[256]; human_bytes((double)p->done,done_s,sizeof(done_s)); human_bytes(sp,sp_s,sizeof(sp_s));

    if (p->total>0){
        long long remain=p->total-p->done; if(remain<0) remain=0;
        int pct=(int)((p->done*100.0)/p->total+0.5);
        char tot_s[32]; human_bytes((double)p->total,tot_s,sizeof(tot_s));
        if (remain>0){
            snprintf(line,sizeof(line),"\r%s%s  %s / %s  %3d%%  avg %s/s  ETA %.0fs",
                      p->prefix,p->name,done_s,tot_s,pct,sp_s,(sp>0)?(remain/sp):0.0);
        } else {
            snprintf(line,sizeof(line),"\r%s%s  %s / %s  %3d%%  avg %s/s",
                      p->prefix,p->name,done_s,tot_s,pct,sp_s);
        }
    } else {
        snprintf(line,sizeof(line),"\r%s%s  %s  avg %s/s",p->prefix,p->name,done_s,sp_s);
    }
    /* MCU 上可按需输出 */
    /* printf("%s", line); */
}
static void prog_done_line(Prog* p){ (void)p; /* printf("\n"); */ }

// ================= 发送端核心 =================
static int send_block(const YContext* c, uint8_t seq, const uint8_t* payload, int plen){
    uint8_t code=(plen==YMD_BLK1K)? YMD_STX : YMD_SOH;
    uint8_t hdr[3]={ code, seq, (uint8_t)(~seq) };
    uint16_t crc=crc16_ccitt(payload,plen);
    uint8_t crc2[2]={ (uint8_t)(crc>>8), (uint8_t)(crc&0xFF) };

    for(int r=0;r<c->cfg.retry_max;++r){
        if (y_write(c,hdr,3)!=3) return -1;
        if (y_write(c,payload,plen)!=plen) return -2;
        if (y_write(c,crc2,2)!=2) return -3;
        uint8_t ch;
        if (y_getc(c,&ch,c->cfg.rx_timeout_ms)==1){
            if (ch==YMD_ACK) return 0;
            if (ch==YMD_CAN) return -10; /* peer canceled */
            /* else NAK/garbage -> retry */
        }
    }
    return -4; /* exceed retry */
}
static int send_block0(const YContext* c, const char* filename, int64_t fsz){
    uint8_t buf[YMD_BLK128]={0}; int p=0;
    if (filename && *filename){
        int n=(int)strlen(filename); if(n>YMD_BLK128-2) n=YMD_BLK128-2;
        memcpy(buf+p,filename,n); p+=n; buf[p++]=0;
        char sz[32]; if (fsz<0) fsz=0; snprintf(sz,sizeof(sz),"%lld",(long long)fsz);
        int m=(int)strlen(sz); if(p+m+1>YMD_BLK128) m=YMD_BLK128-p-1;
        memcpy(buf+p,sz,m); p+=m; buf[p++]=0;
    } else {
        buf[0]=0; /* empty name => end */
    }
    return send_block(c,0x00,buf,YMD_BLK128);
}

int ymd_send_multi(YContext* ctx, const char* const* files, int nfiles){
    if (!ctx || !ctx->port || !ctx->timer || !ctx->store) return -100;

    /* wait for 'C' */
    uint8_t ch=0; uint32_t t0=ctx->timer->now_ms();
    while ((int)(ctx->timer->now_ms()-t0) < ctx->cfg.hs_total_ms){
        if (y_getc(ctx,&ch,1000)==1 && ch==YMD_CHC) break;
    }
    if (ch!=YMD_CHC){ ylog(ctx,"[TX] no 'C' from receiver"); return -101; }

    uint8_t* blk=(uint8_t*)malloc(YMD_BLK1K); if(!blk) return -102;

    for(int i=0;i<nfiles;i++){
        const char* path=files[i];
        void* f=ctx->store->open_read(path);

        if(!f){
            char msg[128];
            snprintf(msg, sizeof(msg), "[TX] open_read fail: %s", path);
            ylog(ctx, msg);
            free(blk);
            return -103;
        }

        int64_t fsz=-1;
        if (ctx->store->size) fsz=ctx->store->size(f);
        if (fsz<0 && ctx->store->seek && ctx->store->tell){
            ctx->store->seek(f,0,2); fsz=ctx->store->tell(f); ctx->store->seek(f,0,0);
        }

        /* basename */
        const char* base=path; for(const char* p=path;*p;++p){ if(*p=='/'||*p=='\\') base=p+1; }

        if (send_block0(ctx,base,fsz)!=0){ ylog(ctx,"[TX] send block0 fail"); ctx->store->close_read(f); free(blk); return -104; }

        /* swallow any extra 'C'/ACK without waiting */
        drain_input(ctx);

        Prog P; prog_init(&P, ctx->timer,"[TX] ",base,fsz);
        uint8_t seq=1; int plen= ctx->cfg.packet_prefer_1k? YMD_BLK1K:YMD_BLK128;
        uint64_t done=0;

        for(;;){
            int r=ctx->store->read(f,blk,plen);
            if (r<0){ ylog(ctx,"[TX] read error"); ctx->store->close_read(f); free(blk); return -105; }
            if (r==0) break;
            if (r<plen) memset(blk+r,0x1A,plen-r);
            int rc=send_block(ctx,seq,blk,plen);
            if (rc!=0){ ylog(ctx,"[TX] data block fail"); ctx->store->close_read(f); free(blk); return -106; }
            seq++; done+=(uint32_t)r; if((int64_t)done>fsz && fsz>=0) done=(uint64_t)fsz;
            yprog(ctx,"[TX]",base,done,fsz);
            P.done = (long long)done;
            prog_tick(&P,false);
        }

        /* --- tolerant EOT sequence --- */
        y_putc(ctx,YMD_EOT);
        /* wait NAK (ignore others) */
        {
            uint32_t t1=ctx->timer->now_ms(); int got=0;
            while((int)(ctx->timer->now_ms()-t1) < ctx->cfg.rx_timeout_ms){
                if (y_getc(ctx,&ch,200)==1){
                    if (ch==YMD_NAK){ got=1; break; }
                    if (ch==YMD_CAN){ ylog(ctx,"[TX] canceled during EOT"); ctx->store->close_read(f); free(blk); return -107; }
                }
            }
            if(!got){ ylog(ctx,"[TX] EOT/NAK mismatch"); ctx->store->close_read(f); free(blk); return -107; }
        }
        y_putc(ctx,YMD_EOT);
        /* wait ACK (ignore others) */
        {
            uint32_t t2=ctx->timer->now_ms(); int got=0;
            while((int)(ctx->timer->now_ms()-t2) < ctx->cfg.rx_timeout_ms){
                if (y_getc(ctx,&ch,200)==1){
                    if (ch==YMD_ACK){ got=1; break; }
                    if (ch==YMD_CAN){ ylog(ctx,"[TX] canceled during EOT#2"); ctx->store->close_read(f); free(blk); return -108; }
                }
            }
            if(!got){ ylog(ctx,"[TX] EOT/ACK mismatch"); ctx->store->close_read(f); free(blk); return -108; }
        }

        drain_input(ctx);
        ctx->store->close_read(f);

        if (P.total > 0 && P.done < P.total) P.done = P.total;
        prog_tick(&P,true);
        prog_done_line(&P);
    }

    /* end session with empty block0 */
    if (send_block0(ctx,"",0)!=0){ ylog(ctx,"[TX] final empty block0 fail"); free(blk); return -109; }
    drain_input(ctx);
    free(blk);
    return 0;
}

// ================= 接收端核心 =================
static int read_packet_rest(const YContext* c, uint8_t code, uint8_t* buf, int* payload, int ms){
    if (code==YMD_SOH) *payload=YMD_BLK128;
    else if (code==YMD_STX) *payload=YMD_BLK1K;
    else { *payload=0; return 0; }
    int tsz=2+*payload+2;
    int n=y_readx(c,buf,tsz,ms);
    return (n==tsz)? 0 : -1;
}

int ymd_recv_multi(YContext* ctx, const char* out_dir){
    if (!ctx || !ctx->port || !ctx->timer || !ctx->store) return -200;

    uint8_t pkt[1+2+YMD_BLK1K+2]; int payload=0; uint8_t code=0; int file_cnt=0;

    for(;;){
        int tail=(file_cnt>0);
        int total_wait_ms= tail? (ctx->cfg.rx_timeout_ms+1000) : ctx->cfg.hs_total_ms;
        if (total_wait_ms<1000) total_wait_ms=1000;

        uint32_t t0=ctx->timer->now_ms(); int got0=0;

        for(;;){
            y_putc(ctx,YMD_CHC);
            if (y_getc(ctx,&code,1000)==1){
                if (code==YMD_SOH || code==YMD_STX){ got0=1; break; }
                if (code==YMD_CAN){ ylog(ctx,"[RX] canceled by sender"); return -201; }
            }
            if ((int)(ctx->timer->now_ms()-t0) > total_wait_ms){
                if (tail){ ylog(ctx,"[RX] tail idle -> session end"); return 0; }
                else { ylog(ctx,"[RX] wait block0 timeout"); return -202; }
            }
        }

        if (!got0) return -202;
        if (read_packet_rest(ctx,code,pkt,&payload,ctx->cfg.rx_timeout_ms)!=0){ ylog(ctx,"[RX] block0 rest timeout"); return -203; }

        uint8_t seq=pkt[0], inv=pkt[1];
        if ((uint8_t)(seq+inv)!=0xFF || seq!=0x00){ ylog(ctx,"[RX] block0 seq err"); return -204; }
        uint16_t rxcrc=((uint16_t)pkt[2+payload]<<8)|pkt[2+payload+1];
        if (rxcrc!=crc16_ccitt(pkt+2,payload)){ ylog(ctx,"[RX] block0 CRC err"); return -205; }

        const char* name=(const char*)(pkt+2);
        const char* sizeStr=name+(int)strlen(name)+1;
        int64_t fsz=0;
#if defined(_MSC_VER)
        if (*sizeStr) fsz=_strtoi64(sizeStr,NULL,10);
#else
        if (*sizeStr) fsz=(int64_t)strtoll(sizeStr,NULL,10);
#endif

        char fname[256]; snprintf(fname,sizeof(fname),"%s",name);
        if (!*fname){ y_putc(ctx,YMD_ACK); break; } /* end */

        ymodem_store_set_file_size_hint(fsz);
        void* fw=ctx->store->open_write(out_dir,fname);
        if (!fw){
            ymodem_store_set_file_size_hint(-1);
            y_putc(ctx,YMD_CAN); ylog(ctx,"[RX] create file fail"); return -206;
        }

        y_putc(ctx,YMD_ACK); y_putc(ctx,YMD_CHC);
        Prog P; prog_init(&P, ctx->timer,"[RX] ",fname,fsz);
        uint64_t done=0;

        for(;;){
            if (y_getc(ctx,&code,ctx->cfg.rx_timeout_ms)!=1){
                ylog(ctx,"\n[RX] data wait timeout"); ctx->store->close_write(fw,0); return -207;
            }
            if (code==YMD_SOH || code==YMD_STX){
                if (read_packet_rest(ctx,code,pkt,&payload,ctx->cfg.rx_timeout_ms)!=0){
                    ylog(ctx,"\n[RX] data rest timeout"); ctx->store->close_write(fw,0); return -208;
                }
                uint8_t s1=pkt[0], s2=pkt[1];
                if ((uint8_t)(s1+s2)!=0xFF){ y_putc(ctx,YMD_NAK); continue; }
                uint16_t c=((uint16_t)pkt[2+payload]<<8)|pkt[2+payload+1];
                if (c!=crc16_ccitt(pkt+2,payload)){ y_putc(ctx,YMD_NAK); continue; }

                int wlen=payload;
                if (fsz>=0 && (int64_t)wlen>(fsz-(int64_t)done)) wlen=(int)(fsz-(int64_t)done);
                if (wlen>0){
                    if (ctx->store->write(fw,pkt+2,wlen)!=wlen){
                        ylog(ctx,"\n[RX] write fail"); ctx->store->close_write(fw,0); return -209;
                    }
                    done+=(uint32_t)wlen;
                    P.done = (long long)done;
                    yprog(ctx,"[RX]",fname,done,fsz>0?fsz:-1);
                    prog_tick(&P,false);
                }
                y_putc(ctx,YMD_ACK);
            } else if (code==YMD_EOT){
                y_putc(ctx,YMD_NAK);
                uint8_t e2; uint32_t t1=ctx->timer->now_ms(); int ok=0;
                while((int)(ctx->timer->now_ms()-t1) < (ctx->cfg.rx_timeout_ms*2)){
                    if (y_getc(ctx,&e2,200)==1){
                        if (e2==YMD_EOT){ ok=1; break; }
                        if (e2==YMD_CAN){ ylog(ctx,"\n[RX] canceled during EOT"); ctx->store->close_write(fw,0); return -210; }
                    }
                }
                if(!ok){ ylog(ctx,"\n[RX] EOT seq err"); ctx->store->close_write(fw,0); return -210; }

                y_putc(ctx,YMD_ACK); y_putc(ctx,YMD_CHC);

                if (P.total > 0 && P.done < P.total) P.done = P.total;
                ctx->store->close_write(fw,1);
                prog_tick(&P,true);
                prog_done_line(&P);
                file_cnt++;
                break;
            } else if (code==YMD_CAN){
                ctx->store->close_write(fw,0); ylog(ctx,"\n[RX] canceled by sender"); return -211;
            } else {
                y_putc(ctx,YMD_NAK);
            }
        }
    }
    return 0;
}
```

</details>

<details>
<summary>component/ymodem/ymodem.h</summary>

```c
#ifndef __YMODEM_H
#define __YMODEM_H

#include <stdint.h>

/* -------------------------------
 *           Protocol defs
 * ------------------------------- */
#define YMD_SOH   0x01
#define YMD_STX   0x02
#define YMD_EOT   0x04
#define YMD_ACK   0x06
#define YMD_NAK   0x15
#define YMD_CAN   0x18
#define YMD_CHC   0x43  /* 'C' */

#define YMD_BLK128 128
#define YMD_BLK1K  1024

/* -------------------------------
 *       OOP-like Port Abstraction
 * ------------------------------- */
typedef struct YPort {
    void*  self;
    int  (*open)(struct YPort*, const char* name, uint32_t baud);
    void (*close)(struct YPort*);
    int  (*putc)(struct YPort*, uint8_t ch);
    int  (*getc)(struct YPort*, uint8_t* ch, int ms);
    int  (*write)(struct YPort*, const void* buf, int len);
    int  (*read_exact)(struct YPort*, void* buf, int len, int ms);
} YPort;

typedef struct YTimer {
    void* self;
    uint32_t (*now_ms)(void);
    void     (*sleep_ms)(int ms);
} YTimer;

typedef struct YStore {
    void* self;
    /* sender side */
    void*   (*open_read)(const char* path);
    int     (*read)(void* fh, void* buf, int len);
    int     (*seek)(void* fh, int64_t off, int whence);   /* optional */
    int64_t (*tell)(void* fh);                            /* optional */
    int64_t (*size)(void* fh);                            /* optional */
    void    (*close_read)(void* fh);
    /* receiver side */
    void*   (*open_write)(const char* out_dir, const char* name);
    int     (*write)(void* fh, const void* buf, int len);
    void    (*close_write)(void* fh, int ok);
} YStore;

/* 回调 */
typedef struct YHooks {
    void (*on_log)(const char* s);
    void (*on_progress)(const char* tag, const char* name, uint64_t done, int64_t total);
} YHooks;

/* 配置 */
typedef struct YConfig {
    int rx_timeout_ms;      /* 单步读超时，默认 3000 */
    int hs_total_ms;        /* 握手总时长，默认 20000 */
    int retry_max;          /* 数据块重试次数，默认 10 */
    int packet_prefer_1k;   /* 1=1K 包，0=128B 包 */
} YConfig;

/* 上下文 */
typedef struct YContext {
    YPort*  port;
    YTimer* timer;
    YStore* store;
    YHooks  hooks;
    YConfig cfg;
} YContext;

void ymodem_recv(const char* out_dir);
int ymodem_send(const char* const* files, int n);
int ymd_recv_multi(YContext* ctx, const char* out_dir);
int ymd_send_multi(YContext* ctx, const char* const* files, int nfiles);

void ymodem_store_set_file_size_hint(int64_t sz);
int64_t ymodem_store_get_file_size_hint(void);

#endif /* __YMODEM_H */
```

</details>

<details>
<summary>component/ymodem/ymodem_port.c</summary>

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ymodem.h"
#include "main.h"
#include "usart.h"

#define FLASH_STORAGE_START_ADDR 0x08040000U
#define FLASH_STORAGE_END_ADDR   0x08080000U

typedef struct {
    const uint8_t* base;
    uint32_t len;
    uint32_t pos;
} DummyReadHandle;

static const uint8_t g_dummy_payload[] =
"RocketPi UART/Flash YMODEM demo\r\n"
"--------------------------------\r\n"
"This file is generated inside firmware and streamed via the YMODEM sender.\r\n"
"You can verify the protocol end-to-end by checking that this sentence and the\r\n"
"mini log below are present on the PC after reception.\r\n"
"\r\n"
"Timestamp,Item,Value\r\n"
"00:00.0,Board,STM32F401\r\n"
"00:00.1,Build,rocketpi_uart_ymodem\r\n"
"00:00.2,Size,~1 KB\r\n"
"\r\n"
"Have fun testing!\r\n";
static const uint32_t g_dummy_payload_len = (uint32_t)(sizeof(g_dummy_payload) - 1U);

static void* dummy_open_read(const char* path){
    (void)path;
    DummyReadHandle* h = (DummyReadHandle*)malloc(sizeof(DummyReadHandle));
    if (!h) return NULL;
    h->base = g_dummy_payload;
    h->len  = g_dummy_payload_len;
    h->pos  = 0;
    return h;
}
static int dummy_read_bytes(void* fh, void* buf, int len){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (!h || !buf || len<=0) return -1;
    uint32_t remain = (h->pos < h->len)? (h->len - h->pos) : 0;
    if (remain==0) return 0;
    if ((uint32_t)len > remain) len = (int)remain;
    memcpy(buf, h->base + h->pos, (size_t)len);
    h->pos += (uint32_t)len;
    return len;
}
static int dummy_seek_any(void* fh, int64_t off, int whence){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (!h) return -1;
    int64_t target = 0;
    switch (whence){
        case 0: target = off; break;
        case 1: target = (int64_t)h->pos + off; break;
        case 2: target = (int64_t)h->len + off; break;
        default: return -1;
    }
    if (target < 0 || target > (int64_t)h->len) return -1;
    h->pos = (uint32_t)target;
    return 0;
}
static int64_t dummy_tell_any(void* fh){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (!h) return -1;
    return (int64_t)h->pos;
}
static int64_t dummy_size_any(void* fh){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (!h) return -1;
    return (int64_t)h->len;
}
static void dummy_close_read(void* fh){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (h) free(h);
}

typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t cursor;
    uint8_t  erased;
    uint8_t  busy;
} FlashStoreState;

typedef struct {
    FlashStoreState* state;
    uint32_t start_addr;
    uint32_t wrote;
    uint32_t capacity;
    uint32_t expected;
} FlashWriteHandle;

static FlashStoreState g_flash_store = {
    .start = FLASH_STORAGE_START_ADDR,
    .end   = FLASH_STORAGE_END_ADDR,
    .cursor = FLASH_STORAGE_START_ADDR,
    .erased = 0,
    .busy = 0,
};

static void flash_store_session_reset(void){
    g_flash_store.cursor = g_flash_store.start;
    g_flash_store.erased = 0;
    g_flash_store.busy = 0;
    ymodem_store_set_file_size_hint(-1);
}

static uint32_t flash_store_free_bytes(void){
    if (g_flash_store.cursor >= g_flash_store.end) return 0;
    return g_flash_store.end - g_flash_store.cursor;
}

static uint32_t flash_store_expected_bytes(void){
    int64_t hint = ymodem_store_get_file_size_hint();
    if (hint <= 0) return 0;
    if (hint > (int64_t)UINT32_MAX) return UINT32_MAX;
    return (uint32_t)hint;
}

static void flash_clear_error_flags(void){
    uint32_t flags = 0;
#ifdef FLASH_FLAG_EOP
    flags |= FLASH_FLAG_EOP;
#endif
#ifdef FLASH_FLAG_OPERR
    flags |= FLASH_FLAG_OPERR;
#endif
#ifdef FLASH_FLAG_WRPERR
    flags |= FLASH_FLAG_WRPERR;
#endif
#ifdef FLASH_FLAG_PGAERR
    flags |= FLASH_FLAG_PGAERR;
#endif
#ifdef FLASH_FLAG_PGPERR
    flags |= FLASH_FLAG_PGPERR;
#endif
#ifdef FLASH_FLAG_PGSERR
    flags |= FLASH_FLAG_PGSERR;
#endif
    if (flags) {
        __HAL_FLASH_CLEAR_FLAG(flags);
    }
}

static int flash_store_erase_all(void){
    HAL_StatusTypeDef st;
    uint32_t err = 0;

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return -1;
    }
    flash_clear_error_flags();

    FLASH_EraseInitTypeDef erase = {0};
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = FLASH_SECTOR_6;
    erase.NbSectors = 2;
    st = HAL_FLASHEx_Erase(&erase, &err);

    HAL_FLASH_Lock();

    if (st != HAL_OK || err != 0xFFFFFFFFU) {
        printf("[FLASH] erase failed, status=%ld err=%lu\r\n", (long)st, (unsigned long)err);
        return -1;
    }
    return 0;
}

static void* flash_open_write(const char* out_dir, const char* name){
    (void)out_dir; (void)name;
    if (g_flash_store.busy) return NULL;
    if (!g_flash_store.erased){
        if (flash_store_erase_all() != 0) return NULL;
        g_flash_store.erased = 1;
    }
    uint32_t avail = flash_store_free_bytes();
    if (avail == 0) return NULL;

    FlashWriteHandle* h = (FlashWriteHandle*)malloc(sizeof(FlashWriteHandle));
    if (!h) return NULL;
    h->state = &g_flash_store;
    h->start_addr = g_flash_store.cursor;
    h->wrote = 0;
    h->capacity = avail;
    h->expected = flash_store_expected_bytes();
    if (h->expected > 0 && h->expected > h->capacity){
        free(h);
        return NULL;
    }
    g_flash_store.busy = 1;
    return h;
}

static int flash_write_bytes(void* fh, const void* buf, int len){
    FlashWriteHandle* h=(FlashWriteHandle*)fh;
    if (!h || !buf || len<=0) return 0;
    if ((uint32_t)len > (h->capacity - h->wrote)){
        len = (int)(h->capacity - h->wrote);
    }
    if (len<=0) return 0;

    const uint8_t* src=(const uint8_t*)buf;
    if (HAL_FLASH_Unlock() != HAL_OK) return -1;

    for (int i=0;i<len;++i){
        uint32_t addr = h->start_addr + h->wrote;
        HAL_StatusTypeDef st = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, (uint64_t)src[i]);
        if (st != HAL_OK){
            HAL_FLASH_Lock();
            return -1;
        }
        h->wrote++;
    }

    HAL_FLASH_Lock();
    return len;
}

static void flash_close_write(void* fh, int ok){
    FlashWriteHandle* h=(FlashWriteHandle*)fh;
    if (!h) return;
    if (ok){
        h->state->cursor = h->start_addr + h->wrote;
    }
    h->state->busy = 0;
    free(h);
    ymodem_store_set_file_size_hint(-1);
}

static uint32_t stm32_now_ms(void){ return HAL_GetTick(); }
static void     stm32_sleep_ms(int ms){ HAL_Delay(ms); }

typedef struct {
    UART_HandleTypeDef* huart;
} STM32Serial;

static int stm32_open(YPort* p, const char* name, uint32_t baud){
    (void)name; (void)baud;
    STM32Serial* s = (STM32Serial*)p->self;
    return (s && s->huart) ? 0 : -1;
}
static void stm32_close(YPort* p){ (void)p; }

static int stm32_putc(YPort* p, uint8_t ch){
    STM32Serial* s=(STM32Serial*)p->self;
    if (!s || !s->huart) return -1;
    if (HAL_UART_Transmit(s->huart, &ch, 1, 1000) == HAL_OK) return 1;
    return -1;
}
static int stm32_getc(YPort* p, uint8_t* ch, int ms){
    STM32Serial* s=(STM32Serial*)p->self;
    if (!s || !s->huart) return -1;
    if (HAL_UART_Receive(s->huart, ch, 1, (ms>=0)? (uint32_t)ms : HAL_MAX_DELAY) == HAL_OK) return 1;
    return 0;
}
static int stm32_write(YPort* p, const void* buf, int len){
    STM32Serial* s=(STM32Serial*)p->self;
    if (!s || !s->huart) return -1;
    if (HAL_UART_Transmit(s->huart, (uint8_t*)buf, (uint16_t)len, 5000) == HAL_OK) return len;
    return -1;
}
static int stm32_read_exact(YPort* p, void* buf, int len, int ms){
    STM32Serial* s=(STM32Serial*)p->self;
    if (!s || !s->huart) return -1;
    uint32_t start = HAL_GetTick();
    uint8_t* q=(uint8_t*)buf; int got=0;
    while (got < len){
        uint32_t elapsed = HAL_GetTick() - start;
        uint32_t budget = (ms>0 && ms > (int)elapsed) ? (ms - elapsed) : 0;
        if (budget==0) break;
        if (HAL_UART_Receive(s->huart, q+got, 1, budget) == HAL_OK){
            got++;
        }
    }
    return (got==len)? len : -1;
}

static void cli_log(const char* s){
    printf("%s\r\n", s);
}
static void cli_prog(const char* tag, const char* name, uint64_t done, int64_t total){
    (void)tag; (void)name; (void)done; (void)total;
}

extern UART_HandleTypeDef huart2;

static void ymodem_make_ctx(YContext* out_ctx, UART_HandleTypeDef* huart){
    static STM32Serial ss;
    static YPort  port;
    static YTimer timer;
    static YStore store;

    ss.huart = huart;

    port.self = &ss;
    port.open = stm32_open;
    port.close= stm32_close;
    port.putc = stm32_putc;
    port.getc = stm32_getc;
    port.write= stm32_write;
    port.read_exact = stm32_read_exact;

    timer.self = NULL;
    timer.now_ms = stm32_now_ms;
    timer.sleep_ms = stm32_sleep_ms;

    store.self = NULL;
    store.open_read  = dummy_open_read;
    store.read       = dummy_read_bytes;
    store.seek       = dummy_seek_any;
    store.tell       = dummy_tell_any;
    store.size       = dummy_size_any;
    store.close_read = dummy_close_read;

    store.open_write = flash_open_write;
    store.write      = flash_write_bytes;
    store.close_write= flash_close_write;

    out_ctx->port  = &port;
    out_ctx->timer = &timer;
    out_ctx->store = &store;
    out_ctx->hooks.on_log = cli_log;
    out_ctx->hooks.on_progress = cli_prog;
    out_ctx->cfg.rx_timeout_ms    = 3000;
    out_ctx->cfg.hs_total_ms      = 20000;
    out_ctx->cfg.retry_max        = 10;
    out_ctx->cfg.packet_prefer_1k = 1;
}

void ymodem_recv(const char* out_dir){
    (void)out_dir;
    flash_store_session_reset();
    YContext ctx;
    ymodem_make_ctx(&ctx, &huart2);
    int rc = ymd_recv_multi(&ctx, "");
    printf("[YMODEM] recv rc=%d\r\n", rc);
}

int ymodem_send(const char* const* files, int n){
    YContext ctx;
    ymodem_make_ctx(&ctx, &huart2);
    if (!files || n<=0) return -1;
    return ymd_send_multi(&ctx, files, n);
}
```

</details>

<!-- BSP_DRIVERS_END -->
