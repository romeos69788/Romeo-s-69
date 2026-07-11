# ROMEOS 69 — υπενθυμίσεις αγορών (AliExpress / BOM)

**Ενημέρωση:** 2026-07-11

---

## AliExpress — item 1005010806316629 (~€9,08)

**Link:** https://www.aliexpress.com/item/1005010806316629.html

**Γιατί μπήκε στο καλάθι (πιθανό — επιβεβαίωσε τίτλο στο καλάθι):**

| Πιθανό προϊόν | Σχέση με ROMEOS 69 Alpha |
|---------------|--------------------------|
| **SparkFun QwiicBus EndPoint (COM-16988)** ή clone | **CN5 OUTDOOR-I2C** — I2C SHT40 outdoor μέσω Cat5/RJ45 |
| DS3231 RTC module (6P) | **U1 DIGITAL CLOCK** — αν δεν έχεις ήδη module |
| SHT40/41 I2C breakout | Outdoor θερμοκρασία/υγρασία (δεύτερο EndPoint + sensor) |

**Πιο πιθανό** (από docs): **Qwiic EndPoint COM-16988** — σημειωμένο ως **«⏳ λείπει»** στο review πριν συναρμολόγηση.

### CN5 — τι χρειάζεσαι

```
Alpha PCB (CN5) ── EndPoint #1 ── Cat5/RJ45 ── EndPoint #2 (outdoor) ── SHT40
```

| Τεμάχιο | Ποσότητα | Σημείωση |
|---------|----------|----------|
| **QwiicBus EndPoint** | **2×** | 1 onboard κοντά CN5 · 1 outdoor |
| **SHT40/41** Qwiic | **1×** | 0x44 στο I2C bus |
| Cat5 / RJ45 | καλώδιο | outdoor run |

**EN:** hardwire **3V3** στο EndPoint (όχι pin στο CN5 κλέμα).

### Επιβεβαίωση πριν checkout

1. Άνοιξε καλάθι → **copy τίτλο προϊόντος** (αν δεν ταιριάζει με EndPoint/DS3231/SHT40 → ρώτα ξανά)
2. Αν είναι **1× EndPoint** → πρόσθεσε **2η** μονάδα (χρειάζεσαι 2 για CN5)
3. **Όχι** SCT-013 CT — rule ROMEOS 69 (kWh HP από WiFi μετρητή)

---

## Άλλα parts (όχι AliExpress link — checklist συναρμολόγησης Alpha)

| Part | Για |
|------|-----|
| ESP32 DevKit 38pin | H1/H2 |
| MCP41050 | U2 · CN1 ROOM-NTC spoof |
| DS3231 module | U1 |
| DS18B20 probes | CN3/4/10 |
| Viewe 7″ panel | CN_PANEL |
| Buzzer KY-012 | U2 |
| 12V→5V buck module | MODULE zone |
| Fan spoofer daughterboard | AC FAN SPOOFER zone |

**Φάση 2 (Alpha/Beta):** ρελέ K1–K6 · F2 · CN_BETA — **όχι** σε αυτή την παραγγελία PCB Alpha.
