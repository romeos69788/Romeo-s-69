# Beta rev A — τελικός έλεγχος πριν παραγγελία JLCPCB

**Ημερομηνία:** 2026-07-12  
**Πηγές:** full schematic + PCB layout (χρήστης) · locked docs session 2026-07-12  
**Κατάσταση:** ✓ **ΠΑΡΑΓΓΕΛΙΑ ΕΓΚΡΙΘΗΚΕ** (χρήστης 2026-07-12) · άφιξη ~τέλος Ιουλίου · bench test μετά

**Επόμενο:** [`BETA-BENCH-BRINGUP-HANDOFF-2026-07-12.md`](BETA-BENCH-BRINGUP-HANDOFF-2026-07-12.md)

---

## ✓ Pre-fab — ολοκληρώθηκε (2026-07-12)

| Έλεγχος | Αποτέλεσμα |
|---------|------------|
| Update PCB from Schematic | ✓ |
| Design Manager nets | **52/52** |
| DRC | **0 errors** |
| CN1 OPT1-CURRENT | ✓ pin1=OPT_CT_IN · pin2=GND |
| CN2/CN3 DS18 | ✓ |
| CN4 flow | ✓ |
| U1–U4 + REL_K1…K8 | ✓ |
| K1–K8 field map + silk | ✓ |

---

## 🔴 ΜΗΝ παραγγείλεις αν δεν περάσει *(ιστορικό — πέρασε)*

### 1. Schematic ↔ PCB — **ίδια nets / ίδια GPIO**

Το **PCB layout** (screenshot) φαίνεται **παλιό routing** — **διαφορετικό** από το κλειδωμένο schematic:

| Net / λειτουργία | **Κλειδωμένο** (schematic session) | **PCB screenshot** (φαίνεται) | |
|------------------|-------------------------------------|-------------------------------|---|
| REL_K1 | GPIO **32** H2-7 | IO32 ✓ | ✓ |
| REL_K2 | GPIO **26** H2-10 | IO**33** | 🔴 |
| REL_K3 | GPIO **27** H2-11 | IO**25** | 🔴 |
| REL_K4 | GPIO **12** H2-13 | IO**26** | 🔴 |
| REL_K5 | GPIO **13** H2-15 | IO**27** | 🔴 |
| REL_K6 | GPIO **15** H1-16 | IO**14** | 🔴 |
| REL_K7 | GPIO **14** H2-12 | IO**12** | 🔴 |
| REL_K8 | GPIO **25** H2-9 | IO**13** | 🔴 |
| BETA_DS18_DATA | GPIO **4** H1-13 | IO**18** / IO**19** (ξεχωριστά) | 🔴 |
| BETA_FLOW_SIG | GPIO **34** H2-5 | IO**4** | 🔴 |
| OPT_CURRENT_SIG | GPIO **35** H2-6 | IO**21** | 🔴 |
| CN_ALPHA TX/RX | GPIO **17/16** H1-11/12 | IO**1/3** (UART0) | 🔴 |

**→ EasyEDA:** **Design → Update PCB from Schematic** (ή ξανά-annotate) · **zoom κάθε net** στο copper · πρέπει να ταιριάζει **ακριβώς** τον πίνακα [`BETA-RELAY-MAP.md`](BETA-RELAY-MAP.md) § H1/H2.

---

### 2. CN_ALPHA (U5)

| Pin | Πρέπει |
|-----|--------|
| 1 | GND |
| 2 | TX → **GPIO17** |
| 3 | RX → **GPIO16** |
| 4 | **NC** — **όχι** 3V3 / 5V |

Silk: **CN_ALPHA · TO ALPHA** (όχι 7″ SCREEN).

---

### 3. DRC / ERC

- [ ] **0 errors** (nets πλήρης)
- [ ] **230 V** ζώνη · clearance/creepage OK
- [ ] **Mounting holes** pads

---

## 🟡 Έλεγξε (δεν σταματά fab αν σκόπιμο)

| Θέμα | Σημείωση |
|------|----------|
| **LED-1 / LED-2** στο DS18 bus | OK για ένδειξη · **R9 4,7 kΩ** μία φορά · αν πρόβλημα bench → αφαίρεσε ένα LED |
| **D1 Zener 8,2 V** | OK Beta · Alpha έχει 5,6 V |
| **LED-K5…K8** ονομασία | LED-5…8 vs LED-K5…K8 — cosmetic |
| **U3 vs U4** footprint | Connector **U4**=K5/K6 · **U3**=K7/K8 (schematic lock) — silk REL3/REL4 να ταιριάζει |
| **CN2/CN3 pin order** | **1=3V3 · 2=GND · 3=SING** |

---

## ✓ Κλειδωμένα (schematic session — επιβεβαίωσε στο copper)

### Τροφοδοσία

```
12V IN → buck → CN9 5V_PRE → C3/C4/D1 → F1 → 5V_ESP → H2-19 + U1–U4 pin4
```

### OPT (hardware wired · firmware standby)

| Κλέμα | Net | GPIO |
|-------|-----|------|
| CN1 SCT-013 | OPT_CT_IN → C6 → OPT_CURRENT_SIG | **35** |
| CN2/CN3 DS18 | BETA_DS18_DATA + R9 4k7 | **4** |
| CN4 YF-B10 | BETA_FLOW_SIG + R16 10k → 3V3 · pin2 **5V_ESP** | **34** |

### Ρελέ K1–K8 (πεδίο)

| K | Πεδίο | GPIO |
|---|-------|------|
| K1 | Κυκλοφορητής 1 | 32 |
| K2 | Κυκλοφορητής 2 | 26 |
| K3 | Κυκλοφορητής 3 | 27 |
| K4 | Κυκλ. ηλιακού | 12 |
| K5 | Τρίοδη βάνα | 13 |
| K6 | HP | 15 |
| K7 | 4 kW αντίσταση | 14 |
| K8 | ST-BY spare | 25 |

**U1–U4:** pin1 GND · pin4 **5V_ESP** · LED 1kΩ παράλληλα IN.

---

## Alpha (ίδιο cart)

- [ ] **CN_BETA** 4P · νέο Gerber
- [ ] DRC 0 errors

→ [`JLCPCB-ORDER-CHECKLIST-2026-07-12.md`](JLCPCB-ORDER-CHECKLIST-2026-07-12.md)

---

## Σχετικά

- [`BETA-RELAY-MAP.md`](BETA-RELAY-MAP.md)
- [`BETA-OPT-HARDWARE-MAP.md`](BETA-OPT-HARDWARE-MAP.md)
- [`BETA-ALPHA-UART-LINK.md`](BETA-ALPHA-UART-LINK.md)
