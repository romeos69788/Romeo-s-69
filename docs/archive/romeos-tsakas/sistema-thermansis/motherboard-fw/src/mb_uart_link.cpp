#include <Arduino.h>
#include <HardwareSerial.h>

#include "romeos_uart_frame.h"

#ifndef ROMEOS_UART_MB_RX
#define ROMEOS_UART_MB_RX 9
#endif
#ifndef ROMEOS_UART_MB_TX
#define ROMEOS_UART_MB_TX 10
#endif
#ifndef ROMEOS_UART_BAUD
#define ROMEOS_UART_BAUD 115200
#endif

namespace {

HardwareSerial s_disp_uart(1);

static uint8_t frame_chk(uint8_t magic, uint8_t type, uint8_t seq)
{
    return static_cast<uint8_t>(magic ^ type ^ seq);
}

static void send_pong(uint8_t seq)
{
    romeos_uart_frame_t f{};
    f.magic = ROMEOS_UART_FRAME_MAGIC;
    f.type = ROMEOS_UART_TYPE_PONG;
    f.seq = seq;
    f.chk = frame_chk(f.magic, f.type, f.seq);
    s_disp_uart.write(reinterpret_cast<const uint8_t *>(&f), sizeof(f));
}

}  // namespace

void mb_uart_link_begin(void)
{
    s_disp_uart.begin(ROMEOS_UART_BAUD, SERIAL_8N1, ROMEOS_UART_MB_RX, ROMEOS_UART_MB_TX);
    Serial.printf("[mb_uart] UART1 RX=%d TX=%d baud=%d (pong only)\n",
                  static_cast<int>(ROMEOS_UART_MB_RX),
                  static_cast<int>(ROMEOS_UART_MB_TX),
                  static_cast<int>(ROMEOS_UART_BAUD));
}

void mb_uart_link_poll(void)
{
    romeos_uart_frame_t f{};
    while (s_disp_uart.available() >= static_cast<int>(sizeof(f))) {
        const int r = s_disp_uart.read(reinterpret_cast<uint8_t *>(&f), sizeof(f));
        if (r != static_cast<int>(sizeof(f))) {
            break;
        }
        if (f.magic != ROMEOS_UART_FRAME_MAGIC || f.type != ROMEOS_UART_TYPE_PING) {
            continue;
        }
        if (f.chk != frame_chk(f.magic, f.type, f.seq)) {
            continue;
        }
        send_pong(f.seq);
    }
}
