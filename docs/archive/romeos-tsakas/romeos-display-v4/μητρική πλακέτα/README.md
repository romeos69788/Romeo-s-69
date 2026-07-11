# Μητρική πλακέτα

**Τελευταία ενημέρωση:** 2026-06-25 — PCB **έτοιμο** · **bring-up / δοκιμές** σε εξέλιξη.

**Ξεκίνα από εδώ:** [`SESSION-BRINGUP_2026-06-25.md`](SESSION-BRINGUP_2026-06-25.md)

Φάκελος για σχεδιασμό **και** bring-up της μητρικής CONTROL BOARD v1.0.

## Γιατί υπάρχει

- Να συγκεντρώνονται εδώ ιδέες για **επιπλέον στοιχεία** (GPIO, τροφοδοσία, connectors, μικρόφωνο, κ.λπ.) ώστε να κλειδώνει το schematic/Gerber όταν είστε έτοιμοι.
- **Χρόνος παράδοσης PCB** συχνά της τάξης **~3 εβδομάδες** — ό,τι προλαβαίνει να μπει στη ρεβιού πριν το «φύγει» η παραγγελία, γλιτώνει δεύτερο κύκλο αναμονής.

## Σύνδεση με το υπόλοιπο project

- Το firmware της οθόνης (v4) επικοινωνεί με τη μητρική μέσω **Wi‑Fi / UDP** (`romeos_display_link`, κοινά types σε `romeos_link_types.h` όπου υπάρχουν στο workspace).
- Γενικές σημειώσεις από συζητήσεις (φωνή, UI κ.λπ.): δες `docs/discussion_notes_el.md`.

## Έλεγχος σχεδίου πριν την παραγγελία

- **`REVIEW_board_v1_2026-05-07.md`** — πρώτος έλεγχος βάσει schematic EasyEDA + PCB layout (λειτουργικά μπλοκ, σημεία προσοχής 220 V, ESP32 pins, σχέση με οθόνη v4, προτάσεις). Ενημέρωσέ το όταν αλλάξει έκδοση πλακέτας.
- **`RELAYS_MODULES.md`** — τοπολογία διπλών modules, μπόιλερ μέσω πίνακα, **6ο ρελέ = ηλεκτροβάνα εκτροπής**.
- **`DEFROST_REV_VALVE_AND_MODE_SENSE.md`** — αίσθηση πηνίου βαλβίδας εκτροπής εξωτερικής (module AC+opto), σκοπός εκτροπής νερού σε απόψυξη, σχέση με `PIN_AC_OPTO` / firmware.
- **`AC_FAN_STANDALONE_COLOCATION.md`** — κύκλωμα AC fan spoofer (NE555 + L7805): προσομοίωση ventilator εσωτερικής· αυτόνομο ως προς Romeos GPIO.
- **`POWER_12V_SECURE_AND_BUCK.md`** — shield **12V SECURE** (ασφάλεια 1 A, 1N5408, ένδειξη) πριν το module **LM2596** 12→5 V· καθαρή 5 V προς επεξεργαστή.
- **`SENSOR_SOLAR_PT100_MAX31865.md`** — PT100 στον ηλιακό + module **MAX31865** (SPI, `PIN_CS_SOLAR`)· χωριστά από DS18B/DHT boiler/νερού.
- **`CONNECTORS_AND_INDOOR_AIR_SPOOF.md`** — κλέμες CN4/CN5/U4/U5/U6, LCD I2C, στρατηγική fake εσωτερικού αέρα για inverter.
- **`DISCUSSION_SUMMARY_2026-05-07.md`** — σύνοψη συζήτησης (αισθητήρες/εφεδρεία, HMI 5″ UART vs θερμοστάτης 7″ Wi‑Fi).
- **`MIC_INMP441_I2S.md`** — μικρόφωνο **INMP441** I2S ×2 (μητρική + θερμοστάτης 7″).
- **`MIC_PCB_LOCK_2026-05-08.md`** — **κλείδωμα παραγγελίας:** κλέμα **J_MIC 1×6**, GPIO 2/12/0, αντιστάσεις, checklist EasyEDA.

## Επόμενα βήματα (placeholder)

- [ ] Κλείδωμα αντιστοίχισης **K1…K6** → CIRC1…3, HP, (6ο), και πώς ακριβώς οδηγείται το **μπόιλερ** (contactor).
- [ ] Στο EasyEDA: **κλέμα MIC_I2S** + ανάθεση **3 GPIO** για I2S (BCLK, WS, DIN) χωρίς σύγκρουση — μετά ενημέρωση `motherboard-fw/include/pin_map.h`.
- [ ] Παραγγελία **2× INMP441** breakout (ίδιο module) για μητρική και θερμοστάτη.
