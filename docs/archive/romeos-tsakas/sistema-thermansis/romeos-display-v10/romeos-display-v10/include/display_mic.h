#pragma once

#include <stddef.h>
#include <stdint.h>

/** Γρήγορο init (χωρίς hunt) — μία φορά στο boot. */
void display_mic_init();

void display_mic_set_listening(bool on);
bool display_mic_is_listening();

void display_mic_poll();

bool display_mic_begin_ok();
bool display_mic_has_signal();
int32_t display_mic_last_peak();

/** cmd: "menu" ή "heat" */
typedef void (*display_mic_voice_fn)(const char *cmd);
void display_mic_set_voice_handler(display_mic_voice_fn fn);

bool display_mic_handle_serial(const char *line);
