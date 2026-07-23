#pragma once

/** Tile → detail slide-left; Μενού → hub slide-right. */

void panel_nav_begin();
/** True while the hub (main) screen is active. */
bool panel_nav_on_hub();
/** True while a screen-load animation is running — skip heavy UI work. */
bool panel_nav_busy();
/** Active EEZ screen id (1 = main … 7 = wifi). */
int panel_nav_screen_id();
