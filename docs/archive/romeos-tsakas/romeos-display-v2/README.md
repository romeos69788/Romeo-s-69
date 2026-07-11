| Supported ESP SoCs | ESP32 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 | ESP32-P4 |
| ------------------ | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

## Romeos display v2 (δεύτερη οθόνη)

**Ξεχωριστό project** από το [`../thermostat-ui-demo`](../thermostat-ui-demo): ίδιο **Viewe UEDX80480070E-WB-A** (800×480), ίδιες ρυθμίσεις panel / CH340 / ROOM I2C 19–20, **χωρίς** σύνδεση στην **παλιά** μητρική (`ROMEOS_DISPLAY_STANDALONE=1` στο `platformio.ini` → stub στο `romeos_display_link.cpp`). Στόχος: νέο SquareLine export + λογική εδώ· όταν έρθει **νέα** μητρική, αφαίρεση standalone και ίδιο ESP32 όπως το παλιό → ελάχιστες αλλαγές hardware.

**Το παλιό** `thermostat-ui-demo` μένει για άλλη χρήση (π.χ. φίλος / λέβητας πετρελαίου) — **μην** ανοίγεις τα δύο projects στο ίδιο PlatformIO παράθυρο χωρίς να κοιτάς το path.

**Συζήτηση 2026-04-26 (UI SquareLine, ρολόι, 6 εικόνες σπιτιού, αισθητήρες):** [`docs/DISCUSSION_2026-04-26_UI_AND_SENSORS.md`](./docs/DISCUSSION_2026-04-26_UI_AND_SENSORS.md)

**Συνέχεια Viewe (ROOM / SHT / COM4 — κοινό hardware):** [`../HANDOFF_2026-04-26.md`](../HANDOFF_2026-04-26.md) · [`docs/VIEWE_ROOM_SENSOR_AND_SERIAL.md`](./docs/VIEWE_ROOM_SENSOR_AND_SERIAL.md)

**Wi‑Fi / NVS / HOME (οθόνη v2, 2026-04-30):** [`docs/WIFI_HOME_DISPLAY_V2.md`](./docs/WIFI_HOME_DISPLAY_V2.md)

### Οθόνη v2 — `src/main_clean.cpp` (ενημέρωση **2026-05-01**)

- **Σημείο επαναφοράς (rollback / «καφεδεχόμενο»):** [`docs/RESTORE_POINT_2.md`](./docs/RESTORE_POINT_2.md) — λίστα αρχείων, σύντομος χάρτης φακέλων, οδηγίες επαναφοράς. Προηγούμενο: [`docs/RESTORE_POINT_1.md`](./docs/RESTORE_POINT_1.md).
- **Υπο-οθόνες (placeholder):** τα τέσσερα κουμπιά αριστερά (`HEATING`, `LIGHTS`, `HEAT_PUMP`, `BLINDS`) ανοίγουν πλήρη οθόνη 800×480 με κεντρικό αγγλικό τίτλο (**HEATING CONTROL**, **LIGHTING CONTROL**, **HEAT PUMP CONTROL**, **BLINDS CONTROL**). Κουμπί **BACK** κάτω-δεξιά → `lv_scr_load(ui_THERMOSTAT_1)`. Υλοποίηση: `build_placeholder_subscreens()`, `on_nav_sub_placeholder()`, `on_back_to_main()` στο `main_clean.cpp` (έτοιμο για μελλοντική πλήρωση από την πλακέτα).
- **Ρολόι και ημερομηνία:** ο εσωτερικός χρονοδιακοπής 1 s ενημερώνει ώρα/λεπτά· στη μετάβαση **23:59 → 00:00** καλείται `bump_calendar_at_midnight()` ώστε ημέρα, μήνας, έτος (με `days_in_month` / δίσεκτα) και `s_weekday` (όνομα ημέρας στο `ui_Label4`) να μένουν συμβατά με την ώρα της οθόνης.
- **Ήδη στο ίδιο αρχείο:** Wi‑Fi HOME/HUB, ζώνη ημέρας–νύχτας (5 εικόνες), ROOM, setpoint, επεξεργασία ρολογιού/ημερομηνίας, `strip_thermostat_button_pressed_focus_visuals` (συμπ. `CLICK_FOCUSABLE` / group), demo συναγερμού, on-screen marker **`CLEAN-V2 BASIC+CF`** κ.λπ. Θέμα SquareLine: **`lv_theme_basic_init`** στο `src/squareline/ui.c` όταν `LV_USE_THEME_BASIC`.

**Επανεκκίνηση συζήτησης:** [`../HANDOFF_2026-04-20.md`](../HANDOFF_2026-04-20.md) · **Παράδοση / §9 (19/4):** [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) (Wi‑Fi home + failover, NVS, ρολόι/NTP, PlatformIO)· **Οθόνη Love:** [`docs/UI_LOVE_screen_checkpoint.md`](./docs/UI_LOVE_screen_checkpoint.md)

# LVGL (v8) Porting Example

## Thermostat UI (Romeos) — τελικό σχέδιο

Το **SquareLine export** (`src/squareline/`) και το board **Viewe UEDX80480070E-WB-A** (800×480) χρησιμοποιούνται και εδώ (**`romeos-display-v2`**) και στο αδελφό **[`thermostat-ui-demo`](../thermostat-ui-demo)**. Στο **demo** η πλούσια λογική (Love screen, γωνίες, ντεμο κ.λπ.) είναι στο **`thermostat-ui-demo/src/main.cpp`**. Στο **v2** το τρέχον firmware είναι **`romeos-display-v2/src/main_clean.cpp`** (το `main.cpp` του v2 αποκλείεται στο `platformio.ini`).

| Θέμα | Σημείωση |
|------|----------|
| Ενημέρωση θερμοκρασίας δεξιά (Solar / Boiler / Intel / Outlet) | **Κάθε 2 s**, ίδιο με ROOM και OUT (`k_corner_temp_demo_ms`) |
| Δεύτερη οθόνη (`s_scr_love`) | **Μόνο [`thermostat-ui-demo`](../thermostat-ui-demo):** στο `main.cpp` (`squareline_love_screen_ensure_created`)· **7 γραμμές** grid, ρόλοι/φώτα/MENU, μετάβαση δεξιού βελακιού από `ui_Romeos1`. Λεπτομέρειες: [`docs/UI_LOVE_screen_checkpoint.md`](./docs/UI_LOVE_screen_checkpoint.md). |
| Αρχεία SquareLine | **v2 (εδώ):** `src/squareline/screens/ui_THERMOSTAT_1.c` / `.h`· **demo:** `thermostat-ui-demo/.../ui_Romeos1.*` |
| Build | PlatformIO (`platformio.ini`), env board Viewe |
| **Ενδείξεις Wi‑Fi** | Στο **`main_clean.cpp`**: **HOME** — πράσινο + παλμός τόξων όταν `romeos_display_link_home_wifi_assoc()` (STA στο NVS SSID). Στο παλιό `main.cpp` / SquareLine: ίδια λογική με `ui_room3` / `ui_room4`. Serial: `[romeos_link] WiFi OK` · `HOME_ASSOC=yes/no`. Λεπτομέρειες: [`docs/WIFI_HOME_DISPLAY_V2.md`](./docs/WIFI_HOME_DISPLAY_V2.md). |

Για επαναφορά σε αυτή την κατάσταση: `git checkout` στο τελευταίο commit ή tag **`thermostat-final-design`**.

**Συνοπτικές σημειώσεις συζητήσεων (υδραυλικό, μητρική, κουτί, Buzz κ.λπ.):** [docs/romeos-design-notes.md](./docs/romeos-design-notes.md)

### Πού μπαίνει ο κωδικός Wi‑Fi σπιτιού (όχι σε αρχείο)

**Δεν** τον γράφεις στο `platformio.ini` ούτε σε `.cpp` — **μόνο** στη μνήμη του ESP από **Serial** (οθόνη v2 = **`romeos-display-v2`**, παλιά οθόνη = `thermostat-ui-demo`, μητρική = `motherboard-fw`).

1. Άνοιξε **Serial Monitor** στο **σωστό project** και **COM** (Viewe CH340 συχνά **COM3**)· **115200**· τέλος γραμμής **LF**. Για εξωτερικό script/upload, **κλείσε** πρώτα το monitor (αλλιώς η θύρα είναι κλειδωμένη).
2. Γραμμή + Enter: `WIFI_HOME_SET`
3. Γραμμή + Enter: **SSID** (ακριβώς όπως στο router).
4. Γραμμή + Enter: **κωδικός** — ή **κενή γραμμή** (μόνο Enter) αν το AP είναι **ανοιχτό (open)**· αλλιώς το ESP μπορεί να μείνει με `last_disc_reason=210` (ασυμβατή ασφάλεια).

Περίμενε `[wifi_nvs] home Wi‑Fi saved to NVS`. Με το τρέχον firmware η σύνδεση **εφαρμόζεται αμέσως** (`romeos_display_link_on_home_wifi_saved_to_nvs`) — το **reset** είναι προαιρετικό. Έλεγχος: `WIFI_HOME_SHOW`· βοήθεια: `WIFI_HOME_HELP`· διαγραφή: `WIFI_HOME_CLR`.

**Σημείωση:** το ESP32 έχει **μόνο 2.4 GHz**· SSID μόνο-5GHz δεν συνδέεται. Πλήρης σημειογραφία: [`docs/WIFI_HOME_DISPLAY_V2.md`](./docs/WIFI_HOME_DISPLAY_V2.md).

**Αυτόματα (Windows):** φάκελος [`../scripts/`](../scripts/) — κλείσε Serial Monitor στο Cursor, μετά διπλό κλικ στο **`SET-WIFI-DISPLAY-COM4.bat`** (οθόνη) ή **`SET-WIFI-MOTHERBOARD-COM7.bat`** (μητρική)· θα ζητήσει **μόνο** τον κωδικό. Αν το COM σου είναι άλλο: `powershell -File scripts\set-wifi-home.ps1 -Port COMx`.

### Μετά από flash μητρικής — ενημέρωση οθόνης

Το firmware της **οθόνης** είναι **ξεχωριστό project** (`thermostat-ui-demo`, ESP32‑S3). Αν άλλαξες πρωτόκολλο/link/NVS που αφορά και τα δύο ESP, κάνε **`pio run -t upload`** εδώ στο **COM της οθόνης** (στο `platformio.ini` συνήθως διαφορετικό COM από τη μητρική). Στο LAN ενημέρωσε το **`ROMEOS_MB_LAN_IP`** ώστε να ταιριάζει με την τρέχουσα IP της μητρικής (`shared/include/romeos_link_types.h`).

### Ρολόι & ώρα δικτύου

- **LVGL / ρολόι:** αποφυγή μεγάλης απόκλισης με βάση monotonic (`esp_timer_get_time`) + rebase ώρας — `src/main.cpp` (λπ. `squareline_clock_*`).
- **SNTP / internet:** `src/romeos_ntp.cpp` — ξεκινάει μόνο με **Wi‑Fi συνδεδεμένο** και διαδρομή προς internet (NTP UDP 123 προς `pool.ntp.org` / `time.google.com`). Ζώνη **EET-2 / EEST-3** (Ελλάδα). Σε **RomeosMB-only** συνήθως **δεν** γίνεται sync· το ρολόι βασίζεται σε NVS + monotonic (`main.cpp`). Serial: `[romeos_ntp] wall clock set from NTP` όταν έχει γίνει sync· αν μετά από ~60 s δεν έχει sync, εκτυπώνεται προειδοποίηση.

### Ενδείξεις Wi‑Fi στην κεντρική οθόνη

- **Λογική:** `romeos_display_link_home_wifi_assoc()` στο `romeos_display_link.cpp` (σύγκριση SSID από `esp_wifi_sta_get_ap_info()` με NVS, trim μόνο στο SSID).
- **`romeos-display-v2` + `main_clean.cpp`:** animation HOME Wi‑Fi (`HomeWifiAnim`)· χωρίς NVS home **δεν** γίνεται αυτόματο fallback στο `RomeosMB` (μένει επανάληψη στο home SSID). **`thermostat-ui-demo` / `main.cpp`:** `squareline_wifi_link_status_timer_cb` κ.λπ.
- **Αν το HOME μένει κόκκινο:** λάθος SSID/κωδικός, μόνο 5 GHz, open AP με λάθος αποθηκευμένο κωδικό (βλ. `last_disc_reason` στο serial — `210` = open vs WPA mismatch), ή AP εκτός εμβέλειας.
- **Αν το εικονίδιο «μητρική / UDP» μένει κόκκινο** με πράσινο HOME: συνήθως λείπει **`ROMEOS_MB_LAN_IP`** στο `platformio.ini` της οθόνης ή η μητρική δεν στέλνει UDP στο LAN.

### Επόμενα βήματα (μητρική)

**Κατάσταση link:** η **επικοινωνία μητρική ↔ οθόνη** (SoftAP `RomeosMB`, UDP, structs στο `shared/`) είναι **ενεργή και επιβεβαιωμένη** στο hardware — βλ. [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) §2.8, §7.

**Συνέχεια firmware:** πραγματική τηλεμετρία (`-DROMEOS_MB_TELEMETRY_OK` στη μητρική, πραγματικές τιμές στο `build_tx()`), λογική K1–K6, αισθητήρες — όπως πίνακας στο `motherboard-fw/README.md`. Η βάση UI: **`romeos-display-v2`** → `src/main_clean.cpp` + SquareLine `ui_THERMOSTAT_1`· **demo** → `thermostat-ui-demo/src/main.cpp` + `ui_Romeos1`.

## Overview

This example demonstrates how to port `LVGL v8`. And for `RGB/MIPI-DSI` interface LCDs, it can enable the avoid tearing and rotation function.

## How to Use

### Step 1. Configure the libraries

- [Optional] `ESP32_Display_Panel`:

  - This example already has the [esp_panel_board_custom_conf.h](./src/esp_panel_board_custom_conf.h) and [esp_panel_drivers_conf.h](./src/esp_panel_drivers_conf.h) configuration files in the project directory. Edit these files as needed
  - see [Board Configuration Guide](../../../docs/envs/use_with_arduino.md#configuration-guide) for more information

- [Optional] `esp-lib-utils` :

  - This example already has the [esp_utils_conf.h](./src/esp_utils_conf.h) configuration file in the project directory. Edit this file as needed
  - See [Configuring esp-lib-utils](../../../docs/envs/use_with_arduino.md#configuring-esp-lib-utils) section for more information

- [Optional] `lvgl` :

  - This example already has the [lv_conf.h](./src/lv_conf.h) configuration file which been modified with the recommended configurations in the project directory. Edit this file as needed
  - See [Configuring LVGL](../../../docs/envs/use_with_arduino.md#configuring-lvgl) section for more information

### Step 2. Configure PlatformIO

- This example uses the `BOARD_ESPRESSIF_ESP32_S3_LCD_EV_BOARD_2_V1_5` board as default. You can choose another board in the `[platformio]:default_envs` of the [platformio.ini](./platformio.ini) file.
- If there is no the board you want to use, please check the following:

  - **If using a [supported board](../../../README.md#supported-boards)**, take the `BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_1_85` board as an example, follow the steps:

    - Copy a supported board file in the [boards](./boards/) directory, which has the same chip as your board, for example, the `BOARD_ESPRESSIF_ESP32_S3_LCD_EV_BOARD_2_V1_5.json` file. Then paste it to the [boards](./boards/) directory and rename it to `BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_1_85.json`. Modify the content as needed.
    - Add a new board env in the *platformio.ini* file as follows:

      ```ini
      ...
      [env:BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_1_85]
      build_flags =
          ${common.build_flags}
          ${spi_qspi_lcd.build_flags}   ; or ${rgb_mipi_lcd.build_flags}
          -DESP_PANEL_BOARD_DEFAULT_USE_SUPPORTED=1
          -DBOARD_M5STACK_M5CORE2
      board = BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_1_85
      ...
      ```

    - Add the board env to the `[platformio]:default_envs` as follows:

      ```ini
      ...
      default_envs =
        ...
        ; BOARD_ESPRESSIF_ESP32_S3_USB_OTG
        ; BOARD_ESPRESSIF_ESP32_P4_FUNCTION_EV_BOARD
        ...
        BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_1_85
      ...
      ```

  - **If using a custom board**, follow the steps:

    - Modify the [BOARD_CUSTOM.json](./boards/BOARD_CUSTOM.json) board file by referring to a supported board file which has the same chip as your board.
    - Modify the `[env:BOARD_CUSTOM]` board env in the *platformio.ini* file as needed
    - Modify the *esp_panel_board_custom_conf.h* file and set `ESP_PANEL_BOARD_DEFAULT_USE_CUSTOM` to `1`. Then change other configurations as needed in the file

  - See [PlatformIO Docs](https://docs.platformio.org/en/latest/projectconf/index.html) for more information

### Step 3. Configure the example

- [Optional] Edit the macro definitions in the [lvgl_v8_port.h](./src/lvgl_v8_port.h) file

  - **If using `RGB/MIPI-DSI` interface**, change the `LVGL_PORT_AVOID_TEARING_MODE` macro definition to `1`/`2`/`3` to enable the avoid tearing function. After that, change the `LVGL_PORT_ROTATION_DEGREE` macro definition to the target rotation degree
  - **If using other interfaces**, please don't modify the `LVGL_PORT_AVOID_TEARING_MODE` and `LVGL_PORT_ROTATION_DEGREE` macro definitions

### Step 4. Compile and upload the project

- Connect the board to your computer
- Click the `upload` button

### Step 5. Check the serial output

- Open the serial monitor
- Check the output logs

## Serial Output

The following are the logs output when using the `Espressif:ESP32_S3_LCD_EV_BOARD_2_V1_5` development board. The logs content may vary with different development boards or different configurations, and it is provided for reference only.

```bash
...
Initializing board
[I][Panel][esp_panel_board.cpp:0066](init): Initializing board (Espressif:ESP32_S3_LCD_EV_BOARD_2_V1_5)
[I][Panel][esp_panel_board.cpp:0235](init): Board initialize success
[I][Panel][esp_panel_board.cpp:0253](begin): Beginning board (Espressif:ESP32_S3_LCD_EV_BOARD_2_V1_5)
[I][Panel][esp_lcd_touch_gt1151.c:0050](esp_lcd_touch_new_i2c_gt1151): version: 1.0.5
[I][Panel][esp_lcd_touch_gt1151.c:0234](read_product_id): IC version: GT1158_000101(Patch)_0102(Mask)_00(SensorID)
[I][Panel][esp_panel_board.cpp:0459](begin): Board begin success
Initializing LVGL
[I][LvPort][lvgl_v8_port.cpp:0769](lvgl_port_init): Initializing LVGL display driver
Creating UI
IDLE loop
IDLE loop
...
```

## Troubleshooting

Please check the [FAQ](../../../README.md#faq) first to see if the same question exists. If not, please create a [Github Issue](https://github.com/esp-arduino-libs/ESP32_Display_Panel/issues). We will get back to you as soon as possible.
