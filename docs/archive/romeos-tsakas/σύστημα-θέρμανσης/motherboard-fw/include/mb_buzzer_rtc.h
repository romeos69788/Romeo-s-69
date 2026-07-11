#pragma once

#include <Arduino.h>

/** Ενεργό buzzer module (KY-012): HIGH = ήχος. */
void mb_buzzer_set(bool on);

/** Τρία μπιπ κατά την εκκίνηση (12 V ON / reset). Μπλοκάρει ~700 ms. */
void mb_buzzer_boot_chime();

/** Serial: BUZZ ON/OFF/TEST, I2C SCAN, RTC READ, BUZZ HELP. Επιστρέφει true αν χειρίστηκε. */
bool mb_buzzer_rtc_handle_serial(const char *line);

void mb_buzzer_rtc_print_help();

/** Γραμμή 4 LCD 2004: DD/MM/YYYY HH:MM:SS από U9 DS3231. */
void mb_rtc_format_lcd_line(char *buf, size_t len);

/** SNTP (Ελλάδα) όταν Wi‑Fi συνδεδεμένο — γράφει U9 DS3231 για LCD γραμμή 4. */
void mb_rtc_ntp_poll(void);
