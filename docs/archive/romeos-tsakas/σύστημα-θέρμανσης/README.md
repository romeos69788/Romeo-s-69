# Σύστημα θέρμανσης — Romeos Tsakas

**Κύριος φάκελος εργασίας από 2026-06-27.**  
Όλα τα υπόλοιπα projects στο `Romeos Tsakas` (v2–v6, `thermostat-ui-demo`, κ.λπ.) είναι **ιστορικό / πειραματικό** μέχρι να φτάσαμε εδώ.

---

## Τι περιέχει

| Φάκελος | Ρόλος | Upload |
|---------|--------|--------|
| **`motherboard-fw/`** | Firmware **μητρικής** CONTROL BOARD v1.0 (ESP32, COM7 CP210x) | `pio run -e esp32_motherboard -t upload --upload-port COM7` |
| **`romeos-display-v10/romeos-display-v10/`** | Firmware **θερμοστάτη 7″** Viewe (ESP32-S3, COM3 CH340) · EEZ UI v10 | `pio run -e BOARD_VIEWE_V10 -t upload --upload-port COM3` |
| **`shared/`** | Κοινό πρωτόκολλο Wi‑Fi/UDP, NVS, `romeos_link_types.h` | — (include από τα δύο projects) |

**Δεν** αντιγράφηκαν: παλιά demos, HP board, remote web UI — μόνο ό,τι χρειάζεται **τώρα** για bring-up.

---

## Αρχιτεκτονική (3 «οθόνες» — μην μπερδεύονται)

```
┌─────────────────────────────────────────────────────────────┐
│  ΜΗΤΡΙΚΗ (ESP32) — λεβητοστάσιο                            │
│  · LCD 2004 I2C (4 γραμμές) ─── ΜΟΝΟ εδώ, καλώδιο I2C       │
│  · DHT εξωτερικού CN5, ρελέ K1–K6, buzzer συναγερμού       │
│  · Wi‑Fi UDP → θερμοστάτη 7″                                │
└──────────────────────────┬──────────────────────────────────┘
                           │ Wi‑Fi (ίδιο LAN, UDP 9000/9001)
┌──────────────────────────▼──────────────────────────────────┐
│  ΘΕΡΜΟΣΤΑΤΗΣ 7″ (ESP32-S3) — χώρος (Viewe v10)              │
│  · UI EEZ: PRYMARY, MENU, HEAT, BOILER, …                    │
│  · SHT εσωτερική θερμοκρασία/υγρασία (στην οθόνη)           │
│  · Εξωτερική θερμοκρασία ← UDP από μητρική (DHT CN5)        │
└─────────────────────────────────────────────────────────────┘
```

### LCD 2004 (4 γραμμές) — **αποκλειστικά μητρική**

- **Όχι** η οθόνη 7″ · **όχι** firmware οθόνης.
- Συνδέεται στο κλέμα **LCD-2004/I2C** της πλακέτας (SCL, SDA, 5V, GND).
- **Σκοπός (κλειδωμένη πολιτική):**
  1. **Κανονική λειτουργία:** status / διαγνωστικά στο λεβητοστάσιο — **τι ακριβώς θα γράφει κάθε γραμμή θα το ορίσουμε αργότερα** (όχι τελικό κείμενο ακόμα).
  2. **Βλάβη:** buzzer μητρικής + **κείμενο βλάβης στο LCD** (π.χ. «ΒΛΑΒΗ: K1 ρελέ»). Η 7″ μπορεί να δείχνει alarm παράλληλα, αλλά το **LCD είναι η τοπική ένδειξη λεβητοστασίου**.

Τρέχον bring-up κείμενο (προσωρινό): DHT CN5 + «OUTDOOR SENSOR LIVE» — **μόνο για δοκιμή**, όχι τελικό layout.

---

## Φάση τώρα: **BRING-UP** (έλεγχος hardware)

**Όχι** πλήρης λογική θέρμανσης ακόμα — μόνο επιβεβαίωση ότι:

- [ ] Μητρική τροφοδοτείται, ESP32 boot, Serial COM7
- [ ] LCD 2004 ανάβει και ενημερώνεται
- [ ] DHT CN5 διαβάζεται (Serial + LCD + UDP)
- [ ] Ρελέ K1–K6 (bench — προσωρινά ON στο boot)
- [ ] Θερμοστάτης 7″ v10 UI OK (COM3)
- [ ] **UDP link** μητρική ↔ 7″ (ίδιο Wi‑Fi, εικονίδιο «Λεβήτα» / link)
- [ ] Εσωτερική θερμοκρασία (SHT οθόνης) + εξωτερική (από μητρική)

Μετά bring-up → **φάση λειτουργίας** (κώδικας K1–K6, κατώφλια, αυτόματος έλεγχος, κ.λπ.).

Λεπτομέρειες checklist: [`BRINGUP-CHECKLIST.md`](BRINGUP-CHECKLIST.md)

**Handoff συνεδρίας (2026-06-28):** [`SESSION-HANDOFF_2026-06-28.md`](SESSION-HANDOFF_2026-06-28.md) — Solar **PAUSED** · Mic μητρική **PAUSED** · Mic οθόνη **PAUSED** (I2S OK, φωνή όχι).

**Handoff bench I/O (2026-06-08):** [`SESSION-HANDOFF_2026-06-08-BENCH-IO.md`](SESSION-HANDOFF_2026-06-08-BENCH-IO.md) — **Buzzer OK** · **DS3231 + LCD γραμμή 4 OK** · AC opto bench · build `2026-06-08-lcd-rtc-line4`.

---

## COM ports (PC σου)

| Συσκευή | COM | Chip |
|---------|-----|------|
| Μητρική | **COM7** | CP210x |
| Θερμοστάτης 7″ | **COM3** | CH340 |

Η επικοινωνία **μητρική ↔ 7″** είναι **Wi‑Fi UDP**, όχι καλώδιο μεταξύ COM.

---

## Γρήγορα commands

**Σημείωση build:** το PlatformIO **δεν** compile σωστά μέσα από ελληνικό path (`σύστημα-θέρμανσης`). Χρησιμοποίησε το junction **`sistema-thermansis`** (ίδιος φάκελος, ASCII όνομα):

```powershell
cd "D:\Romeos Tsakas\sistema-thermansis\motherboard-fw"
cd "D:\Romeos Tsakas\sistema-thermansis\romeos-display-v10\romeos-display-v10"
```

**Μητρική:**
```powershell
cd "D:\Romeos Tsakas\sistema-thermansis\motherboard-fw"
pio run -e esp32_motherboard -t upload --upload-port COM7
pio device monitor -p COM7 -b 115200
```

**Θερμοστάτης 7″ (v10 μόνο):**
```powershell
cd "D:\Romeos Tsakas\sistema-thermansis\romeos-display-v10\romeos-display-v10"
pio run -e BOARD_VIEWE_V10 -t upload --upload-port COM3
```

**Μην** flash `thermostat-ui-demo` ή άλλα παλιά projects στην 7″.

---

## Επανέναρξη chat

> «Συνέχισε από `SESSION-HANDOFF_2026-06-08-BENCH-IO.md` στο `sistema-thermansis`»

(ESP-NOW / blackout: `SESSION-HANDOFF_2026-06-28.md`)

---

*Δημιουργήθηκε 2026-06-27 · v10 οθόνης επαναφέρθηκε · DHT + LCD bring-up σε εξέλιξη.*
