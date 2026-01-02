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
