# Bring-up checklist — σύστημα θέρμανσης

**Φάση:** hardware + link · **όχι** τελική λογική θέρμανσης.

---

## A — Μητρική (χωρίς 230 V στα ρελέ)

| # | Έλεγχος | OK |
|---|---------|-----|
| A1 | Τροφοδοσία 12 V → buck → ~5 V | |
| A2 | Serial COM7 115200 → `[boot]` + `[tick]` | |
| A3 | **LCD 2004** — 4 γραμμές, I2C 0x27, 5V | |
| A4 | **DHT CN5** — `[dht] T=... RH=...` στο Serial | |
| A5 | Ρελέ K1–K6 LED (bench ON — προσωρινό) | |
| A6 | Buzzer σιωπηλό σε κανονική κατάσταση | |
| A7 | **Buzzer boot** — 3 μπιπ με 12 V ON | ✓ |
| A8 | **RTC U9** — `I2C SCAN` 0x68, LCD γρ.4 ώρα | ✓ |

### LCD — τι σημαίνει (προσωρινό bring-up)

| Γραμμή | Τώρα (δοκιμή) | Μετά (τελικό — θα ορίσουμε) |
|--------|----------------|------------------------------|
| 1 | `OUTDOOR SENSOR LIVE` / `CN5 DHT waiting...` | Status ή ALARM |
| 2 | Θερμοκρασία + υγρασία DHT | Μήνυμα βλάβης ή πεδίο 2 |
| 3 | `CN5 DHT -> Viewe 7in` | … |
| 4 | `LCD2004 bring-up OK` | `SYSTEM OFF` / κατάσταση |

**Σε βλάβη (μελλοντικά):** buzzer ON + LCD π.χ. `ALARM: RELAY K1 FAULT`.

---

## B — Θερμοστάτης 7″ (v10)

| # | Έλεγχος | OK |
|---|---------|-----|
| B1 | UI v10 — PRYMARY, MENU | |
| B2 | Wi‑Fi `MERCUSYS_Romeos` · «Σπίτι» γαλάζιο | |
| B3 | SHT — εσωτερική T / RH | |
| B4 | Εξωτερική T από μητρική (όχι `--`) | |
| B5 | «Λεβήτα» / link UDP πράσινο όταν μητρική online | |
| B6 | Setpoint +/- (UDP προς μητρική) | |

---

## C — Link μητρική ↔ 7″

- Και τα δύο στο **ίδιο Wi‑Fi**
- Μητρική IP από Serial: `[wifi] STA home OK ip=...`
- Αν link αποτυγχάνει: `ROMEOS_MB_LAN_IP` στην οθόνη (build ή docs)

---

## D — Solar PT100 (MAX31865) — **ΕΚΚΡΕΜΕΙ (paused)**

Λεπτομέρειες: [`SESSION-HANDOFF_SOLAR-PT100_2026-06-08.md`](SESSION-HANDOFF_SOLAR-PT100_2026-06-08.md)

| # | Έλεγχος | OK |
|---|---------|-----|
| D1 | Probe multimeter white–red **~126 Ω** | ✓ |
| D2 | CN5 DHT + CN4 DS18 live | ✓ |
| D3 | Firmware `mb_solar_rtd` flash COM7 | ✓ |
| D4 | 2-wire καλωδίωση (2×red RTD+, white RTD−) | ✓ |
| D5 | Bench ένδειξη ~25 °C (μερικές φορές) | ~ |
| D6 | Αντίδραση χεριού / αναπτήρα + σταθερότητα | **✗ εκκρεμεί** |
| D7 | `SOLAR DIAG` — σταθερό R, fault=0 | **✗ εκκρεμεί** |

**Τελευταία δοκιμή:** κολλημένοι **24,1 °C**, χωρίς αντίδραση θέρμανσης. **Δεν αλλάζουμε hardware** — Solar **paused** · mic **paused** (2026-06-28).

---

## E — INMP441 μητρική — **PAUSED** (2026-06-08)

Λεπτομέρειες: [`SESSION-HANDOFF_MIC-MB-PAUSED_2026-06-08.md`](SESSION-HANDOFF_MIC-MB-PAUSED_2026-06-08.md)

| # | Έλεγχος | OK |
|---|---------|-----|
| E1 | Καλωδίωση CN1 + L/R pin 6 | ✓ |
| E2 | 3.29 V CN1 | ✓ |
| E3 | Firmware I2S (7 GPIO combos) | ✓ sw |
| E4 | Ήχος `peak>1` | **✗** — PCB/CN1 ύποπτο |

---

## E2 — INMP441 οθόνη 7″ — **PAUSED** (2026-06-28)

Handoff: [`SESSION-HANDOFF_MIC-DISPLAY-PAUSED_2026-06-28.md`](SESSION-HANDOFF_MIC-DISPLAY-PAUSED_2026-06-28.md)

| # | Έλεγχος | OK |
|---|---------|-----|
| E2.1 | Module → J2 (18/13/10) | ✓ |
| E2.2 | I2S Serial MIC HEAR | ✓ |
| E2.3 | «μενού» → SCREEN_MENU | **✗** paused |
| E2.4 | «θέρμανση» → SCREEN_HEAT | **✗** paused |

---

## F — Αργότερα (όχι τώρα)

- 230 V ρελέ + φορτία
- Flow U6
- Πλήρης λογική K1–K6
- Τελικό layout γραμμών LCD §7.6.1 design notes
