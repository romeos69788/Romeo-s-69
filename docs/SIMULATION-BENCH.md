# Εικονική δοκιμή Alpha ↔ Beta (αντί για πάγκο;)

**Σύντομη απάντηση:** Δεν υπάρχει πρόγραμμα που να αντικαθιστά πλήρως τον φυσικό πάγκο (CT, ρελέ, ρεύμα, θόρυβο UART). Υπάρχουν όμως εργαλεία για **μέρος** της λογικής πριν το flash στις πλακέτες.

## Τι κάνει ήδη το project

| Περιβάλλον | Τι είναι | Τι δεν είναι |
|------------|----------|--------------|
| `pio run -e alpha_rev_a_bench` | Compile χωρίς MQTT secrets (CI / γρήγορο build) | Προσομοίωση hardware |

## Επιλογές λογισμικού

### 1. Wokwi (προτεινόμενο για λογική + UART)

- **Ιστότοπος:** https://wokwi.com  
- **VS Code / Cursor:** extension *Wokwi Simulator* (+ PlatformIO)  
- **Τι καλύπτει:** 2× ESP32, GPIO, LED, κουμπιά, Serial, **UART μεταξύ δύο chips** (π.χ. Alpha TX17/RX16 ↔ Beta)  
- **Τι δεν καλύπτει ρεαλιστικά:** SCT-013 RMS, πραγματικά ρελέ/φορτία, Wi‑Fi/MQTT σταθερότητα πεδίου, θόρυβο γραμμής  
- **Οδηγός για αρχάριους:** [`WOKWI-START.md`](WOKWI-START.md)  
- **Έτοιμο setup Alpha:** `alpha/diagram.json` + `alpha/wokwi.toml`

Χρήσιμο για: πρωτόκολλο Alpha↔Beta, state machine, εντολές, timeouts — πριν το bench με καλώδια.

### 2. Unit / host tests (χωρίς προσομοιωτή MCU)

- Χωρισμός λογικής (θερμοστάτης, εντολές MQTT, parsing UART) σε συναρτήσεις χωρίς `digitalWrite`  
- Τρέχουν στον PC με Google Test / Unity  
- Καλύτερο για καθαρή λογική· το pin/UART μένει για hardware

### 3. Άλλα (λιγότερο κατάλληλα για εμάς)

| Εργαλείο | Σχόλιο |
|----------|--------|
| Proteus | Πιο schematic/PCB· ESP32 Arduino dual-board δύσκολο |
| QEMU / Espressif emulator | Προχωρημένο· όχι έτοιμο για Arduino + 2 πλακέτες |
| Tinkercad | Όχι πραγματικό ESP32 dual UART όπως το δικό μας |

## Σύσταση για ROMEOS 69

```
PC (compile + Wokwi UART logic)
        ↓
Πάγκος (2 ESP32 ή Alpha+Beta PCB, LED αντί ρελέ, Serial)
        ↓
Πεδίο (CT, TUYA, φορτία)
```

1. **Λογική / UART:** Wokwi ή host unit tests  
2. **I/O πλακετών:** φυσικός πάγκος (όπως τώρα)  
3. **Ισχύς / CT / Wi‑Fi:** μόνο πραγματικό hardware

## Σχετικά pins (rev A)

- Alpha↔Beta UART: **TX=17, RX=16**, 115200  
- CT: **GPIO35** μόνο (SCT-013) — όχι σε πλήρη προσομοίωση  
- RS485 MAX485: **όχι** στο rev A bench

*Τελευταία ενημέρωση: 2026-07-19*
