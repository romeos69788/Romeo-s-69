# Beta — πλακέτα εξωτερικού (outdoor relay board)

Firmware / hardware για την **πλακέτα Beta** στον εξωτερικό χώρο (HP / inverter outdoor unit).

## Ρόλος rev A

**Κυρίως ρελέ** — contactors, OEM heat/cool lines, pump/servo τροφοδοσία HP.

**Προαιρετικές κλέμες (hardware only · όχι firmware rev A):** **OPT1-CURRENT** · **OPT2/3-WATER-T** · **OPT4-WATER-FLOW**

**Όχι στην Beta (μεταφέρθηκαν / μένουν στην Alpha):**

| Λειτουργία | Πού rev A |
|------------|-----------|
| DEFROST | **Alpha** — **CN_DEFROST** → GPIO14 |
| DS18 νερού, SHT40, MCP41050 | **Alpha** |
| Λογική ελέγχου / MQTT | **Alpha** ESP32 |

## Σύνδεση με Alpha — **CN_ALPHA** (κλείδωμα 2026-07-12)

**4P UART** → **CN_BETA** στην Alpha · [`../docs/BETA-ALPHA-UART-LINK.md`](../docs/BETA-ALPHA-UART-LINK.md)

| Pin | Silk | ESP32 Beta |
|-----|------|------------|
| 1 | GND | GND |
| 2 | TX | **GPIO17** (H1-11) |
| 3 | RX | **GPIO16** (H1-12) |
| 4 | NC | — |

**115200** 8N1 · **crossover** καλώδιο (A-TX→B-RX) · **όχι** 5V στο link · κάθε πλακέτα δική τροφοδοσία.

## Κατάσταση

- [ ] PlatformIO project scaffold
- [ ] UART πρωτόκολλο Alpha ↔ Beta (αν χρειαστεί)
- [ ] Ρελέ HP / OEM wiring
- [ ] **Session επόμενη:** ορισμός ρόλου + κλέμες + εντολές πριν fab (ίδια ροή με Alpha)

## Session επόμενη — ατζέντα (πριν παραγγελία)

1. **Ρόλος:** Beta = **ρελέ outdoor** · Alpha = αισθητήρες + λογική + MQTT
2. **Ρελέ K1–K6:** ποιο κουτί/γραμμή ελέγχει το καθένα (HP contactor, pumps, 4 kW, solar valve, spare)
3. **CN_BETA** (Alpha side): GND · BETA_TX (17) · BETA_RX (16) · NC — **χωρίς** τροφοδοσία Beta από Alpha
4. **UART:** τι εντολές στέλνει Alpha → Beta · τι status επιστρέφει (relay states, faults)
5. **Τροφοδοσία Beta:** δικό της 5 V / 12 V IN · fuse · **F2/5V_AUX** αν χρειάζεται εδώ
6. **230 V:** 3× διπλά ρελέ modules · ζώνη HV · LED · **όχι** DEFROST (μένει Alpha CN_DEFROST)
7. **Lock checklist** → schematic → PCB → DRC → Gerber (όπως Alpha)

### Προτεινόμενος πίνακας ρελέ (αφετηρία συζήτησης — **όχι locked**)

| Ρελέ | Προτεινόμενη λειτουργία | Σημείωση |
|------|-------------------------|----------|
| K2 | Μικρός κυκλοφορητής / buffer | defrost → OFF (Alpha GPIO14) |
| K3 | Κύριος κυκλοφορητής | defrost → ON |
| K4 | Heater 4 kW / solar valve | |
| K1, K5, K6 | HP OEM / spare | TBD αύριο |

## Docs

- [`../docs/ALPHA-REV-A.md`](../docs/ALPHA-REV-A.md)
- [`../docs/ALPHA-REV-A-PCB-REVIEW.md`](../docs/ALPHA-REV-A-PCB-REVIEW.md)
- [`../docs/BETA-ALPHA-UART-LINK.md`](../docs/BETA-ALPHA-UART-LINK.md) — **CN_ALPHA / CN_BETA**
