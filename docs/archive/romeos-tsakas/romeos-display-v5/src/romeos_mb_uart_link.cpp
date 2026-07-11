#include "romeos_mb_uart_link.h"

#include <Arduino.h>
#include <HardwareSerial.h>

#include "romeos_uart_frame.h"

/* Μην χρησιμοποιείς GPIO17 — buzzer οθόνης. Σύνδεση: DISP_TX → MB_RX, DISP_RX ← MB_TX */
#ifndef ROMEOS_UART_DISP_RX
#define ROMEOS_UART_DISP_RX 47
#endif
#ifndef ROMEOS_UART_DISP_TX
#define ROMEOS_UART_DISP_TX 21
#endif
#ifndef ROMEOS_UART_BAUD
#define ROMEOS_UART_BAUD 115200
#endif

namespace {

constexpr uint32_t k_ping_ms = 1000u;
constexpr uint32_t k_stale_ms = 2500u;

HardwareSerial s_mb_uart(1);
uint8_t s_seq = 0;
uint32_t s_last_pong_ms = 0;
uint32_t s_last_ping_ms = 0;

static uint8_t frame_chk(uint8_t magic, uint8_t type, uint8_t seq)
{
    return static_cast<uint8_t>(magic ^ type ^ seq);
}

static void send_ping(void)
{
    romeos_uart_frame_t f{};
    f.magic = ROMEOS_UART_FRAME_MAGIC;
    f.type = ROMEOS_UART_TYPE_PING;
    f.seq = s_seq++;
    f.chk = frame_chk(f.magic, f.type, f.seq);
    s_mb_uart.write(reinterpret_cast<const uint8_t *>(&f), sizeof(f));
}

static void drain_rx(void)
{
    romeos_uart_frame_t f{};
    while (s_mb_uart.available() >= static_cast<int>(sizeof(f))) {
        const int r = s_mb_uart.read(reinterpret_cast<uint8_t *>(&f), sizeof(f));
        if (r != static_cast<int>(sizeof(f))) {
            break;
        }
        if (f.magic != ROMEOS_UART_FRAME_MAGIC || f.type != ROMEOS_UART_TYPE_PONG) {
            continue;
        }
        if (f.chk != frame_chk(f.magic, f.type, f.seq)) {
            continue;
        }
        s_last_pong_ms = millis();
    }
}

}  // namespace

void romeos_mb_uart_link_begin(void)
{
    s_mb_uart.begin(ROMEOS_UART_BAUD, SERIAL_8N1, ROMEOS_UART_DISP_RX, ROMEOS_UART_DISP_TX);
    s_last_pong_ms = 0;
    s_last_ping_ms = 0;
    Serial.printf("[mb_uart] display UART1 RX=%d TX=%d baud=%d (ping only)\n",
                  static_cast<int>(ROMEOS_UART_DISP_RX),
                  static_cast<int>(ROMEOS_UART_DISP_TX),
                  static_cast<int>(ROMEOS_UART_BAUD));
}

void romeos_mb_uart_link_poll(void)
{
    drain_rx();
    const uint32_t now = millis();
    if (now - s_last_ping_ms >= k_ping_ms) {
        s_last_ping_ms = now;
        send_ping();
    }
}

bool romeos_mb_uart_link_connected(void)
{
    if (s_last_pong_ms == 0u) {
        return false;
    }
    return (millis() - s_last_pong_ms) < k_stale_ms;
}
