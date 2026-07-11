# Handoff συνολικό — σύστημα θέρμανσης (2026-06-28)

**Κύριος φάκελος:** `sistema-thermansis/` (ίδιο περιεχόμενο στο `σύστημα-θέρμανσης/`)

---

## Κατάσταση bring-up

| Θέμα | Κατάσταση | Handoff |
|------|-----------|---------|
| DHT CN5, DS18 CN4, Wi‑Fi/UDP | OK (μετά recovery 2026-07-01) | [`POWER-OUTAGE-RECOVERY.md`](POWER-OUTAGE-RECOVERY.md) |
| Solar PT100 CN2 | **PAUSED** | [`SESSION-HANDOFF_SOLAR-PT100_2026-06-08.md`](SESSION-HANDOFF_SOLAR-PT100_2026-06-08.md) |
| INMP441 **μητρική** | **PAUSED** — pk=1 | [`SESSION-HANDOFF_MIC-MB-PAUSED_2026-06-08.md`](SESSION-HANDOFF_MIC-MB-PAUSED_2026-06-08.md) |
| INMP441 **οθόνη 7″** | **PAUSED** — I2S OK, φωνή όχι | [`SESSION-HANDOFF_MIC-DISPLAY-PAUSED_2026-06-28.md`](SESSION-HANDOFF_MIC-DISPLAY-PAUSED_2026-06-28.md) |
| **Buzzer U3 + RTC U9 + LCD γρ.4** | **OK** (2026-06-08) | [`SESSION-HANDOFF_2026-06-08-BENCH-IO.md`](SESSION-HANDOFF_2026-06-08-BENCH-IO.md) |
| Θερμοστάτης v10 UI | OK | [`romeos-display-v10/romeos-display-v10/README.md`](romeos-display-v10/romeos-display-v10/README.md) |

---

## Συμπέρασμα mic (INMP441)

- **Modules:** πιθανόν **OK** (I2S δεδομένα στην οθόνη, όχι pk=1)
- **Μητρική PCB v1.0:** πιθανό **trace / CN1 / σύγκρουση GPIO26–27 με K2/K3**
- **Φωνητικές εντολές:** **δεν ολοκληρώθηκαν** — χρειάζεται ESP-SR ή ισοδύναμο

---

## COM ports

| Συσκευή | COM | Upload |
|---------|-----|--------|
| Μητρική | COM7 | `pio run -e esp32_motherboard -t upload --upload-port COM7` |
| Οθόνη 7″ | COM3 | `pio run -e BOARD_VIEWE_V10 -t upload --upload-port COM3` |

Checklist: [`BRINGUP-CHECKLIST.md`](BRINGUP-CHECKLIST.md)

---

## Recovery διακοπής ρεύματος (2026-07-01)

**Αιτία:** μετά blackout η μητρική πήρε **192.168.0.101** (όχι .103) → η οθόνη δεν έβρισκε UDP target.

**Διόρθωση (έγινε):**
- `ROMEOS_MB_LAN_IP=192.168.0.101` + flash οθόνης **BUILD 2026-07-01-lan-101-no-mic**
- Flash μητρικής με **`ROMEOS_MB_MIC_ENABLE=0`** (γρηγορότερο boot, χωρίς I2S hunt)
- Οθόνη: boot σταθερό, Wi‑Fi `192.168.0.100`, mic OFF

**Για μόνιμη λύση:** DHCP reservation στο router → μητρική **πάντα .101** (MAC `88:57:21:8e:3b:cc`).

Λεπτομέρειες: [`POWER-OUTAGE-RECOVERY.md`](POWER-OUTAGE-RECOVERY.md)

---

## Link οθόνη ↔ μητρική — ESP-NOW (2026-07-01)

**Νέο transport:** peer **ESP-NOW** (όχι router / LAN UDP).

| Build | Flags |
|-------|--------|
| Οθόνη | `BUILD 2026-07-01-espnow-link`, `ROMEOS_LINK_ESPNOW=1` |
| Μητρική | `ROMEOS_LINK_ESPNOW=1`, `ROMEOS_MB_WIFI_STA=0` |

- Auto-pairing: πρώτο πακέτο → αποθήκευση MAC στο NVS (`mb_mac` / `disp_mac`)
- Home Wi‑Fi / MQTT / Internet: **αργότερα** (ξεχωριστό βήμα)

---

**Τελευταία ενημέρωση:** 2026-06-08 — bench I/O handoff · 2026-07-01 — ESP-NOW link · home Wi‑Fi ανεβαίνει σε επόμενη φάση.
