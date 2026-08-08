# SESSION 2026-08-08/09 — Alpha↔Beta link + pre-order fixes

**Pause:** ~00:30 2026-08-09 · συνέχεια αύριο (EasyEDA Alpha/Beta πριν 3ο fab)  
**Project:** `D:\ROMEOS 69` · boards Alpha v1 + Beta v2

---

## Τι κλείδωσε σήμερα (bench)

### UART Alpha ↔ Beta
- Flywire (χωρίς CN_BETA στη φυσική Alpha): Alpha **IO17→Beta RX**, **IO16←Beta TX**, **GND**
- Project: `board-link-test/`
  - Alpha: `PULSE 1..8` + wait `ACK` · 3 s
  - Beta: active **LOW** · ACK · boot chase
- **Link OK** — ρελέ οπλίζουν στη σειρά

### LED φιλοσοφία (σωστή ένδειξη)
- Songle: οπλισμός = GPIO **LOW** → ψείρα module **ON**
- Idle = GPIO **HIGH** → ψείρες **OFF**
- LED-K μητρικής Beta (παλιό κύκλωμα): `REL→R→LED→GND` = **active HIGH** → φαίνονταν **ανάποδα** από τις ψείρες
- **Ένδειξη οπλισμού στο rev που έχεις τώρα = ψείρα module**

### Map / σταύρωμα
- Διπλά modules: φυσικό αριστερό/δεξί ≠ net K1/K2 χωρίς διόρθωση
- Χρήστης στο EasyEDA: LED silk οπτικά αριστερά/δεξιά με **επίτηδες** cathode cross (LED-K1→REL_K2 κ.λπ.) + νέο κύκλωμα `5V→R→LED→REL`

### Flash
- In-socket fail αν **GPIO12** έχει net (strap) → ESP έξω, USB flash, ξανά μέσα
- MAC: Alpha `…48:18` · Beta `…3b:cc` — πάντα επιβεβαίωση πριν upload
- Μία φορά κατά λάθος erase/upload στην Alpha· επανήλθε

---

## EasyEDA — αλλαγές πριν 3ο fab (σε εξέλιξη)

### Beta v2 → v2.1
Checklist: [`boards/alpha-beta/PRE-ORDER-CHECKLIST-v2.1.md`](boards/alpha-beta/PRE-ORDER-CHECKLIST-v2.1.md)

| Θέμα | Κατάσταση |
|------|-----------|
| LED-K `5V→R→LED→REL` + οπτικό σταύρωμα | Σε εξέλιξη χρήστη |
| `REL_K7` **GPIO12 → GPIO19** (H1 IO19) · IO12=NC | Κλειδωμένο · να ολοκληρωθεί PCB route |
| Zener: γραμμή→+5V, άνοδος→GND · BOM 5.6V | Χρήστης OK φορά |
| Σβήσιμο LED-1/2/4 + C1/C2/C5 από DS18/FLOW | Οδηγίες δοσμένες · να τελειώσει |
| Κράτα R9 4.7k DS18 + R16 10k FLOW | — |

### Alpha v1 → επόμενο fab
Checklist: [`boards/alpha-beta/PRE-ORDER-CHECKLIST-ALPHA.md`](boards/alpha-beta/PRE-ORDER-CHECKLIST-ALPHA.md)

| Θέμα | Κατάσταση |
|------|-----------|
| Gerber με **CN_BETA** (τρέχον EasyEDA το έχει· παλιά παραγγελία όχι) | Επαλήθευση Gerber preview πριν πληρωμή |
| `DEFROST_SIG` **GPIO12 → GPIO34** · IO12=NC | Αύριο |
| DS18: χωρίς C στο DATA · χωρίς LED στο bus | Αύριο |
| Zener 5.6V φορά | Ίδιο με Beta |
| UART: Beta 17/16 · Panel TX25/RX33 | Έλεγχος labels αύριο |
| CT μόνο στη Beta OPT1 | Μην βάλεις 2ο CT στην Alpha |

---

## Αρχεία

| Path | Ρόλος |
|------|--------|
| `board-link-test/` | Bench UART + ρελέ |
| `docs/boards/alpha-beta/README.md` | Pin maps, LED notes, links |
| `docs/boards/alpha-beta/PRE-ORDER-CHECKLIST-v2.1.md` | Beta fab |
| `docs/boards/alpha-beta/PRE-ORDER-CHECKLIST-ALPHA.md` | Alpha fab |
| `docs/SESSION-2026-08-05-CONTROL-BOARD-BRINGUP.md` | Power/zener/GPIO12 πρώτο bring-up |

---

## Αύριο (σειρά)

1. Beta: τελείωσε GPIO19 route + σβήσιμο C/LED αισθητήρων · DRC  
2. Alpha: DEFROST→34 · DS18 caps/LEDs · UART label check  
3. Και τα δύο: Gerber preview screenshots στο `docs/boards/alpha-beta/`  
4. FW pin map μετά το fab (K7=19, κ.λπ.) — όχι επείγον για την τωρινή πλακέτα

---

## Κανόνες άθικτοι

- CT: **μόνο** SCT-013 στο HP (Beta OPT1)  
- Όχι 230 V στα contacts μέχρι κλείδωμα map  
- MAX485: στοκ για future HP RS485 · όχι στο τρέχον Alpha↔Beta (UART)  
