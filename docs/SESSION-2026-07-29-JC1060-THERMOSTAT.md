# SESSION 2026-07-29 — JC1060 θερμοστάτης χώρου

**Θέμα:** Bring-up JC1060P470C · EEZ · λειτουργικό home/menu · Wi‑Fi/NTP · polish knob  
**Update:** 2026-08-02 ~19:00 — **pause** · Wi‑Fi OK · knob pad=10 + shadow · sync cloud

---

## Κατάσταση (όπου μείναμε)

| Στοιχείο | Τιμή |
|----------|------|
| Οθόνη | JC1060P470C_I_W_Y · ESP32-P4 · **1024×600** · **COM14** |
| Ρόλος | **Θερμοστάτης χώρου** (όχι Alpha hub) |
| Panel hub | Viewe · COM4 · V4 κλειδωμένο — **άθικτο** (μόνο diagnostics) |
| UI τώρα | 6 οθόνες · home+menu labels · tiles→3–6 · soft slide ~980 ms |
| Setpoint | 10–40 °C · οπτικό mid=**20 °C** · arc knob **pad=10** + shadow |
| Ρολόι | NTP Αθήνα όταν Wi‑Fi up · blink `:` |
| Wi‑Fi | `secrets/wifi.env` → `MERCUSYS_ROMEOS` · εικονίδιο **πράσινο** όταν OK |
| C6 | ESP-Hosted slave **2.12.11** (SDIO OTA από embed `data/c6_slave_2.12.11.bin`) |
| Firmware | `display-jc1060/` · embed `img_1`…`img_6` + fonts + C6 bin |

### Πλοήγηση

- Home MENU → οθόνη 2 (slide)
- Menu BACK → home · tiles → οθόνες 3–6 (HP / φώτα / θέρμανση / ρολά)
- Detail BACK → menu

### Wi‑Fi notes (2026-08-02)

- P4 χωρίς radio · Wi‑Fi = **ESP32-C6** via ESP-Hosted SDIO (CLK18 CMD19 D0–D3=14–17 RST54)
- Παλιό Guition slave → RPC fail · OTA στο **2.12.11** το έλυσε
- Ίδια credentials με panel · C6 RF πιο αδύναμο από panel STA (~−30 dB στο ίδιο desk) · τελικά **συνδέθηκε** στο lab
- CT rule άθικτο: μόνο SCT-013 στο HP line

---

## Review σχεδίου (2026-08-02) — σύνοψη

| Οθόνη | Συμπέρασμα |
|-------|------------|
| A home | Κρατάμε κύκλο · knob polish OK |
| B menu | Hub μέσω tap |
| C/E | Απλοποίηση αργότερα |
| D | Πραγματικά ονόματα |
| F | Καλύτερη λεπτομέρεια |

---

## Επόμενα (άλλη μέρα)

1. Live data από Alpha / αισθητήρες (όχι μόνο placeholders)
2. EEZ refinements C/D/E αν κλειδώσει το review
3. Σταθερότητα Wi‑Fi στο λεβητοστάσιο (RF/κεραία C6)
4. Λέβητας / MQTT αργότερα

---

## Σχετικά

- [`DISPLAY-TWO-ROLES-2026-07-19.md`](DISPLAY-TWO-ROLES-2026-07-19.md)
- [`Thermostat_JC1060/README.md`](../Othoni%20Levita/Thermostat_JC1060/README.md)
- Firmware: `display-jc1060/`
