# ROMEOS 69 — υπενθυμίσεις αγορών (AliExpress / BOM)

**Ενημέρωση:** 2026-07-11

---

## AliExpress — item 1005010806316629 (~€9,08) ✓ επιβεβαιώθηκε

**Link:** https://www.aliexpress.com/item/1005010806316629.html

**Προϊόν:** **AMS1117-3.3V LDO module** (5 τεμάχια / πακέτο ~€9,08)  
**Pins:** VIN · VOUT (3,3 V) · GND · ~800 mA max

### Γιατί μπήκε στο καλάθι

**3,3 V τροφοδοσία** για κυκλώματα ROMEOS — **δεν** είναι Qwiic EndPoint.

| Χρήση | Σχέση Alpha rev A |
|-------|-------------------|
| **Bench / πρωτότυπο** | DS18, CN5, CN_DEFROST δοκιμές πριν έρθει PCB |
| **Outdoor κουτί αισθητήρα** | SHT40 / EndPoint: **5 V → AMS1117 → 3,3 V** τοπικά (μακριά καλώδιο) |
| **Εφεδρικό 3,3 V rail** | Αν το DevKit 3,3 V (H2-1) «σέρνεται» με πολλά DS18 + SHT40 |
| **Γενικό stock** | DIY · daughterboards · fan spoofer zone |

### Στη μητρική Alpha rev A (PCB)

**Κύριο 3,3 V:** από **ESP32 DevKit H2-1** → CN5 · CN_DEFROST · CN3/4/10 (3V3 pin).

Τα **AMS1117 modules δεν τοποθετούνται υποχρεωτικά πάνω στο PCB** — είναι **εξωτερικά/bench/outdoor**.

**Αν χρειάζεσαι ξεχωριστό rail:** VIN ← **5V_ESP** ή **5V_PRE** (μετά fuse) · VOUT → 3,3 V bus · **κοινό GND**.

### Ποσότητα

**5 τεμάχια** — OK για stock (1 bench · 1 outdoor · 2 spare).

### ⚠️ Δεν αντικαθιστά

| Item | Σημείωση |
|------|----------|
| **Qwiic EndPoint COM-16988** | Ξεχωστή παραγγελία — **2×** για CN5 |
| **SHT40** | Ξεχωστή παραγγελία |
| **SCT-013 CT** | **Όχι** στην **Alpha** rev A · **1×** optional **Beta CN1** (OPT1) — βλ. [`BETA-OPT-HARDWARE-MAP.md`](../BETA-OPT-HARDWARE-MAP.md) |

---

## AliExpress — SCT-013 CT (Beta OPT1-CURRENT)

**Link:** https://www.aliexpress.com/item/1005007531832172.html

**Προϊόν:** **SCT-013** split-core CT · έκδοση **1 V output** (built-in burden)  
**Ποσότητα:** **1×** για **Beta CN1** · **όχι** δεύτερο CT elsewhere

| Έκδοση | Πότε |
|--------|------|
| **20 A / 1 V** | Γραμμή HP / outdoor (προτείνεται αν max ~20 A) |
| **10 A / 1 V** | Μικρότερο φορτίο |

**PCB:** R_CT1/R_CT2 **10 kΩ** · C_CT1 **10 µF** · **GPIO35** · CN1 **2P** (κόψιμο TRS jack στο πεδίο).

---

## AliExpress — YF-B10 flow (Beta CN4 OPT4)

**Link:** https://www.aliexpress.com/item/1005005588692911.html

**Προϊόν:** **YF-B10** Hall brass flow · **3/4"** · ~2–50 L/min  
**Καλώδιο:** κόκκινο **5V** · μαύρο **GND** · κίτρινο **pulse**

**PCB CN4:** pin **GND · 5V_ESP · PULSE** → **GPIO34** + **R12 10 kΩ** → 3V3.

---

## Άλλα parts — checklist συναρμολόγησης Alpha

| Part | Για |
|------|-----|
| ESP32 DevKit 38pin | H1/H2 |
| MCP41050 | U2 · CN1 ROOM-NTC |
| DS3231 module | U1 |
| DS18B20 probes | CN3/4/10 |
| **Qwiic EndPoint ×2** | CN5 outdoor I2C |
| **SHT40/41** | outdoor θερμοκρασία/υγρασία |
| Viewe 7″ panel | CN_PANEL |
| Buzzer KY-012 | U2 |
| 12V→5V buck module | MODULE zone |

**Φάση 2:** ρελέ K1–K6 · F2 · CN_BETA
