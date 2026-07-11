# shared — μητρική ↔ οθόνη

**Viewe ROOM / Serial / handoff 26/4/2026:** [`../HANDOFF_2026-04-26.md`](../HANDOFF_2026-04-26.md) · [`../thermostat-ui-demo/docs/VIEWE_ROOM_SENSOR_AND_SERIAL.md`](../thermostat-ui-demo/docs/VIEWE_ROOM_SENSOR_AND_SERIAL.md)

**Επανεκκίνηση συζήτησης:** [`../HANDOFF_2026-04-20.md`](../HANDOFF_2026-04-20.md) · **Παράδοση / §9:** [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) (home Wi‑Fi + failover, NVS, `ROMEOS_MB_LAN_IP`, PlatformIO).

## Σκοπός

Κοινά **`#include`** και τεκμηρίωση ώστε:

- **`motherboard-fw/`** (ESP32 μητρική) και  
- **`thermostat-ui-demo/`** (ESP32-S3 + Viewe 800×480, LVGL)

να συμφωνούν σε **διάταξη πλαισίου δεδομένων** και σταθερές **Wi‑Fi/UDP** (ιδιωτικό link, όχι οικιακό router).

### Ζωντανή υλοποίηση (Wi‑Fi + UDP)

| Σταθερά | Τιμή |
|---------|------|
| SSID AP (μητρική) | `ROMEOS_MB_AP_SSID` — προεπιλογή **`RomeosMB`** |
| Κωδικός AP | `ROMEOS_MB_AP_PASS` — προεπιλογή **`romeos2026`** (άλλαξέ τον με `-D` στο `platformio.ini`) |
| UDP οθόνη → μητρική | `ROMEOS_UDP_PORT_MB_LISTEN` (**9000**) |
| UDP μητρική → οθόνη | `ROMEOS_UDP_PORT_DISPLAY_LISTEN` (**9001**) |
| LAN: σταθερή IP μητρικής (οθόνη) | `ROMEOS_MB_LAN_IP` — `-DROMEOS_MB_LAN_IP=\"192.168.x.x\"` **μόνο στην οθόνη** όταν και τα δύο STA στο σπίτι· αν `""`, στόχος UDP = gateway / `192.168.4.1` σε lab |

**Λειτουργία:** και τα δύο firmware μπορούν να συνδεθούν στο **οικιακό Wi‑Fi** (διαπιστευτήρια σε NVS — `romeos_wifi_nvs.h` / `romeos_wifi_nvs.cpp`, εντολές Serial §9.3 στο handoff). **`romeos_wifi_nvs_poll_serial()`** επιστρέφει **`true`** όταν μόλις αποθηκεύτηκαν ή διαγράφηκαν home credentials — η οθόνη (`romeos-display-v2` / `thermostat-ui-demo`) καλεί τότε `romeos_display_link_on_home_wifi_saved_to_nvs()` ώστε να εφαρμοστεί το STA **χωρίς reboot**.

**Fallback RomeosMB:** Στο **`romeos-display-v2`**, αν υπάρχουν home credentials στο NVS, **δεν** γίνεται πλέον αυτόματη εναλλαγή στο **`RomeosMB`** μετά από timeout (ξαναδοκιμάζει μόνο το home SSID). Στο **`thermostat-ui-demo`** / άλλες ροές ισχύει η περιγραφή του handoff (χωρίς NVS home → STA στο `RomeosMB`). Η μητρική ανοίγει SoftAP όπου χρειάζεται. Στο subnet **`192.168.4.x`** η οθόνη στέλνει στο gateway (**192.168.4.1**)· στο LAN, με `ROMEOS_MB_LAN_IP`, προς αυτή την IP.

**Ενδείξεις στην οθόνη (Viewe):** το **HOME** vs **ESP32** στο `ui_Romeos1` εξηγούνται στο [`../thermostat-ui-demo/README.md`](../thermostat-ui-demo/README.md) (ενότητα «Ενδείξεις Wi‑Fi») και στο [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) §9.6.

**Ανταλλαγή ESP32 στη μητρική (εφεδρικό):** ίδιο firmware (`esp32_motherboard`) — το SSID/κωδικός/θύρες UDP παραμένουν· η οθόνη ξανασυνδέεται στο **`RomeosMB`** όταν το νέο module boot-άρει. Λεπτομέρειες: [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) §2.8, [`../motherboard-fw/README.md`](../motherboard-fw/README.md) § Build / upload.

## Αρχεία

| Αρχείο | Περιεχόμενο |
|--------|-------------|
| **`include/romeos_link_types.h`** | `romeos_mb_to_display_v1_t`, `romeos_display_to_mb_v1_t`, θύρες, SSID/AP, `ROMEOS_MB_LAN_IP` |
| **`include/romeos_wifi_nvs.h`** | NVS home Wi‑Fi (`romeos_wifi` / `home_ssid`, `home_pass`) + `romeos_wifi_nvs_poll_serial()` → `bool` (`true` αν μόλις άλλαξαν τα home credentials) |
| **`src/romeos_wifi_nvs.cpp`** | Υλοποίηση· συμπεριλαμβάνεται στο build και των δύο projects μέσω `build_src_filter` στο `platformio.ini` |

## Αντιστοίχιση UI (SquareLine `Romeos1` / `ui_Romeos1`)

| Πεδίο πλαισίου | Widget / λογική UI |
|----------------|---------------------|
| `room_display_c_x10` | `ui_Room` (γωνία), spoof NTC από boiler μέσω MCP — **§12.13** |
| `outdoor_c_x10` | `ui_Out` + ετικέτα OUT |
| `solar_c_x10` | σειρά **Solar** (δεξιά) |
| `boiler_c_x10` | σειρά **Boiler** |
| `supply_c_x10` | **Intel** (supply) |
| `return_c_x10` | **Outlet** (return) |
| `heat_pump_on` … `heater_on` | τέσσερις γραμμές On/Off δεξιά (ντεμο: HP, Pump1, Pump2, Heater) |
| `relay_k1`…`k6` | πραγματικά `REL_K1`…`K6` μητρικής (**§12.4**, **§12.15**) |
| `flow_sig_high`, `defrost_active` | `FLOW_SIG`, `DEFROST_SIG` |

**Δεύτερη οθόνη** (`s_scr_love` — grid ρολών/φώτων/MENU, `main.cpp`): τοπικό UI · **δεν** χρειάζεται πλαίσιο RF για την εμφάνιση· τεκμηρίωση: [`../thermostat-ui-demo/docs/UI_LOVE_screen_checkpoint.md`](../thermostat-ui-demo/docs/UI_LOVE_screen_checkpoint.md).

**Πολιτική βλάβης:** `flags` bit1/bit2 — **§7.12**, **§12.13** (full-screen fault / master shutdown).

## Πού συνδέεται κάθε project

- `motherboard-fw/platformio.ini` → `-I ../shared/include` + `build_src_filter` για `../../shared/src/romeos_wifi_nvs.cpp` (βάση `src/`)
- `thermostat-ui-demo/platformio.ini` → `-I ../shared/include` + ίδιο `build_src_filter` για το `romeos_wifi_nvs.cpp`
- `romeos-display-v2/platformio.ini` → ίδιο pattern (`romeos_wifi_nvs` στο shared)

## Πηγή SquareLine

Ο φάκελος **`../Romeos1/`** είναι export SquareLine (ίδια οθόνη). Το ενσωματωμένο αντίγραφο build βρίσκεται στο **`thermostat-ui-demo/src/squareline/`**.
