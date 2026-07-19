# Wokwi — πρώτα βήματα (για αρχάριους)

Δεν χρειάζεται προηγούμενη εμπειρία. Ξεκίνα από το **browser** (χωρίς εγκατάσταση), μετά σύνδεσε το με το firmware Alpha.

## Τι είναι

Το [Wokwi](https://wokwi.com) είναι **προσομοιωτής ESP32 στο PC/browser**. Βλέπεις Serial, LED, κουμπιά — χωρίς να κατεβάσεις κώδικα στην πραγματική πλακέτα.

**Δεν** αντικαθιστά τον πάγκο (CT, πραγματικά ρελέ, ρεύμα).

---

## Βήμα 1 — δοκίμασε στο browser (5′)

1. Άνοιξε: https://wokwi.com/projects/new/esp32  
2. Θα δεις ένα ESP32 και ένα απλό sketch (συνήθως blink).  
3. Πάτα το πράσινο **▶ Start simulation**.  
4. Άνοιξε το **Serial Monitor** κάτω — θα δεις εκτυπώσεις.  
5. Άλλαξε κάτι στο sketch (π.χ. `delay(1000)` → `delay(200)`) και ξαναπάτα ▶.

Αν αυτό δουλέψει, ξέρεις ήδη το βασικό: κώδικας → προσομοίωση → Serial/LED.

Λογαριασμός: δωρεάν στο site (για αποθήκευση project).

---

## Βήμα 2 — το δικό μας Alpha στο Cursor / VS Code

Στο `D:\ROMEOS 69\alpha` υπάρχουν ήδη:

| Αρχείο | Ρόλος |
|--------|-------|
| `diagram.json` | Εικονικό ESP32 + LED για ρελέ K1–K6 |
| `wokwi.toml` | Δείχνει στο build `alpha_rev_a_bench` |

### Εγκατάσταση (μία φορά)

1. Στο Cursor (ή VS Code): εγκατάστησε extension **Wokwi Simulator** (`Wokwi.wokwi-vscode`).  
2. `F1` → **Wokwi: Request a new License** → άνοιξε browser → **GET YOUR LICENSE** → σύνδεση λογαριασμού → επιβεβαίωση.  
3. Πρέπει να δεις μήνυμα τύπου «License activated».

### Τρέξιμο προσομοίωσης

Στο PowerShell:

```powershell
cd "D:\ROMEOS 69\alpha"
pio run -e alpha_rev_a_bench
```

Μετά στο Cursor:

1. Άνοιξε το αρχείο `alpha/diagram.json`.  
2. `F1` → **Wokwi: Start Simulator** (ή το κουμπί ▶ στο diagram).  
3. Στο Serial θα δεις: `ROMEOS 69 Alpha rev A — motherboard-fw` και `[mqtt] bench mode…`.  
4. Τα LED K1–K6 αντιστοιχούν στα GPIO των ρελέ.  
5. Πάτα το κουμπί **BOOT** στο εικονικό ESP32 → εναλλάσσει το **heater (K4)**.

> Πάντα **build πρώτα** (`pio run`), μετά Start Simulator — αλλιώς λείπει το `.bin`.

---

## Τι θα δεις / τι όχι

| Ναι στο Wokwi | Όχι (μένει πάγκος) |
|---------------|---------------------|
| Serial logs | SCT-013 CT (GPIO35) πραγματικό RMS |
| LED αντί για ρελέ | 230 V / φορτία |
| Κουμπί BOOT → heater | Alpha↔Beta UART (δύο πλακέτες — αργότερα) |
| Compile + λογική χωρίς flash | TUYA meter, θόρυβος γραμμής |

---

## Συχνά προβλήματα

| Πρόβλημα | Λύση |
|----------|------|
| «firmware not found» | `pio run -e alpha_rev_a_bench` στο `alpha\` |
| Extension χωρίς license | `F1` → Request a new License |
| Τίποτα στο Serial | έλεγξε baud 115200 · ξαναπάτα ▶ |
| Cursor δεν έχει Wokwi | δοκίμασε VS Code ή μόνο browser στο βήμα 1 |

---

## Επόμενα (όταν συνηθίσεις)

1. Δεύτερο ESP32 στο ίδιο diagram για **UART Alpha↔Beta** (TX17/RX16).  
2. Περισσότερα κουμπιά / αισθητήρες εικονικά.  
3. Πραγματικός πάγκος για επιβεβαίωση I/O.

Επίσημα docs: https://docs.wokwi.com/vscode/getting-started  

*Τελευταία ενημέρωση: 2026-07-19*
