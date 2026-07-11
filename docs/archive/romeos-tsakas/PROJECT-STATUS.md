# Romeos — κατάσταση projects (ενημέρωση 2026-06-27)

## Κύριος φάκελος εργασίας

**`σύστημα-θέρμανσης/`** — από εδώ και πέρα bring-up + firmware:
- `motherboard-fw/` · `romeos-display-v10/` · `shared/`
- README + `BRINGUP-CHECKLIST.md` μέσα στον φάκελο

Τα υπόλοιπα folders (v2–v6, `thermostat-ui-demo`, κ.λπ.) = ιστορικό.

---

## Μητρική πλακέτα — bring-up

- PCB **συναρμολογημένο / έτοιμο**
- **COM7** = μητρική ESP32 (CP210x) · **COM3** = οθόνη v10 (CH340)
- Firmware μητρικής **τρέχει** — Wi‑Fi STA OK (π.χ. 192.168.0.103)
- Handoff: `romeos-display-v4/μητρική πλακέτα/SESSION-BRINGUP_2026-06-25.md`

## Ενεργό firmware οθόνης

**`romeos-display-v10/romeos-display-v10/`** — Viewe 7″ ESP32-S3, EEZ `romeos-display-v10`, env **`BOARD_VIEWE_V10`**, upload **COM3**.

- Οθόνες: PRYMARY, MENU (hub), HEAT, BOILER, LIGHT, BLINDS
- Οικιακό Wi‑Fi: `MERCUSYS_Romeos` (NVS + `secrets/wifi_home.ini`)
- NTP, SHT εσωτερικό, DHT εξωτερικό (UDP μητρική), alarm, MIC toggle (stub φωνής)
- Build banner: `BUILD 2026-05-24-v10-mic-touch`
- Τεκμηρίωση: `romeos-display-v10/romeos-display-v10/README.md`, `docs/HANDOFF_2026-05-24.md`

### Προηγούμενο (σταθερό αλλά αντικαταστάθηκε στο UI)

**`romeos-display-v5/`** — EEZ `othoni_7`, env `BOARD_VIEWE_OTHONI_7`. Κράτα για reference / άλμπουμ backgrounds (`BOARD_VIEWE_BG_PREVIEW`).

## Scripts (ρίζα `scripts/`)

| Script | Σκοπός |
|--------|--------|
| `SET-WIFI-DISPLAY-COM3.bat` | Provisioning home Wi‑Fi στην **οθόνη v5** (COM3) |
| `SET-WIFI-DISPLAY-COM4.bat` | Παλιότερο COM (v2/demo) |
| `SET-WIFI-MOTHERBOARD-COM7.bat` | Μητρική |
| `set-wifi-home.ps1` | Κοινό Serial `WIFI_HOME_SET` |

## Άλλα folders

| Φάκελος | Σημείωση |
|---------|----------|
| `motherboard-fw/` | Firmware μητρικής |
| `shared/` | Κοινός κώδικας (wifi_nvs, κ.λπ.) |
| `romeos-remote/` | Web UI (MQTT) |
| `romeos-display-v5/` | Προηγούμενο UI `othoni_7` · preview album backgrounds |
| `romeos-display-v10/` | **Τρέχον** EEZ UI + MENU hub |
| `romeos-display-v2` … `v4`, `v6` | Παλαιότερα / πειραματικά |
| `thermostat-ui-demo/` | Προηγούμενο demo οθόνης |

## Κοινός κώδικας Wi‑Fi

`shared/include/romeos_wifi_nvs.h` + `shared/src/romeos_wifi_nvs.cpp` — το v5 έχει αντίγραφο στο `src/romeos_wifi_nvs.cpp` με build header από `scripts/wifi_home_build_flags.py`.
