# ROMEOS 69 — Alpha rev A (μητρική καυστήρα)

Firmware PlatformIO για την **μητρική πλακέτα Alpha** στον καυστήρα.

Συνδέεται με:

| Σύνδεση | Πρωτόκολλο | Σημειώσεις |
|---------|-----------|------------|
| **Beta** (εξωτερικό/αίθριο) | UART | `TX=17`, `RX=16`, 115200 |
| **Οθόνες θερμοστάτη** | UART / ESP-NOW | `TX=25`, `RX=33` |
| **HP outdoor board** | RS485 | **Όχι στο rev A** · future `MAX485EPA` |
| **iPhone remote** | MQTT (HiveMQ) | [`shared/romeos-remote/`](../shared/romeos-remote/) |
| **Boiler panel meter** | TUYA WiFi | Ξεχωριστή συσκευή · όχι CT |
| **HP panel meter** | WiFi ψηφιακός | kWh αντλίας · **όχι CT στη μητρική** |

## Ενέργεια / ρεύμα HP

**Όχι CT** (SCT-013) στη μητρική Alpha rev A.

- **kWh αντλίας:** WiFi ψηφιακός μετρητής στον πίνακα της HP (ξεχωριστή συσκευή)
- **kWh καυστήρα / 4 kW:** TUYA WiFi μετρητές πίνακα (ξεχωριστά)
- **GPIO35:** ελεύθερο (δεν χρησιμοποιείται για CT)

## Θερμοκρασία νερού (DS18B20)

**Κλέμες CN3, CN4, CN10** — κοινό **1-Wire** `DS18_DATA` → **GPIO4** (H1 pin 13).

| Κλέμα | Max probes | Σημείωση |
|-------|------------|----------|
| CN3 / CN4 / CN10 | 3 έκαστη | Pull-up **4,7 kΩ μόνο στο CN10** · **100 nF** ανά κλέμα |

## Ηλιακός — **όχι CN6**

- **CN6 / MAX31865 / PT100:** **αφαιρούνται** από rev A
- **T ηλιακού:** DS18B20 επιστροφής (~2 m) στο bus νερού · firmware ROM **SOLAR-RETURN**
- **Κύκλωμα πεδίου:** ξεχωριστό κλειστό κύκλωμα (δοχείο διαστολής, PRV, μικρός κυκλοφορητής ανακυκλοφορίας)
- **Interlock:** τριόδη ανοιχτή → μικρός solar pump **OFF** · μεγάλος inverter **ON**

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

### Build flags

Ρύθμισε credentials στο `secrets/platformio.local.ini` (δες `secrets/README.md`) ή απευθείας:

```ini
-DROMEOS_MQTT_HOST=\"xxxx.s1.eu.hivemq.cloud\"
...
```

## Build

```powershell
cd D:\ROMEOS 69\alpha
pio run -e alpha_rev_a_bench    # compile χωρίς broker
pio run -e alpha_rev_a            # production (με secrets)
pio run -t upload -e alpha_rev_a
pio device monitor
```

## TODO (επόμενα βήματα)

- [ ] Ανάθεση ROM DS18 (συμπ. **SOLAR-RETURN** στο bus GPIO4)
- [ ] UART πρωτόκολλο Alpha ↔ Beta
- [ ] ESP-NOW / UDP sync setpoint προς οθόνες
- [ ] TUYA / WiFi meter integration (HP kWh + καυστήρα)
- [ ] Λογική ελέγχου (pumps, HP, defrost)
