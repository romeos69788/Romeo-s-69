#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void romeos_mb_uart_link_begin(void);
void romeos_mb_uart_link_poll(void);
bool romeos_mb_uart_link_connected(void);

#ifdef __cplusplus
}
#endif
