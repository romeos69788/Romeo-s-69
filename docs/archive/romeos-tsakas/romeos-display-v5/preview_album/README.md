# Άλμπουμ backgrounds (προεπισκόπηση στην οθόνη)

Δοκιμή **πώς φαίνεται** κάθε background στην πραγματική οθόνη 7″ (φωτεινότητα/χρώμα — ίδιο RGB565 format με το τελικό EEZ project).

**Δεν αλλάζει** το κανονικό firmware `BOARD_VIEWE_OTHONI_7`.

## Βήματα

1. Βάλε τις εικόνες εδώ: **`preview_album/images/`**  
   - Μορφή: PNG ή JPG  
   - Μέγεθος: **800×480** (αν είναι άλλο, γίνεται resize στο build — καλύτερα ακριβώς 800×480)

2. Εγκατάσταση (μία φορά):
   ```powershell
   pip install pillow
   ```

3. Flash άλμπουμ (COM3):
   ```powershell
   cd "D:\Romeos Tsakas\romeos-display-v5"
   pio run -e BOARD_VIEWE_BG_PREVIEW -t upload --upload-port COM3
   ```

4. Στην οθόνη: **σύρε** ή **πάτα αριστερά/δεξιά μισή οθόνη** για προηγ./επόμ.  
   **Εναλλακτικά Serial 115200:** `n` ή `>` επόμενη · `p` ή `<` προηγ. · `1`…`9`/`0` για εικόνα 1–10.

5. Όταν διαλέξεις background → EEZ project με αυτή την εικόνα → ξανά **`BOARD_VIEWE_OTHONI_7`**.

## Επιστροφή στο κανονικό UI

```powershell
pio run -e BOARD_VIEWE_OTHONI_7 -t upload --upload-port COM3
```
