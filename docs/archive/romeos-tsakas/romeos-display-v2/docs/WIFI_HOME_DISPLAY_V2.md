# Wi‑Fi / NVS / HOME — `romeos-display-v2` (ενημέρωση 2026-04-30)

Σύνοψη για συνέχεια: τι ισχύει στο firmware της **δεύτερης οθόνης** (Viewe, `main_clean.cpp` + `romeos_display_link.cpp`).

## Υλικό / θύρα

- **CH340** συχνά **COM3** (Windows)· πριν scripted serial ή upload, **κλείσε** το PlatformIO **Device Monitor** (αλλιώς «port busy»).
- Το ESP32-S3 έχει Wi‑Fi **μόνο 2.4 GHz**· SSID που εκπέμπει **μόνο** 5 GHz δεν συνδέεται.

## NVS home (`shared` — `romeos_wifi_nvs`)

Εντολές serial **115200**, τέλος γραμμής **LF**:

| Εντολή | Σκοπός |
|--------|--------|
| `WIFI_HOME_SET` | Μετά: γραμμή **SSID**, μετά γραμμή **κωδικός** (ή κενή γραμμή για open AP). |
| `WIFI_HOME_SHOW` | Εμφανίζει αποθηκευμένο SSID (όχι κωδικό). |
| `WIFI_HOME_CLR` | Διαγραφή home από NVS. |
| `WIFI_HOME_HELP` | Βοήθεια. |

Μετά από επιτυχή αποθήκευση, το `romeos_display_link_on_home_wifi_saved_to_nvs()` ξαναφορτώνει NVS και καλεί `WiFi.begin` **χωρίς υποχρεωτικό reboot** (κλήση από `main_clean.cpp` / `main.cpp` όταν `romeos_wifi_nvs_poll_serial()` επιστρέφει `true`).

## Open guest (Mercusys κ.λπ.)

Αν το AP είναι **Security: None (open)**:

- Στο `WIFI_HOME_SET`, η γραμμή κωδικού πρέπει να είναι **κενή** (μόνο Enter).
- Αν αποθηκεύσεις **κωδικό** σε open AP, το STA συχνά αποτυγχάνει με **`last_disc_reason=210`** (`WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY` — «δεν βρέθηκε AP με συμβατή ασφάλεια»): το ESP προσπαθεί WPA2 ενώ το AP είναι open.

Για κανονικό δίκτυο με WPA2, χρησιμοποίησε τη γραμμή κωδικού όπως στο router.

## Πολιτική `romeos_display_link` (όχι standalone)

- Αν υπάρχουν **home credentials στο NVS**, **δεν** γίνεται πλέον αυτόματο **fallback στο `RomeosMB`** μετά από timeout (αποφυγή ατελούς reconnect όταν το RomeosMB δεν υπάρχει στο σπίτι).
- Σύγκριση «είμαι στο home SSID;» με **`esp_wifi_sta_get_ap_info()`** (πιο αξιόπιστο από `WiFi.SSID()` μόνο του).
- Κάθε **30 s** offline εκτυπώνεται γραμμή `home still offline ... last_disc_reason=...` για διάγνωση.

## HOME UI

- Πράσινο / animation όταν `romeos_display_link_home_wifi_assoc()` είναι true (STA στο αποθηκευμένο SSID).
- Υλοποίηση τόξων: `main_clean.cpp` (`HomeWifiAnim`, `lv_obj_remove_style_all` στα arcs για artifacts).

## NTP / ρολόι

Με επιτυχή STA και διαδρομή προς internet, `romeos_ntp` ξανατρέχει — εμφάνιση `[romeos_ntp] wall clock set from NTP` στο serial.

## Μητρική (UDP)

Με home Wi‑Fi και **χωρίς** `ROMEOS_MB_LAN_IP` στο build, εμφανίζεται προειδοποίηση· για UDP προς τη μητρική στο LAN ορίζεται `-DROMEOS_MB_LAN_IP=\"...\"` στο `platformio.ini` της οθόνης (`shared/include/romeos_link_types.h`).

## Επιβεβαιωμένη ρύθμιση (Mercusys MR30G, 2026-04-30)

- **Guest 2.4 GHz** `MEGuest_82C2`, **open** → NVS με **κενό** password· STA OK, `HOME_ASSOC=yes`.
- Κύριο δίκτυο `MERCUSYS_Romeos` (Smart Connect), WPA2 — για IoT προτιμάται σταθερό **2.4 GHz** SSID αν το Smart Connect στέλνει συσκευές μόνο στο 5 GHz.
