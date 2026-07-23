#pragma once

/** Home Wi‑Fi + NTP → update hub top-bar clock/date labels. */

void panel_wifi_ntp_begin();
/** Network / NTP (no LVGL). Call often from loop. */
void panel_wifi_ntp_poll();
/**
 * Push buffered time into LVGL labels. Call only while LVGL locked.
 * screen_id: EEZ ScreensEnum (1=main … 7=wifi). Only that screen's clock is updated.
 */
void panel_wifi_ntp_apply_ui(int screen_id);
