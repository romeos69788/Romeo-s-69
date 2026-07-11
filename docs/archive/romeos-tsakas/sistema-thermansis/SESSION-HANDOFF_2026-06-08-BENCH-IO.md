# Handoff — bench I/O μητρικής (2026-06-08)

**Φάκελος:** `sistema-thermansis/` (junction ASCII · ίδιο με `σύστημα-θέρμανσης/`)

**Build μητρικής (τελευταίο flash COM7):** `2026-06-08-lcd-rtc-line4`

---

## Τι ολοκληρώθηκε αυτή τη συνεδρία

| Στοιχείο | Αποτέλεσμα | Σημείωση |
|----------|------------|----------|
| **Buzzer U3 (KY-012)** | **OK** | Ήταν **ανάποδα** συνδεδεμένο (SING/VCC/GND). Σωστή σειρά silk: **SING · VCC · GND** (άνω→κάτω). |
| **Boot chime** | **OK** | 3 μπιπ με **12 V ON** (χωρίς USB). Λειτουργία **active-high** (DC, όχι PWM). |
| **DS3231 U9 (ρολόι)** | **OK** | `I2C SCAN` → **0x68**. Μπαταρία CR2032 κρατά ώρα (δοκιμή **2 λεπτά** χωρίς 12 V — συνέχισε). |
| **LCD γραμμή 4** | **OK** | Αντικατέστησε μικρόφωνο: **`DD/MM/YYYY HH:MM:SS`** (ανανέωση ~1 s). |
| **AC opto module (CN2 / βαλβίδα)** | **Δοκιμασμένο bench** | Λάμπα 220 V + διακόπτης → `defrost=` αλλάζει. Θέρμανση/ψύξη → **στο πεδίο**. |
| **ESP-NOW MB↔7″** | **OK** (προηγ. φάση) | Βλ. [`SESSION-HANDOFF_2026-06-28.md`](SESSION-HANDOFF_2026-06-28.md) |

---

## LCD 2004 — τρέχον layout (bring-up)

| Γραμμή | Περιεχόμενο |
|--------|-------------|
| 1 | Εξωτερικό CN5: `EXTERIOR xxC RH xx%` |
| 2 | CN4 DS18: `CN4 DS18 bench xxC` |
| 3 | Ηλιακός PT100: `SOLAR ...` |
| 4 | **Ρολόι U9:** `08/06/2026 15:42:30` ή `RTC U9 --` / `RTC halted` |

Μικρόφωνο: **PAUSED** — δεν εμφανίζεται πλέον στο LCD.

---

## Serial commands μητρικής (COM7, 115200)

### Buzzer
- `BUZZ TEST` — 3 μπιπ
- `BUZZ ON` / `BUZZ OFF` / `BUZZ HOLD` (5 s για πολύμετρο)
- `BUZZ DIAG` — δοκιμή HIGH / LOW / PWM / square wave
- `BUZZ MODE ACTIVE` — KY-012 (προεπιλογή)

### RTC / I2C
- `I2C SCAN` — αναμένεται **0x27** (LCD) + **0x68** (DS3231)
- `RTC READ` — ίδια ώρα με LCD γραμμή 4

### Άλλα (υπάρχοντα)
- `REL HELP`, `DS18 DIAG`, `CN5 DIAG`, `SOLAR DIAG`, κ.λπ.

---

## Flash

```powershell
cd "D:\Romeos Tsakas\sistema-thermansis\motherboard-fw"
pio run -e esp32_motherboard -t upload --upload-port COM7
pio device monitor -p COM7 -b 115200
```

**Οθόνη 7″** (ESP-NOW, χωρίς αλλαγή αυτή τη συνεδρία):

```powershell
cd "D:\Romeos Tsakas\sistema-thermansis\romeos-display-v10\romeos-display-v10"
pio run -e BOARD_VIEWE_V10 -t upload --upload-port COM3
```

---

## Γνωστά θέματα / PAUSED (χωρίς αλλαγή αυτή τη συνεδρία)

| Θέμα | Κατάσταση | Handoff |
|------|-----------|---------|
| Solar PT100 | **PAUSED** | [`SESSION-HANDOFF_SOLAR-PT100_2026-06-08.md`](SESSION-HANDOFF_SOLAR-PT100_2026-06-08.md) |
| Mic μητρική | **PAUSED** | [`SESSION-HANDOFF_MIC-MB-PAUSED_2026-06-08.md`](SESSION-HANDOFF_MIC-MB-PAUSED_2026-06-08.md) |
| Mic οθόνη | **PAUSED** | [`SESSION-HANDOFF_MIC-DISPLAY-PAUSED_2026-06-28.md`](SESSION-HANDOFF_MIC-DISPLAY-PAUSED_2026-06-28.md) |
| DS18 μπόιλερ / ηλιακός | **Δυσαρέσκεια χρήστη** | — |
| DHT εξωτερικού CN5 | **Δυσαρέσκεια** | — |
| Κύκλωμα ρελέ | **Μικρό πρόβλημα PCB** | bench |
| **Ρύθμιση ώρας RTC** | **Εκκρεμεί** | `RTC SET` ή NTP όταν Wi‑Fi — **δεν** μπλοκάρει bring-up |
| Home Wi‑Fi / MQTT ανά συσκευή | **Επόμενη φάση** | ESP-NOW λειτουργεί χωρίς router |

---

## Αρχεία firmware που άλλαξαν

- `motherboard-fw/src/mb_buzzer_rtc.cpp` — buzzer + RTC read + LCD format + Serial
- `motherboard-fw/include/mb_buzzer_rtc.h`
- `motherboard-fw/src/main.cpp` — LCD γραμμή 4, boot chime στο τέλος setup
- `motherboard-fw/platformio.ini` — `ROMEOS_MB_BUZZER_ACTIVE=1`, build id

---

## Επόμενα βήματα (όταν συνεχίσεις)

1. **AC opto / βαλβίδα εκτροπής** — σύνδεση στην εξωτερική μονάδα · κλείδωμα πόλωσης θέρμανση/ψύξη.
2. **RTC SET** ή NTP — σωστή ώρα (προαιρετικό τώρα).
3. **Flow U6**, **ρελέ K1–K6** λογική, **230 V** πεδίο — μετά bench.
4. Αντικατάσταση/βελτίωση **αισθητήρων** (DHT εξωτερικού, DS18 νερού) — ξεχωριστή απόφαση.
5. **Home Wi‑Fi** MB + οθόνη (ξεχωριστά) — χωρίς να σπάσει ESP-NOW.

---

## Επανέναρξη chat

> «Συνέχισε από `SESSION-HANDOFF_2026-06-08-BENCH-IO.md` στο `sistema-thermansis`»

Συνδυασμός με ESP-NOW / blackout: [`SESSION-HANDOFF_2026-06-28.md`](SESSION-HANDOFF_2026-06-28.md)

---

*Τελευταία ενημέρωση: 2026-06-08 — buzzer OK · RTC/LCD γραμμή 4 OK · opto bench procedure documented.*
