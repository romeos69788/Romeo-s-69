# Μητρική πλακέτα — bring-up (2026-06-25)

**Κατάσταση:** PCB **έτοιμο / συναρμολογημένο** · ξεκινάμε **έλεγχο λειτουργίας**.

**Για επανέναρξη chat:** *«συνέχισε bring-up μητρικής από SESSION-BRINGUP_2026-06-25»*

---

## COM ports (επιβεβαιωμένα 2026-06-25)

| Συσκευή | COM | Chip USB |
|---------|-----|----------|
| **Μητρική ESP32** | **COM7** | CP210x (Silicon Labs) |
| **Οθόνη / θερμοστάτης 7″** | **COM3** | CH340 |

**Σημαντικό:** Μητρική ↔ οθόνη **δεν** μιλάνε με καλώδιο Serial μεταξύ τους — μιλάνε **Wi‑Fi UDP** (θύρες 9000/9001). Τα COM είναι **ξεχωριστά** για flash και Serial Monitor.

---

## Πού μείναμε (τελευταία συζήτηση μητρικής — assembly)

Πριν την παύση, δουλεύαμε **συναρμολόγηση / BOM στο bench**, όχι νέο schematic:

| Θέμα | Απόφαση |
|------|---------|
| **U11** (CN5 DHT) | **Μένει** · κάθοδος → DHT_DATA, άνοδος → GND · OK **1N4007** αν δεν έχεις 1N4448 |
| **C2** (5V_PRE) | Αντί 10 µF/16 V → **22 µF / 50 V** OK (παράλληλα με C1 470 µF) |
| **C1** | **470 µF / 16 V** — **μένει** |
| **Τροφοδοσία** | 12 V → **shield** → **buck** → **CN1/U3** (+5V/GND) · F1→ESP, F2→AUX |

**HP πλακέτα (outdoor):** ξεχωριστό project — βλ. `κατασκευή αντλίας θερμότητας νερού νερού/`

---

## Τι είχε μείνει από σχεδίαση (πριν fab)

Από `REVIEW_board_v1_2026-05-07.md` + `motherboard-fw/README.md`:

- [ ] ERC/DRC τελικό (αν δεν τρέχηκε πριν παραγγελία)
- [ ] Silk CN4/CN5/U6 — ονόματα πεδίου
- [ ] **Bring-up firmware:** αισθητήρες → Serial + UDP
- [ ] Ρελέ: **LED δοκιμή** πριν 230 V
- [ ] Λογική K1–K6 (μετά sensors OK)
- [ ] Οθόνη v10 ↔ μητρική UDP στο ίδιο LAN

---

## Live check 2026-06-25 (COM7)

Firmware **τρέχει** — Serial **115200**:

```
[boot] setup start
[boot] setup done — loop
[wifi] STA home OK ip=192.168.0.103
[tick] millis=5000 flow=0 defrost=1
```

**Σημαίνει:** ESP32 OK · Wi‑Fi σπιτιού OK · loop τρέχει · **flow=0** · **defrost=1** (έλεγχος καλωδίωσης CN2 / pull-up R9).

---

## Σειρά ελέγχου (προτείνεται)

### Φάση A — χωρίς 230 V

1. **Τροφοδοσία:** 12 V → shield → buck → **~5,0 V** στο multimeter (κενό φορτίο) · LED7 + LED τροφοδοσίας ESP/AUX.
2. **COM7 Serial Monitor** 115200 → `[boot]` + κάθε 5 s `[tick]`.
3. **LCD 2004 I2C** (addr **0x27**) → γραμμές status / alarm.
4. **Αισθητήρες** (ένα-ένα): CN4 DS18 · CN5 DHT · U6 flow · CN2 defrost · U4 backup.
5. **Ρελέ:** modules **χωρίς 220 V** — LED στις εξόδους ή μ μόνο coil click · `[tick]` + manual test GPIO.

### Φάση B — οθόνη (COM3)

6. Οθόνη **v10** στο **COM3** — upload αν χρειάζεται (`BOARD_VIEWE_V10`).
7. Και τα **δύο** στο **ίδιο Wi‑Fi** (`MERCUSYS_Romeos`) — μητρική ήδη **192.168.0.103**.
8. Οθόνη: εξωτερική θερμοκρασία / setpoint / alarm από UDP.

### Φάση C — αργότερα

9. 230 V ρελέ με **φορτία** (κυκλοφορητές κ.λπ.) — μόνο μετά Φάση A+B.
10. MCP41050 / MAX31865 / buzzer / MIC I2S.

---

## Εντολές

**Μητρική — monitor:**
```powershell
cd "D:\Romeos Tsakas\motherboard-fw"
pio device monitor -p COM7 -b 115200
```

**Μητρική — upload:**
```powershell
pio run -e esp32_motherboard -t upload --upload-port COM7
```

**Οθόνη — upload:**
```powershell
cd "D:\Romeos Tsakas\romeos-display-v10\romeos-display-v10"
pio run -e BOARD_VIEWE_V10 -t upload --upload-port COM3
```

---

## Αρχεία αναφοράς

| Αρχείο | Περιεχόμενο |
|--------|-------------|
| `motherboard-fw/README.md` | Firmware, Wi‑Fi, πλάνο δοκιμών |
| `motherboard-fw/include/pin_map.h` | GPIO |
| `thermostat-ui-demo/docs/romeos-design-notes.md` | §12.x πλήρης χάρτης |
| `PROJECT-STATUS.md` | COM3/COM7, v10 |
| `REVIEW_board_v1_2026-05-07.md` | Έλεγχος σχεδίου |

---

*Bring-up ξεκίνησε 2026-06-25.*
