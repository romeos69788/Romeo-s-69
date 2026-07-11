# ROMEOS 69 — Alpha rev A (μητρική καυστήρα)

Firmware PlatformIO για την **μητρική πλακέτα Alpha** στον καυστήρα.

Συνδέεται με:

| Σύνδεση | Πρωτόκολλο | Σημειώσεις |
|---------|-----------|------------|
| **Beta** (εξωτερικό/αίθριο) | UART | `TX=17`, `RX=16`, 115200 |
| **Οθόνες θερμοστάτη** | UART / ESP-NOW | `TX=25`, `RX=33` |
| **HP outdoor board** | RS485 | **Όχι στο rev A** · future `MAX485EPA` |
| **iPhone remote** | MQTT (HiveMQ) | [romeos-remote](https://github.com/romeos69788/romeos-remote) |
| **Boiler panel meter** | TUYA WiFi | Ξεχωριστή συσκευή · όχι CT |

## CT (ρεύμα)

**Μοναδικό CT** — SCT-013 20A/1V στη γραμμή αντλίας θερμότητας:

- **ADC:** GPIO **35** (`alpha::kCtHpAdc`)
- **Όχι** δεύτερο CT για σύνολο καυστήρα (TUYA meter)
- **Όχι** CT για 4kW κεντρικός πίνακας

## Ρελέ K1–K6

| Ρελέ | GPIO (TBD schematic) | Λειτουργία (προτεινόμενη) |
|------|---------------------|---------------------------|
| K1 | 26 | — |
| K2 | 27 | — |
| K3 | 14 | — |
| K4 | 12 | Heater / boiler |
| K5 | 13 | — |
| K6 | 15 | — |

> Επιβεβαίωσε τα GPIO με το rev A schematic πριν την πρώτη ενεργοποίηση.

## MQTT (συμβατό με romeos-remote)

| Topic | Κατεύθυνση | Περιεχόμενο |
|-------|-----------|-------------|
| `romeos/<DEVICE_ID>/state` | publish | JSON τηλεμετρίας |
| `romeos/<DEVICE_ID>/cmd` | subscribe | `setpoint_c_x10`, `heater` |

### Build flags (platformio.ini)

```ini
-DROMEOS_MQTT_HOST=\"xxxx.s1.eu.hivemq.cloud\"
-DROMEOS_MQTT_USER=\"...\"
-DROMEOS_MQTT_PASS=\"...\"
-DROMEOS_MQTT_DEVICE_ID=\"romeos-mb\"
-DROMEOS_WIFI_SSID=\"...\"
-DROMEOS_WIFI_PASS=\"...\"
```

Μην κάνεις commit κωδικούς στο git.

## Build

```bash
cd motherboard-fw
pio run -e alpha_rev_a_bench    # compile χωρίς broker
pio run -e alpha_rev_a            # production (με build_flags)
pio run -t upload -e alpha_rev_a
pio device monitor
```

## TODO (επόμενα βήματα)

- [ ] DS18B20 / NTC ανάθεση αισθητήρων θερμοκρασίας
- [ ] UART πρωτόκολλο Alpha ↔ Beta
- [ ] ESP-NOW / UDP sync setpoint προς οθόνες
- [ ] CT RMS (SCT-013 calibration)
- [ ] TUYA meter integration (boiler panel)
- [ ] Λογική ελέγχου (pumps, HP, defrost)
