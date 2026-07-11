# ROMEOS 69 — Alpha rev A (μητρική καυστήρα)

Firmware PlatformIO για την **μητρική πλακέτα Alpha** στον καυστήρα.

Συνδέεται με:

| Σύνδεση | Πρωτόκολλο | Σημειώσεις |
|---------|-----------|------------|
| **Viewe 7″ panel** (λεβητοστάσι) | **CN_PANEL** UART2 | `TX=25`, `RX=33`, 115200 · **5V_AUX** |
| **Beta** (outdoor) | UART1 | `TX=17`, `RX=16`, 115200 · **κυρίως ρελέ HP** |
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
- **GPIO34:** ελεύθερο (δεν χρησιμοποιείται για flow sensor)

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

## Outdoor θερμοκρασία / υγρασία (SHT40 · I2C)

**CN5** 5P → **QwiicBus EndPoint** onboard · **SHT40/41** μέσω 2ου EndPoint + Cat5.

| CN5 pin | Net | ESP32 |
|---------|-----|-------|
| 1 GND | GND | H1-1 / H2-14 |
| 2 3V3 | 3V3 | **H2-1** |
| 3 SDA | I2C_SDA | **H1-6** (GPIO21) |
| 4 SCL | I2C_SCL | **H1-3** (GPIO22) |
| 5 EN | **3V3** | **H2-1** (ίδιο με pin 2) |

Κοινό I2C bus με **DS3231** RTC.

**Σημείωση:** DS3231 (U1) και CN5 **μοιράζονται** τα ίδια `I2C_SDA`/`I2C_SCL` — **σωστό** (multi-device bus). Διευθύνσεις: DS3231 **0x68** · SHT40 **0x44**.

## Ροή νερού — **όχι CN7 / CN8**

- **CN7 / CN8 / FLOW_SIG / GPIO34:** **αφαιρούνται** από rev A
- **Μεγάλος inverter κυκλοφορητής:** χειροκίνητες σκάλες · η επιλογή **μένει** μετά διακοπή ρεύματος
- **COP / HP tuning:** σταθερή σκάλα κυκλοφορητή + **ΔT** (2× DS18 στον εναλλάκτη HP στο bus GPIO4)
- ROMEOS **παρακολουθεί** θερμοκρασίες και kWh — **δεν** ρυθμίζει ταχύτητα κυκλοφορητή

## Απόψυξη HP — **CN_DEFROST**

**Μεταφορά από Beta στην Alpha** (η Beta μένει κυρίως για ρελέ).

| CN_DEFROST pin | Net | ESP32 |
|----------------|-----|-------|
| 1 3V3 | 3V3 | H2-1 |
| 2 SING | **DEFROST_SIG** | **GPIO14** (H2-12) |
| 3 GND | GND | H1-1 / H2-14 |

- **Module 230 V → opto** στο πεδίο (πηνίο τριόδης βάνας εξωτερικής HP)
- **R9 4,7 kΩ** pull-up στο PCB
- Λογική: defrost ενεργό → K2 OFF · K4 απομόνωση ηλιακού · K3 ON

## CN_PANEL — Viewe 7″ (λεβητοστάσι)

| CN_PANEL pin | Net | ESP32 |
|--------------|-----|-------|
| 1 GND | GND | H1-13 / H2-14 |
| 2 5V | **5V_AUX** | μετά F2 1,5 A |
| 3 TX | **PANEL_TX** | **GPIO25** (H2-9) |
| 4 RX | **PANEL_RX** | **GPIO33** (H2-8) |

- **UART2** · **115200** · crossover: Alpha TX→Viewe RX, Alpha RX←Viewe TX
- **~500 mA** peak · Wi‑Fi UDP προς panel μπορεί να συνυπάρχει (backup)

## Ρελέ K1–K6

| Ρελέ | GPIO (TBD schematic) | Λειτουργία (προτεινόμενη) |
|------|---------------------|---------------------------|
| K1 | 32 | TBD (μέχρι κλείδωμα ρελέ) |
| K2 | 26 | Μικρός κυκλοφορητής / buffer (defrost → OFF) |
| K3 | 27 | Κύριος κυκλοφορητής (defrost → ON) |
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
