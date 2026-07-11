/**
 * Απλή σύνδεση UART οθόνη ↔ μητρική — μόνο «ζωντανό» link (ping/pong).
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ROMEOS_UART_FRAME_MAGIC 0x52u /* 'R' */
#define ROMEOS_UART_TYPE_PING   0x01u /* οθόνη → μητρική */
#define ROMEOS_UART_TYPE_PONG   0x02u /* μητρική → οθόνη */

typedef struct __attribute__((packed)) {
    uint8_t magic;
    uint8_t type;
    uint8_t seq;
    uint8_t chk;
} romeos_uart_frame_t;

#ifdef __cplusplus
}
#endif
