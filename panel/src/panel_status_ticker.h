#pragma once

/** Idle status ticker on bottom alarm bar (no fault). Test only. */

void panel_status_ticker_begin();
/** Call often while LVGL locked. Pass hub_visible=false on detail screens. */
void panel_status_ticker_apply_ui(bool hub_visible = true);