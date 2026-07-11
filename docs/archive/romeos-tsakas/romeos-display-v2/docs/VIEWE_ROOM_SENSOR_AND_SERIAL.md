# Viewe — ROOM (SHT/AHT), Serial (COM4 / CH340), μητρική fallback

Τελευταία ενημέρωση handoff: **2026-04-26**. Σκοπός: να συνεχίσεις από εδώ χωρίς να ψάχνεις το chat.

## Serial στο CH340 (COM4)

- Στο board JSON και στο env Viewe: **`ARDUINO_USB_MODE=0`**, **`ARDUINO_USB_CDC_ON_BOOT=0`** ώστε το `Serial` να πάει στο **UART0 → CH340**, όχι στο native USB CDC.
- Στο `platformio.ini` (env Viewe): `upload_port` / `monitor_port` = **COM4** (προσαρμόζεις αν αλλάξει Windows COM).
- **`monitor_rts = 0`**, **`monitor_dtr = 0`**: μερικά CH340 «σιωπούν» το monitor αν το PIO πειράζει RTS/DTR — μετά **reset (EN)** στο board.
- Monitor: **115200 8N1**. Αν δεν βλέπεις boot log, άνοιξε monitor και πάτα **EN**.

## ROOM — ποιο I2C και ποια chips

- **Προτεινόμενο wiring (ίδιο bus με GT911 touch):** **SDA = GPIO19**, **SCL = GPIO20**, **3V3**, **GND** (παράλληλα στο touch I2C). Ορίζεται με:
  - `-DROMEOS_ROOM_I2C_SDA=19`
  - `-DROMEOS_ROOM_I2C_SCL=20`
- **Δεν** χρησιμοποιούμε GPIO18/38 (GT911 INT/RST). SPI SD 10–13: όχι για SHT.

### Firmware — τι υποστηρίζεται στο I2C0 (touch bus)

| Chip / module        | Διεύθυνση I2C | Αρχείο / σημείωση                                      |
|---------------------|----------------|--------------------------------------------------------|
| SHT4x (SHT40/41/45) | 0x44 (0x45)    | `src/romeos_room_sht_touch_bus.cpp` — probe SHT4x πρώτα |
| SHT3x (SHT30/31/35) | 0x44 / 0x45    | ίδιο αρχείο, μετά το SHT4x                             |
| AHT20 / AHT21       | 0x38           | fallback αν αποτύχουν SHT4x/SHT3x στα 0x44/0x45        |

- Init: `src/romeos_room_temp_sensor.cpp` — `romeos_room_temp_sensor_after_board_begin()` μετά το `Board::begin()`, scan `[i2c0]`, **`romeos_room_sht_touch_bus_begin_auto()`**.
- Poll ~2 s: `romeos_room_temp_sensor_poll_in_app_loop()` από `loop()`.
- Αν **δεν** βρέθηκε αισθητήρας στο boot: **ξανά-ανίχνευση κάθε 45 s** (late attach / χαλαρή επαφή). Serial: `[room_temp] late I2C detect OK ...`.

### Τι **δεν** είναι drop-in στο 19/20 με τον τρέχοντα κώδικα ROOM

- **DS18B20** (One-Wire), **NTC νερού** (αναλογικό), **PT100 / MAX31865**: άλλο πρωτόκολλο/pin — χρειάζεται νέος κώδικας και σχεδιασμός pin, όχι απλή αλλαγή module στο I2C touch.

## Διάγνωση από Serial

Μετά το boot:

1. `[i2c0] scan ...` — αν βλέπεις **μόνο `0x5D` (GT911)** και **όχι** `0x44` / `0x45` / `0x38`, το ESP **δεν βλέπει** τον αισθητήρα στο bus (καλώδιο, τάση, λάθος pad, SDA/SCL αντεστραμμένα).
2. `[room_temp] family=SHT4x|AHT2x|... ready` ή `NOT FOUND`.
3. One-shot: **`[room_temp] ROOM από μητρική (room_display)...`** όταν ενεργό το fallback (βλ. παρακάτω).

## Fallback ROOM από μητρική (χωρίς τοπικό I2C αισθητήρα)

- Στο `src/main.cpp`: αν **`ROMEOS_ROOM_USE_MB_FALLBACK`** = 1 (default), **έγκυρη UDP τηλεμετρία** και **όχι** demo δεξιά (`mb_proto_reserved` bit0 = 0), η γωνία ROOM δείχνει **`room_display_c_x10`** από το πακέτο μητρικής.
- Απενεργοποίηση: `-DROMEOS_ROOM_USE_MB_FALLBACK=0` στο `platformio.ini`.
- Το **οθόνη → μητρική** `room_from_display` παραμένει **INVALID** όσο δεν υπάρχει τοπικός SHT — δεν «λέμε ψέματα» στη μητρική.

## Σχετικά αρχεία (γρήγορη λίστα)

| Αρχείο | Ρόλος |
|--------|--------|
| `platformio.ini` | env Viewe: COM, `ARDUINO_USB_*`, `ROMEOS_ROOM_*`, `ROMEOS_MB_LAN_IP`, `monitor_rts/dtr` |
| `boards/BOARD_VIEWE_UEDX80480070E_WB_A.json` | `ARDUINO_USB_MODE`, `ARDUINO_USB_CDC_ON_BOOT` |
| `src/board_override/viewe/BOARD_VIEWE_UEDX80480070E_WB_A.h` | Touch I2C: host 0, SDA 19, SCL 20, 100 kHz |
| `src/romeos_room_temp_sensor.cpp` / `.h` | Init, poll, UI callback |
| `src/romeos_room_sht_touch_bus.cpp` / `.h` | SHT3x/SHT4x/AHT, `begin_auto()` |
| `src/main.cpp` | UI ROOM/OUT, `on_mb_from_link`, `room_corner_ui_has_measurement()`, MB fallback |

## Επόμενα βήματα (hardware)

1. Breakout **SHT4x** (VIN / GND / SCL / SDA): **SDA→GPIO19**, **SCL→GPIO20**, **3V3 στο VIN** (αν το board το δέχεται — έλεγξε datasheet του breakout), **GND κοινό**.
2. Επιβεβαίωση στο Serial: εμφάνιση **`ACK 0x44`** (ή `0x38` για AHT) στο `[i2c0]` scan μαζί με `0x5D`.
