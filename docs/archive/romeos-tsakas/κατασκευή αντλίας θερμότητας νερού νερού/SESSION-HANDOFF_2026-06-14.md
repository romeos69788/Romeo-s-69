# Session handoff — HP πλακέτα (σχεδίαση + συζήτηση)

**Τελευταία ενημέρωση:** 2026-06-14 (κλείσιμο session)  
**EasyEDA:** `HP-CONTROL-BOARD-v1.0` · schematic `ROMEOS HP BOARD v1.0`

**Για επανέναρξη:** *«συνέχισε από SESSION-HANDOFF_2026-06-14»*

---

## Τι έγινε σήμερα (ολόκληρη η συζήτηση)

### Α. Σχεδίαση EasyEDA (συνέχεια block-ανά-block)

1. **ESP32 38 pin** — κλείδωμα H1/H2, USB προς εξωτερική άκρη (`ESP32-38PIN-H1-H2-MAP.md`).
2. **Blocks 1–6** — MCP41050+U5, CN4 DS18, U6 flow, CN2 defrost, F1→5V_ESP, LED.
3. **Opto OEM** — PC817 RUN/FAULT (CN_OEM_RUN, CN_OEM_FAULT).
4. **Ρελέ** — RL-K3/K4 heat/cool · RL-K1/K2 pump+servo **και τα δύο → REL_PUMP** (GPIO26).
5. **CN_SERVO**, **MAX485 + CN_BUS**, LED ρελέ/τροφοδοσίας.
6. **Τροφοδοσία** — διόρθωση: **όχι CN_PWR / 12 V στο PCB** · μόνο **CN1 (+5 V)** από εξωτερικά shield + buck (ίδια λογική μητρικής).
7. **Έλεγχος full schematic** (screenshot) — πυρήνας **OK** · λίστα μικρών διορθώσεων πριν ERC (παρακάτω).

### Β. Κείμενα / τεκμηρίωση (αποθήκευση στο φάκελο)

| Αρχείο | Σκοπός |
|--------|--------|
| `HP-BOARD-LEITOURGIA.md` | Τι κάνει η πλακέτα με το κλιματιστικό — δομημένη περιγραφή |
| `HP-BOARD-APLA-GIA-FILO.md` | **Φιλοσοφία λειτουργίας** — για φίλο χωρίς αυτοματισμούς |
| `HP-BOARD-EASYEDA-v1.md` | Ενημερώθηκε Block 1/7 (CN1, όχι 12 V) |
| `HANDOFF_2026-06-08.md` | Ενημερώθηκε §15 + κλέμες + pointers |
| `README.md` | Δείκτης φακέλου |

### Γ. Συμφωνίες που κλειδώθηκαν ξανά

- HP πλακέτα = **φύλακας νερού + μεταφραστής προς OEM** · cascade = **μητρική**.
- Τροφοδοσία HP = **CN1 μόνο 5 V** · 12 V → shield → buck **έξω**.
- Pump + servo power = **ένα** GPIO (`REL_PUMP`) · **δύο** επαφές διπλού ρελέ.
- AC FAN SPOOFER = **ξεχωριστό module** · όχι στο copper HP.

---

## Σχέδιο που έχουμε τώρα (inventory schematic)

### Κεντρικό

| Ref | Ρόλος |
|-----|--------|
| **H1 / H2** | Headers ESP32 DevKit 38 pin |
| **CN1** | Είσοδος **5V_PRE** + GND (από buck module) |

### Τροφοδοσία (μετά CN1)

| Ref | Τιμή / ρόλος |
|-----|----------------|
| C1 | 470 µF / 16 V |
| C2 | 10 µF / 16 V |
| D2 | BZX55C5V6 |
| LED3 + R7 | Ένδειξη 5V_PRE (R7: προτιμ. 1 kΩ) |
| F1 | 1 A → **5V_ESP** → H2-19 |
| F2 | 1,5 A → **5V_AUX** |

### Αισθητήρες / πεδίο

| Ref | Net | Pinout (1→3) |
|-----|-----|----------------|
| **CN4** | DS18B20 | 3V3 · GND · DS18_DATA (+ R10 4k7, C3 100nF) |
| **U6** | Flow Hall | 3V3 · GND · FLOW_SIG |
| **CN2** | DEFROST | 3V3 · DEFROST_SIG · GND (+ R9 4k7) — silk: **DEFROST** (όχι USB) |

### OEM / έλεγχος

| Ref | Ρόλος |
|-----|--------|
| **U1, U3** | PC817 — RUN / FAULT |
| **CN_OEM_RUN** | RUN+, RUN−, GND |
| **CN_OEM_FAULT** | FLT+, FLT−, GND |
| **MCP41050** | NTC spoof (SPI) — ref `U?` → να ονομαστεί (π.χ. U8) |
| **U5** | 2P προς OEM thermistor (AC_POT_W / AC_POT_B) + R8/C4 |
| **RL-K3/K4** | REL_HEAT_CALL · REL_COOL_REV |
| **RL-K1/K2** | REL_PUMP ×2 (pump + servo τροφοδοσία) |

### Έξοδοι / bus

| Ref | Ρόλος |
|-----|--------|
| **CN_SERVO** | 5V_AUX · GND · SERVO_PWM |
| **U4 / MAX485** | RS485 + C7 · CN_BUS A/B/GND · **pin4 = NC** |
| **LED1** | LED_STATUS (GPIO33) |
| **LED5/6/7** | Ένδειξη ρελέ pump / heat / cool |
| **U2 + R22** | LED τροφοδοσίας 5V_AUX |

### Φάση 2 (placeholder)

- **PRESS_DISCH** — H2 pin 6 · χωρίς κλέμα ακόμα.

---

## Blocks — κατάσταση

| Block | Περιεχόμενο | Κατάσταση |
|-------|-------------|-----------|
| 0 | ESP32 H1/H2 | ✅ κλειστό |
| 1/7 | CN1 + F1/F2 + φίλτρο 5V | ✅ κλειστό |
| 1 | MCP41050 + U5 | ✅ κλειστό |
| 2 | CN4 DS18 | ✅ κλειστό |
| 3 | U6 flow | ✅ κλειστό |
| 4 | CN2 defrost | ✅ κλειστό |
| 5 | F1 → 5V_ESP | ✅ κλειστό |
| 6 | LED | ✅ κλειστό |
| — | Opto RUN/FAULT | ✅ στο sheet |
| — | Ρελέ + CN_SERVO | ✅ στο sheet |
| — | MAX485 + CN_BUS | ✅ (διόρθωση pin4) |
| 12 | CN_PRESS | ⏸ φάση 2 |
| 13 | ERC + title block | ⏳ επόμενο |

---

## Διορθώσεις πριν ERC / fab

1. **CN_BUS pin 4** — αφαίρεση 5V → **NC**
2. **CN2 silk** → **DEFROST**
3. **MCP41050** designator `U?` → σταθερό ref
4. **R7** → 1 kΩ (προαιρετικό)
5. **CN_SERVO pin 1** — wire στο **5V_AUX**
6. **H1 pin 3** = NC · UART0 στα pins 4/5
7. **ERC** → Update PCB → silk **CN4** = WATER TEMP · **U6** = FLOW

---

## Επόμενα βήματα

1. Διορθώσεις 1–3 (υποχρεωτικά).
2. ERC + net labels.
3. PCB placement/routing · USB ESP32 προς άκρη.
4. Firmware HP + RS485 πρωτόκολλο (αργότερα).

---

## Αρχιτεκτονική (μία γραμμή)

```
[Οθόνη] ←WiFi→ [Μητρική] ←RS485→ [HP outdoor + schematic αυτό] ↔ [OEM AC] ↔ νερό UFH
```

**Outdoor 12V:** shield → buck → **CN1** (όχι 12V στο HP PCB).

---

## Χρονολόγιο (όλες οι φάσεις session)

- Save As μητρική → HP project · αφαίρεση 230V, LCD, MAX31865, ηλιακός κ.λπ.
- Κλείδωμα pinout · blocks MCP, sensors, power, LED, opto, relays, RS485.
- Διόρθωση τροφοδοσίας (CN1, όχι CN_PWR).
- Review ολόκληρου schematic.
- Αποθήκευση `HP-BOARD-LEITOURGIA.md`.
- Αποθήκευση `HP-BOARD-APLA-GIA-FILO.md` (εξήγηση σε φίλο).
- Τελική ενημέρωση φακέλου + README.

---

*Session κλειστό — καλή συνέχεια όταν ξαναμιλήσουμε.*
