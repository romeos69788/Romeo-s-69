# Display — δύο ρόλοι Viewe 7″

Firmware για **Viewe UEDX80480070E-WB-A** (800×480, ESP32-S3).

**Κλείδωμα ρόλων (2026-07-19):** [`docs/DISPLAY-TWO-ROLES-2026-07-19.md`](../docs/DISPLAY-TWO-ROLES-2026-07-19.md)

| Ρόλος | Ποια οθόνη | Link | Φάκελος / env (στόχος) |
|-------|------------|------|-------------------------|
| **Alpha panel** (λεβητοστάσιο) | **Παλιά** Viewe | **CN_PANEL** UART GPIO25/33 | `display/` · `BOARD_VIEWE_PANEL` |
| **Room thermostat** | **Νέα** Viewe (παραγγελία) | Wi‑Fi UDP / ESP‑NOW | αργότερα · `BOARD_VIEWE_ROOM` |

---

## Οθόνη 1 — Panel Alpha (δουλειά τώρα)

### Σύνδεση CN_PANEL (κλειδωμένο)

| CN_PANEL pin | Net | Alpha ESP32 |
|--------------|-----|-------------|
| 1 GND | GND | — |
| 2 5V | 5V_AUX / 5V_ESP | ~500 mA peak |
| 3 TX | PANEL_TX | **GPIO25** (H2-9) |
| 4 RX | PANEL_RX | **GPIO33** (H2-8) |

- **UART** 115200 8N1 · crossover: Alpha TX → Viewe RX · Alpha RX ← Viewe TX
- Βάση UI: **romeos-display-v10** → μετατροπή σε panel (όχι room setpoint)

### Κατάσταση

- [x] Ρόλος κλειδωμένος (παλιά = panel Alpha)
- [ ] EEZ / UI διαμόρφωση panel
- [ ] PlatformIO scaffold στο `display/`
- [ ] UART pins στο Viewe + protocol με Alpha
- [ ] Εμφάνιση ρελέ Beta (via Alpha)

---

## Οθόνη 2 — Room thermostat (μετά άφιξη)

- Setpoint χώρου · SHT · MENU comfort (HEAT / …)
- Link ασύρματο προς Alpha — **όχι** CN_PANEL
- Βάση: ίδιο v10 με role `ROOM`

---

## Remote (εκτός σπιτιού)

Setpoint / state μέσω MQTT → [`../shared/romeos-remote/`](../shared/romeos-remote/)
