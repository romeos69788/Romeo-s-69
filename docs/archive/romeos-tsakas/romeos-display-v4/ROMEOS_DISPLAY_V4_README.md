# romeos-display-v4

Καθαρό PlatformIO project για την **Viewe** (ίδιο board JSON με v3) + export SquareLine **«WITE THERMOSTAT»** (LVGL 8.3.x). **`LV_COLOR_DEPTH` πρέπει να είναι 16**: το RGB bus της πλακέτας είναι **RGB565** (`ESP_PANEL_LCD_COLOR_BITS_RGB565`)· αν το SquareLine είναι σε 32 bit, το firmware γράφει 4 byte/pixel ενώ η οθόνη περιμένει 2 → «χιόνι» / λάθος εικόνα. Σχεδίασε στο SquareLine με **16 bit** και κάνε export ώστε να ταιριάζει με `lv_conf.h`.

## Τι υπάρχει τώρα

| Τμήμα | Περιγραφή |
|--------|-----------|
| `src/main_v4.cpp` | Μόνο `Board` → `lvgl_port_init` → `ui_init()`. Χωρίς NTP, UDP, SHT, Preferences. |
| `src/lvgl_v8_port.*`, `lv_conf.h`, `esp_panel_*.h`, `esp_utils_conf.h` | Οδήγηση πίνακα / LVGL (ίδια στοίβα hardware με v3 — **όχι** αντιγραφή εφαρμοστικού κώδικα v3). |
| `src/board_override/` | Πίνακας Viewe. |
| `boards/BOARD_VIEWE_UEDX80480070E_WB_A.json` | Επιλογή board PlatformIO. |
| `src/squareline/` | Αντίγραφο του export από το φάκελο **`DISPLAY_Β`** στη ρίζα του repo (ή ισοδύναμο όνομα από SquareLine). |

## Export από SquareLine

1. Εξαγωγή στο τοπικό φάκελο (π.χ. `DISPLAY_Β`).
2. Αντέγραψε **όλα** τα περιεχόμενα στο **`src/squareline/`** (overwrite).
3. Κράτα στο `ui.h` το patch για LVGL include (`__has_include("lvgl.h")` …) — ή ξαναβάλτο μετά το export.
4. Αν λείπει το `lv_i18n.h`, κράτα το stub **`src/squareline/lv_i18n.h`** του project.
5. **`lv_conf.h`**: `LV_COLOR_DEPTH` **16** — ίδιο με το export SquareLine (**16 bit project**). Το `ui.c` κάνει `#error` αν διαφέρει.

## Partition table (16 MB)

Χρησιμοποιείται **`ROMEOS_V4_16MB_SINGLEAPP.csv`**: ένα μεγάλο `factory` app (~15,9 MB). Το stock `default_16MB.csv` δίνει δύο OTA slots ~6,25 MB το καθένα· με τις πολλές γραμματοσειρές του export το `.elf` ξεπερνά αυτό το όριο.

## Build & flash

```text
cd "D:\Romeos Tsakas\romeos-display-v4"
pio run -e BOARD_VIEWE_UEDX80480070E_WB_A
pio run -e BOARD_VIEWE_UEDX80480070E_WB_A -t upload
pio device monitor -e BOARD_VIEWE_UEDX80480070E_WB_A
```

Στο Serial (115200) στο boot: `*** ROMEOS-DISPLAY-V4 (Viewe) firmware ***`.

## Σημείωση για το όνομα φακέλου

Στο workspace το export μπορεί να εμφανίζεται ως `DISPLAY_Β` (ελληνικό **Β**). Αν γράφεις scripts, χρησιμοποίησε το πραγματικό path από το Explorer.

## Μητρική πλακέτα (σχέδιο πριν την παραγγελία)

Στο project υπάρχει ο φάκελος **`μητρική πλακέτα/`** για σημειώσεις και λίστα αλλαγών στη PCB **πριν** σταλεί για κατασκευή (ενδεικτικός χρόνος παράδοσης συχνά **~3 εβδομάδες**). Ξεκίνα από το `μητρική πλακέτα/README.md` και το **`μητρική πλακέτα/REVIEW_board_v1_2026-05-07.md`** (έλεγχος σχεδίου από screenshots).

## Μελλοντικά / σημειώσεις συζήτησης

- **Φωνητικές εντολές (προαιρετικά, ανά οθόνη):** κουμπί μικροφώνου, push‑to‑talk, ίδια εντολή με τα χειροκίνητα on/off — λεπτομέρειες και περιορισμούς (ελληνικά vs τοπική αναγνώριση) στο **`docs/discussion_notes_el.md`**.
