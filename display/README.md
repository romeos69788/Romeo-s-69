# Display — δύο ρόλοι Viewe 7″

Firmware για **Viewe UEDX80480070E-WB-A** (800×480, ESP32-S3).

| Ρόλος | Ποια οθόνη | Link | UX |
|-------|------------|------|-----|
| **Alpha panel** | **Παλιά** Viewe | **CN_PANEL** UART GPIO25/33 | **Hub 6 πλαισίων** — [`PANEL-HUB-6-TILES-LOCK`](../docs/PANEL-HUB-6-TILES-LOCK-2026-07-19.md) |
| **Room thermostat** | **Νέα** Viewe | Wi‑Fi / ESP‑NOW | setpoint · φώτα · ρολά · αργότερα |

Αρχιτεκτονική: [`DISPLAY-TWO-ROLES-2026-07-19.md`](../docs/DISPLAY-TWO-ROLES-2026-07-19.md)

---

## Panel Alpha — hub (κλειδωμένο)

```
1 Νερό · 2 Αντλία · 3 Έξοδοι
4 Μπόιλερ/διαχωριστής · 5 Σύστημα · 6 Συνδέσεις
```

Κάθε πλαίσιο → detail οθόνη · BACK στην κύρια.

### CN_PANEL

| Pin | Net | Alpha |
|-----|-----|-------|
| 1 GND | GND | — |
| 2 5V | 5V | — |
| 3 TX | PANEL_TX | GPIO25 |
| 4 RX | PANEL_RX | GPIO33 |

### Κατάσταση

- [x] Ρόλος panel vs room
- [x] Hub 6 πλαισίων κλειδωμένο
- [ ] Περιεχόμενο detail οθονών (συζήτηση)
- [ ] EEZ UI από την αρχή (χωρίς παλιό room UI)
- [ ] PlatformIO scaffold `display/`
- [ ] UART protocol με Alpha

---

## Remote

MQTT → [`../shared/romeos-remote/`](../shared/romeos-remote/)
