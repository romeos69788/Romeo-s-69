#pragma once

/** Idle status ticker on bottom alarm bar (no fault). Test only. */

void panel_status_ticker_begin();
/** Call often while LVGL locked. */
void panel_status_ticker_apply_ui();
