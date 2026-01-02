#pragma once

#include <stddef.h>
#include <stdio.h>

/**
 * @file debug_driver.h
 * @brief 将 stdio 重定向到调试 UART 的接口，以及轻量级日志辅助函数。
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
/**
 * @brief 将 newlib 的 write 系统调用重定向到调试 UART。
 * @param file C 库提供的文件描述符。
 * @param ptr  待发送缓冲区指针。
 * @param len  发送的字节数。
 * @return 已写入的字节数。
 */
int _write(int file, char *ptr, int len);

/**
 * @brief 将 newlib 的 read 系统调用重定向到调试 UART。
 * @param file C 库提供的文件描述符。
 * @param ptr  接收缓冲区指针。
 * @param len  期望读取的字节数。
 * @return 实际读取的字节数。
 */
int _read(int file, char *ptr, int len);
#elif defined(__ARMCC_VERSION)
/**
 * @brief 将 fputc 重定向到调试 UART。
 * @param ch 待发送字符。
 * @param f  被忽略的文件句柄。
 * @return 实际发送的字符。
 */
int fputc(int ch, FILE *f);

/**
 * @brief 将 fgetc 重定向到调试 UART。
 * @param f 被忽略的文件句柄。
 * @return 接收到的字符（int）。
 */
int fgetc(FILE *f);

/**
 * @brief 使用标准 C 库时，关闭半主机退出钩子。
 * @param x 退出码（未使用）。
 */
void _sys_exit(int x);
#endif

/**
 * @brief 通过调试 UART 发送以 0 结尾的字符串，并规范化 CRLF。
 * @param s 待发送的字符串（可为 NULL）。
 */
void uart_puts(const char *s);

/**
 * @brief printf 风格的调试 UART 输出辅助。
 * @param fmt 格式化描述字符串。
 * @return 预期写入的字符数，错误返回负值。
 */
int uart_printf(const char *fmt, ...);

/**
 * @brief 十六进制转储工具，便于查看二进制缓冲区。
 * @param data 缓冲区起始指针。
 * @param len  要转储的字节数。
 * @param title 可选标题字符串，可为 NULL。
 */
void uart_hexdump(const void *data, size_t len, const char *title);

#ifdef __cplusplus
}
#endif
