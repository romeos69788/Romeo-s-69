# JLCPCB — παραγγελία μαζί (Alpha + Beta + άλλα)

**Ημερομηνία:** 2026-07-12  
**Στόχος:** **μία** αποστολή · όχι πολλαπλά μεταφορικά

---

## Πριν checkout

### Alpha (Mitriki ALPHA · rev A)

- [ ] **CN_BETA** 4P — GND · TX (GPIO17) · RX (GPIO16) · NC · silk **TO BETA**
- [ ] **U3 / CN_PANEL** — οθόνη 7″ (ήδη ✓)
- [ ] DRC **51/51+** · 0 errors
- [ ] Gerber **νέο export** (μετά CN_BETA)
- [ ] Ακύρωσε **παλιό** καλάθι · βάλε **νέο** zip

### Beta (Bita Mitriki · v2.0)

- [ ] **CN_ALPHA** (U5) — silk **TO ALPHA** · **όχι** 5V pin4 · TX/RX → GPIO17/16
- [ ] OPT1–4 silk ✓
- [ ] Ρελέ U1–U4: **VCC = 5V_ESP** (όχι 3V3) — έλεγχος
- [ ] DRC clean → Gerber export

### Καλώδιο πεδίου (μετά fab)

- [ ] **1× flat 4P** Alpha CN_BETA ↔ Beta CN_ALPHA (crossover TX/RX)
- [ ] **Όχι** 5V στο link

### Άλλα PCBs

- [ ] (συμπλήρωσε ό,τι ακόμα περιμένεις — ίδιο cart JLCPCB)

---

## Σχετικά

- [`BETA-ALPHA-UART-LINK.md`](BETA-ALPHA-UART-LINK.md)
- [`ALPHA-REV-A-FINAL-LOCK-2026-07-11.md`](ALPHA-REV-A-FINAL-LOCK-2026-07-11.md)
