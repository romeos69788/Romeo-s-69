#pragma once

/** Home Wi‑Fi + NTP → update hub top-bar clock/date labels. */

void panel_wifi_ntp_begin();
/** Network / NTP (no LVGL). Call often from loop. */
void panel_wifi_ntp_poll();
/** Push buffered time into LVGL labels. Call only while LVGL locked. */
void panel_wifi_ntp_apply_ui();
