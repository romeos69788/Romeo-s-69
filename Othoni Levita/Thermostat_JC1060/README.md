# Thermostat_JC1060 — θερμοστάτης χώρου

**Hardware:** Jingcai **JC1060P470C_I_W_Y** · ESP32-P4 · **1024×600** · GT911 · COM14  
**Ρόλος:** θερμοστάτης χώρου (setpoint · comfort · φώτα · ρολά) — **όχι** panel λεβητοστασίου  
**Panel hub:** παραμένει Viewe 7″ · COM4 · `Othoni_Levita_*`

---

## EEZ — σχεδίασε εδώ

| | |
|--|--|
| **Ανάλυση EEZ** | **1024 × 600** (native — πλήρης οθόνη) |
| **Project τώρα** | `THermostat NEW\THermostat NEW.eez-project` (ήδη σε 1024×600) |
| **Assets** | εικόνες / fonts σε `assets\` |

Μην σχεδιάζεις σε 800×480 (αυτό ήταν το παλιό Viewe / display-v10).

### Πώς αλλάζεις ανάλυση στο EEZ Studio

Στο Create **δεν** υπάρχει επιλογή 1024×600 — ξεκινάει πάντα 800×480. Αλλάζεις **μετά**:

1. Άνοιξε το project.
2. Πάτα το **γρανάζι (Settings)** στη γραμμή εργαλείων.
3. **General → Display width** = `1024` · **Display height** = `600`.
4. Στο αριστερό δέντρο → **Pages** → επίλεξε την κύρια σελίδα (`main`).
5. Δεξιά **Properties** → **Width** = `1024` · **Height** = `600`.
6. Save.

Αν βάλεις τη σελίδα μεγαλύτερη από το Display στις Settings, βγάζει σφάλμα ανάλυσης — πρέπει να ταιριάζουν και τα δύο.

---

## Firmware bring-up (ήδη OK)

| | |
|--|--|
| Project | `D:\ROMEOS 69\display-jc1060\` |
| BSP | Guition **Old_Panel** JD9165 (New_Panel = τρέμουλο) |
| Smoke | μαύρη οθόνη + κείμενο · καθαρή εικόνα 2026-07-29 |
| Mockup EEZ | ✓ 6 οθόνες · λειτουργικό home/menu/tiles 2026-08-02 |
| Factory backup | `..\JC1060P470C_I_W_Y\factory_flash_backup\` (τοπικά) |

Wi‑Fi μέσω ESP32-C6 (ESP-Hosted 2.12.11) · NTP · πράσινο icon — **OK στο lab 2026-08-02**.

**Pause 2026-08-02:** UI+Wi‑Fi OK · συνέχεια live data / EEZ polish άλλη μέρα.

---

## Kit / datasheet

`D:\ROMEOS 69\Othoni Levita\JC1060P470C_I_W_Y\`
