# Romeos1 — SquareLine export (οθόνη θερμοστάτη)

**Παράδοση συνεδρίας firmware/UI (2026-04-13):** [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md)

## Τι είναι

Αυτός ο φάκελος είναι **export από SquareLine Studio** (έκδοση **1.6.0**, LVGL **8.3.11**, project `SquareLine_Project.spj` — βλ. `project.info`).

Περιέχει μόνο τα αρχεία UI (**`ui.c` / `ui.h`**, **`screens/ui_Romeos1.*`**, fonts, images, components) — **όχι** πλήρες firmware οθόνης.

## Σχέση με τα άλλα projects

| Διαδρομή | Ρόλος |
|----------|--------|
| **`../thermostat-ui-demo/`** | Πλήρες **PlatformIO** project: **Viewe UEDX80480070E-WB-A**, LVGL, **`src/squareline/`** = αντίγραφο/συγχρονισμός αυτού του UI + **`src/main.cpp`** (ρολόι, ντεμο, μελλοντική RF λήψη). |
| **`../motherboard-fw/`** | Firmware **μητρικής** ESP32 38 pin — αισθητήρες, ρελέ, MCP, MAX. |
| **`../shared/`** | Κοινά structs **`romeos_link_types.h`** για πλαίσια **μητρική → οθόνη** και **οθόνη → μητρική** (setpoint). |

## Τι εμφανίζει η οθόνη (συνοπτικά)

Σύμφωνα με **`thermostat-ui-demo/docs/romeos-design-notes.md` §1**:

- Ανάλυση **800×480**, κύρια οθόνη **`ui_Romeos1`**.
- **Γωνίες:** ROOM (θερμοκρασία χώρου), OUT (εξωτερική).
- **Κέντρο:** setpoint **5…35 °C**, arc, +/- .
- **Δεξιά στήλη:** Solar, Boiler, Intel (supply), Outlet (return) — ενημέρωση θερμοκρασιών (ντεμο **~2 s** στο `main.cpp`).
- **Κατάσταση:** Heat Pump, Pump 1, Pump 2, Heater (On/Off).
- **Ρολόι** 12/24h με NVS, **δεύτερη οθόνη** `s_scr_love` (grid ρολών/φώτων/MENU στο `thermostat-ui-demo/src/main.cpp` — όχι μέρος του SquareLine export· βελάκι δεξιά όταν δεν γίνεται edit ρολογιού): [`../thermostat-ui-demo/docs/UI_LOVE_screen_checkpoint.md`](../thermostat-ui-demo/docs/UI_LOVE_screen_checkpoint.md).
- **Ζώνες χρώματος** θερμοκρασίας (λευκό / γαλάζιο / πορτοκαλί / κόκκινο) όπως στο doc.
- **Μελλοντικά:** ίδια **πολιτική βλαβών** με LCD μητρικής (**§7.12**, **§12.13**).

## Workflow SquareLine

1. Επεξεργασία στο **SquareLine** → Export.
2. Αντιγραφή/συγχρονισμός αρχείων στο **`thermostat-ui-demo/src/squareline/`** (ώστε να build-άρει το Viewe project).

## Σύνδεση με τη μητρική

Η σύνδεση είναι **ασύρματη** (ιδιωτικό RF, **όχι** internet) — **ESP-NOW** ή **SoftAP+STA**, όπως στο doc. Τα δομικά πεδία ορίζονται στο **`../shared/include/romeos_link_types.h`**.
