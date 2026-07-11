# Handoff — Solar PT100 / MAX31865 (μητρική)

**Ημερομηνία τελευταίας ενημέρωσης:** 2026-06-08  
**Κατάσταση:** **ΕΚΚΡΕΜΕΙ** — σταματάμε προσωρινά, συνεχίζουμε με **INMP441 μικρόφωνο**  
**Συνέχισε chat:** *«συνέχισε από SESSION-HANDOFF_SOLAR-PT100_2026-06-08»*

---

## Σύνοψη (μία πρόταση)

**CN5 DHT + CN4 DS18 OK.** Solar PT100: **hardware probe OK (~126 Ω)**, firmware **on board** (`mb_solar_rtd.cpp`), **ένδειξη bench ~25°C μερικές φορές OK** — **εκκρεμεί** σταθερότητα + αντίδραση θέρμανσης (SPI θόρυβος / φίλτρα).

---

## Τι έχει γίνει μέχρι τώρα (~9+ ώρες bench)

### Hardware (επιβεβαιωμένο multimeter, ρελέ OFF)

| Μέτρηση | Αποτέλεσμα | Σημαίνει |
|---------|------------|----------|
| RTD+ ↔ RTD− | **~126 Ω** | Probe + βίδες OK |
| F− ↔ RTD− | **ανοιχτό (όχι beep)** | **Δεν υπάρχει εσωτερική γέφυρα** → software **μόνο 2-wire** |
| CN4 DS18 | **~25–26 °C** | OK |
| CN5 DHT | **~27 °C, ~45 % RH** | OK |

### Καλωδίωση — **ΜΗΝ την αλλάξεις** (κλειδωμένη)

| Στοιχείο | Ρύθμιση |
|----------|---------|
| Jumpers module | **2 Wire ON**, **2/3 Wire OFF** |
| RTD+ | **2× κόκκινα** μαζί |
| RTD− | **Λευκό** |
| F+ / F− | **Κενά** |
| Module | MAX31865 purple clone, **Rref = 430 Ω** |
| SPI | CS=GPIO5, SCK=18, MISO=19, MOSI=23 · MCP41050 CS=GPIO13 idle HIGH |

**3-wire (F+=red, F−=RTD−)** δεν δουλεύει χωρίς **εξωτερικό σύρμα RTD−→F−** — **δεν το κάναμε**, μείναμε σε 2-wire.

### Firmware (`motherboard-fw/`)

| Αρχείο | Ρόλος |
|--------|-------|
| `src/mb_solar_rtd.cpp` | Driver MAX31865 — **2-wire only**, delta-R από CN4 |
| `include/mb_solar_rtd.h` | API |
| `src/main.cpp` | LCD L2 solar, auto-calibrate vs DS18 |
| `src/mb_wifi_link.cpp` | `solar_c_x10` στο UDP |

**Flash:**
```powershell
cd "D:\Romeos Tsakas\sistema-thermansis\motherboard-fw"
pio run -e esp32_motherboard -t upload --upload-port COM7
```

**Serial:** `SOLAR DIAG` @ 115200 · prefix `[solar]`

**Τρέχουσα λογική (τελευταίο flash):**
- Μόνο **MAX31865_2WIRE**
- Βαθμονόμηση **μία φορά** με CN4 (DS18): `T = T0 + (R − R0) / 0.385`
- Φίλτρο SPI: cluster median σε ζώνη ~110–175 Ω, anti-spike (max 85 °C, max +4 °C/poll)
- Cooling / stuck-recovery (μετά spike 120 °C)

### LCD layout

| Γραμμή | Περιεχόμενο |
|--------|-------------|
| L0 | CN5 exterior T+RH |
| L1 | CN4 DS18 |
| L2 | **Solar PT100** |
| L3 | Status |

---

## Αποτελέσματα δοκιμών (χρονολογικά)

| Φάση | Αποτέλεσμα |
|------|------------|
| Αρχικά | `SOLAR open`, R≈430 Ω (3W χωρίς γέφυρα F−) |
| 2-wire + delta-R | **~25.7 °C** stable, ταιριάζει CN4 — **επιτυχία bench** |
| Χέρι / αναπτήρας (παλιό heating firmware) | Αργή αντίδραση, spike **120 °C** (SPI), κόλλημα ένδειξης |
| Anti-spike firmware (τελευταία δοκιμή χρήστη) | Έναρξη **25,9 °C** → μόνο του **24,1 °C** σε ~2 λεπτά · χέρι 1 λεπτό **κανένα** · αναπτήρας + κρύωμα 2 λεπτά **κανένα** — **κολλημένοι 24,1 °C** |

---

## Τι **δεν** χρειάζεται να κάνεις τώρα (άφησέ το ως έχει)

- Μην αλλάξεις jumpers / καλώδια solar (2-wire όπως πάνω).
- Μην ξαναδοκιμάσεις 3-wire χωρίς σύρμα RTD−→F−.
- Μην πειράξεις CN4/CN5 — δουλεύουν.

---

## Εκκρεμότητα — όταν ξαναγυρίσουμε στο solar

1. **Hardware:** SPI θόρυβος MAX31865 (κοινό bus με MCP41050) — `SOLAR DIAG` raw burst κατά τη θέρμανση.
2. **Προαιρετικό hardware fix:** εξωτερικό σύρμα **RTD−→F−** + 3-wire software (μόνο αν θέλεις ξανά δοκιμή).
3. **Firmware:** ισορροπία φίλτρου — τώρα πολύ συντηρητικό (κολλάει, δεν ακολουθεί θέρμανση).
4. **Βαθμονόμηση:** drift 25,9→24,1 χωρίς αφή · ίσως re-cal μόνο όταν CN4 σταθερό >2 λεπτά.
5. **UDP 7″:** έλεγχος `solar_c_x10` όταν solar stable.

---

## Επόμενο task — INMP441 μικρόφωνο

| Πηγή | Περιεχόμενο |
|------|-------------|
| `romeos-display-v4/μητρική πλακέτα/MIC_PCB_LOCK_2026-05-08.md` | PCB lock καλωδίωσης |
| `motherboard-fw/include/pin_map.h` | GPIO: WS=**12**, BCLK=**2**, SD=**0** (CN1 / H2) |
| Display UI | Εικονίδιο mic ήδη στο v10 UI |

**Δεν ξεκίνησε ακόμα** firmware I2S / wake word στη μητρική.

---

## Άλλα (αμετάβλητα)

- Wi‑Fi: `MERCUSYS_Romeos` · MB ~192.168.0.103  
- 7″ display: UDP από μητρική  
- U11 αφαιρέθηκε (CN5 DATA stuck LOW) — **μην ξαναβάλεις**

---

*Solar PT100 — paused 2026-06-08. Επόμενο task: INMP441.*
