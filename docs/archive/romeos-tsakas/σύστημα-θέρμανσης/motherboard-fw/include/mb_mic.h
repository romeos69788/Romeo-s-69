#pragma once

#include <stddef.h>
#include <stdint.h>

void mb_mic_begin();
void mb_mic_poll();

bool mb_mic_begin_ok();
bool mb_mic_has_signal();

/** Γραμμή 3 LCD — μόνο μικρόφωνο / τελευταία εντολή. */
void mb_mic_format_lcd_line(char *buf, size_t len);

void mb_mic_run_diag();

/** Serial: MIC CMD <κείμενο> — εμφάνιση επιβεβαίωσης στην L3. */
bool mb_mic_handle_serial(const char *line);
