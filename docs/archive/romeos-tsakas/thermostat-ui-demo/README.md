| Supported ESP SoCs | ESP32 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 | ESP32-P4 |
| ------------------ | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

**Δεύτερη οθόνη (νέο project, χωρίς παλιά μητρική):** [`../romeos-display-v2/README.md`](../romeos-display-v2/README.md)

**Συνέχεια Viewe (ROOM / SHT / COM4 / fallback μητρικής — 26/4/2026):** [`../HANDOFF_2026-04-26.md`](../HANDOFF_2026-04-26.md) · **Λεπτομέρειες:** [`docs/VIEWE_ROOM_SENSOR_AND_SERIAL.md`](./docs/VIEWE_ROOM_SENSOR_AND_SERIAL.md)

**Επανεκκίνηση συζήτησης:** [`../HANDOFF_2026-04-20.md`](../HANDOFF_2026-04-20.md) · **Παράδοση / §9 (19/4):** [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) (Wi‑Fi home + failover, NVS, ρολόι/NTP, PlatformIO)· **Οθόνη Love:** [`docs/UI_LOVE_screen_checkpoint.md`](./docs/UI_LOVE_screen_checkpoint.md)

# LVGL (v8) Porting Example

## Thermostat UI (Romeos) — τελικό σχέδιο

Αυτό το project περιέχει το **τελικό UI θερμοστάτη** (800×480, LVGL 8, SquareLine `src/squareline/`) πάνω στο board **Viewe UEDX80480070E-WB-A**. Η λογική οθόνης και ντεμο βρίσκεται κυρίως στο **`src/main.cpp`** (ρολόι/NVS, γωνίες ROOM/OUT, Wi‑Fi τόξα, αντλίες/κυκλοφορητές, θερμοκρασίες δεξιά, boiler ON/OFF, συναγερμός, **δεύτερη οθόνη**).

| Θέμα | Σημείωση |
|------|----------|
| Ενημέρωση θερμοκρασίας δεξιά (Solar / Boiler / Intel / Outlet) | **Κάθε 2 s**, ίδιο με ROOM και OUT (`k_corner_temp_demo_ms`) |
| Δεύτερη οθόνη (`s_scr_love`) | Δημιουργία στο `main.cpp` (`squareline_love_screen_ensure_created`): **7 γραμμές** grid, αριστερά ονόματα ρολών + ▲/▼, δεξιά YPO-1/R + 5 γραμμές φώτων (ON/OFF) + γραμμή **MENU** (← Main / → επόμενη οθόνη). Φόντο `k_love_bg_rgb`, στρογγυλά **65×65**, μπάρες Montserrat **28**, ON/OFF **22**, σύμβολα **28**. Μετάβαση: **δεξί βελάκι** στο `ui_Romeos1` όταν το ρολόι **δεν** είναι σε edit → `MOVE_LEFT`. Λεπτομέρειες: [`docs/UI_LOVE_screen_checkpoint.md`](./docs/UI_LOVE_screen_checkpoint.md). |
| Αρχεία SquareLine | `src/squareline/screens/ui_Romeos1.c`, `ui_Romeos1.h` |
| Build | PlatformIO (`platformio.ini`), env board Viewe |
| **Ενδείξεις Wi‑Fi** (`ui_Romeos1`, `main.cpp`) | **HOME** (`ui_room3`): πράσινο + **παλμός τόξων** όταν STA στο **αποθηκευμένο οικιακό SSID**· κόκκινο στο `RomeosMB` ή χωρίς home creds. **ESP32** (`ui_room4`): πράσινο + παλμός όταν **ζωντανό UDP** με τη μητρική (`romeos_display_link_is_connected()`). HOME = «σπίτι AP», ESP32 = «μητρική». Serial: `[romeos_link] WiFi OK ssid="..."`. |

Για επαναφορά σε αυτή την κατάσταση: `git checkout` στο τελευταίο commit ή tag **`thermostat-final-design`**.

**Συνοπτικές σημειώσεις συζητήσεων (υδραυλικό, μητρική, κουτί, Buzz κ.λπ.):** [docs/romeos-design-notes.md](./docs/romeos-design-notes.md)

### Πού μπαίνει ο κωδικός Wi‑Fi σπιτιού (όχι σε αρχείο)

**Δεν** τον γράφεις στο `platformio.ini` ούτε σε `.cpp` — **μόνο** στη μνήμη του ESP από **Serial** (και στα δύο boards αν θες home και στη μητρική).

1. Άνοιξε **Serial Monitor** στο σωστό project (**οθόνη** = `thermostat-ui-demo`, **μητρική** = `motherboard-fw`) και στο **σωστό COM**· ταχύτητα **115200**· τέλος γραμμής **LF**.
2. Γράψε μία γραμμή και Enter: `WIFI_HOME_SET`
3. Επόμενη γραμμή και Enter: το **SSID** (π.χ. `MERCUSYS_Romeos` — ακριβώς όπως στο router).
4. Επόμενη γραμμή και Enter: ο **κωδικός** (μία γραμμή, χωρίς εισαγωγικά).

Περίμενε `[wifi_nvs] saved`. Μετά **reset** το board. Έλεγχος: `WIFI_HOME_SHOW` (δείχνει SSID, όχι κωδικό)· βοήθεια: `WIFI_HOME_HELP`· διαγραφή: `WIFI_HOME_CLR`.

**Αυτόματα (Windows):** φάκελος [`../scripts/`](../scripts/) — κλείσε Serial Monitor στο Cursor, μετά διπλό κλικ στο **`SET-WIFI-DISPLAY-COM4.bat`** (οθόνη) ή **`SET-WIFI-MOTHERBOARD-COM7.bat`** (μητρική)· θα ζητήσει **μόνο** τον κωδικό. Αν το COM σου είναι άλλο: `powershell -File scripts\set-wifi-home.ps1 -Port COMx`.

### Μετά από flash μητρικής — ενημέρωση οθόνης

Το firmware της **οθόνης** είναι **ξεχωριστό project** (`thermostat-ui-demo`, ESP32‑S3). Αν άλλαξες πρωτόκολλο/link/NVS που αφορά και τα δύο ESP, κάνε **`pio run -t upload`** εδώ στο **COM της οθόνης** (στο `platformio.ini` συνήθως διαφορετικό COM από τη μητρική). Στο LAN ενημέρωσε το **`ROMEOS_MB_LAN_IP`** ώστε να ταιριάζει με την τρέχουσα IP της μητρικής (`shared/include/romeos_link_types.h`).

### Ρολόι & ώρα δικτύου

- **LVGL / ρολόι:** αποφυγή μεγάλης απόκλισης με βάση monotonic (`esp_timer_get_time`) + rebase ώρας — `src/main.cpp` (λπ. `squareline_clock_*`).
- **SNTP / internet:** `src/romeos_ntp.cpp` — ξεκινάει μόνο με **Wi‑Fi συνδεδεμένο** και διαδρομή προς internet (NTP UDP 123 προς `pool.ntp.org` / `time.google.com`). Ζώνη **EET-2 / EEST-3** (Ελλάδα). Σε **RomeosMB-only** συνήθως **δεν** γίνεται sync· το ρολόι βασίζεται σε NVS + monotonic (`main.cpp`). Serial: `[romeos_ntp] wall clock set from NTP` όταν έχει γίνει sync· αν μετά από ~60 s δεν έχει sync, εκτυπώνεται προειδοποίηση.

### Ενδείξεις Wi‑Fi στην κεντρική οθόνη

- **Λογική:** `romeos_display_link_home_wifi_assoc()` στο `romeos_display_link.cpp` (σύγκριση `WiFi.SSID()` με το NVS)· η εφαρμογή στο UI: `squareline_wifi_link_status_timer_cb` στο `main.cpp`.
- **Αν το HOME μένει κόκκινο** ενώ «είσαι στο σπίτι»: έλεγξε ότι το SSID στο `WIFI_HOME_SET` **ταυτίζεται** με το broadcast του AP (π.χ. `_5G` vs χωρίς suffix).
- **Αν το ESP32 μένει κόκκινο** με πράσινο HOME: συνήθως λείπει ή είναι λάθος το **`ROMEOS_MB_LAN_IP`** στο `platformio.ini` της οθόνης, ή η μητρική δεν στέλνει UDP στο LAN.

### Επόμενα βήματα (μητρική)

**Κατάσταση link:** η **επικοινωνία μητρική ↔ οθόνη** (SoftAP `RomeosMB`, UDP, structs στο `shared/`) είναι **ενεργή και επιβεβαιωμένη** στο hardware — βλ. [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) §2.8, §7.

**Συνέχεια firmware:** πραγματική τηλεμετρία (`-DROMEOS_MB_TELEMETRY_OK` στη μητρική, πραγματικές τιμές στο `build_tx()`), λογική K1–K6, αισθητήρες — όπως πίνακας στο `motherboard-fw/README.md`. Η βάση UI παραμένει στο `src/main.cpp` και στο SquareLine export.

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
