EEZ Studio project «othoni_7» (800×480).

Μετά από αλλαγές στο Studio → Build → αντιγραφή:
  src\ui\  →  ..\src\othoni_ui\
Στα .h: #include <lvgl/lvgl.h> → #include "lvgl.h"

Firmware: ..\README.md — pio run -e BOARD_VIEWE_OTHONI_7 -t upload
