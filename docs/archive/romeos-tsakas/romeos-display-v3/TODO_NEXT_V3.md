# Επόμενα βήματα (v3)

- [ ] Μετά το upload: επιβεβαίωση **+/−** 0,5 °C και ότι το τόξο ακολουθεί χωρίς «σναπ πίσω» σε 24,0 (αν ναι → MB UDP / firmware μητρικής).
- [ ] (Προαιρετικό) Σύνδεση **εξωτερικής θερμοκρασίας / υγρασίας** στα labels του export από `romeos_mb_to_display_v1_t`.
- [ ] (Προαιρετικό) Ρύθμιση `k_mb_setpoint_holdoff_after_local_ms` (τώρα **60 s**· μικρότερο μόνο αν η μητρική στέλνει πάντα φρέσκο setpoint).
- [ ] Re-export SquareLine → αντιγραφή **`DISPLAY_A/*` → `src/squareline/`** + build.

Λεπτομέρειες: **`ROMEOS_DISPLAY_V3_STATUS.md`**. UI/LVGL για v4: **`ROMEOS_DISPLAY_V3_UI_SESSION.md`**.
