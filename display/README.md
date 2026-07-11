# Display — Viewe 7″ (CN_PANEL)

Firmware για **Viewe UEDX80480070E-WB-A** (800×480, ESP32-S3) — τοπικός πίνακας **λεβητοστασίου**.

## Σύνδεση με Alpha (CN_PANEL — κλειδωμένο)

| CN_PANEL pin | Net | Alpha ESP32 |
|--------------|-----|-------------|
| 1 GND | GND | — |
| 2 5V | 5V_AUX | μετά F2 |
| 3 TX | PANEL_TX | **GPIO25** (H2-9) |
| 4 RX | PANEL_RX | **GPIO33** (H2-8) |

- **UART** 115200 8N1 · **crossover:** Alpha TX → Viewe RX · Alpha RX ← Viewe TX
- **Τροφοδοσία:** 5V_AUX από Alpha (~500 mA peak)

## Wi‑Fi (προαιρετικό / παράλληλο)

Υπάρχον firmware μπορεί να χρησιμοποιεί **UDP/ESP‑NOW** προς Alpha — το **CN_PANEL** είναι ενσύρματο link στο κουτί λεβητοστασίου.

## Κατάσταση

- [ ] PlatformIO project (`display/`)
- [ ] UART listener on Viewe (pins TBD στο Viewe J2 / expansion)
- [ ] Συμβατότητα με `shared/` link types + MQTT remote

## Remote (εκτός σπιτιού)

Setpoint μέσω MQTT → [`../shared/romeos-remote/`](../shared/romeos-remote/)
