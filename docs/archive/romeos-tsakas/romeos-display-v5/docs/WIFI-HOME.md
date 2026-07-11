# Οικιακό Wi‑Fi και NTP — romeos-display-v5

## Σύνοψη (Μάιος 2026)

- Firmware: `BOARD_VIEWE_OTHONI_7`, build banner `BUILD 2026-05-21-home-wifi-auto`
- Μόνο STA στο σπίτι (`ROMEOS_WIFI_HOME_ONLY=1`) — χωρίς SoftAP `RomeosMB`
- SSID στο router: **`MERCUSYS_Romeos`** (2.4 GHz — το ESP32 **δεν** βλέπει μόνο 5 GHz)
- Εικονίδιο **Σπιτι**: γαλάζιο όταν `HOME_ASSOC=yes` (σύνδεση στο αποθηκευμένο SSID)
- **NTP**: `pool.ntp.org` + `time.google.com`, ζώνη `EET-2/EEST-3` μετά από `WL_CONNECTED`

## Πού μπαίνει ο κωδικός (όχι στο git)

| Τρόπος | Περιγραφή |
|--------|-----------|
| **A. Τοπικό secrets** | `secrets/wifi_home.ini` → γράφεται στο NVS στο πρώτο boot (build script) |
| **B. Serial** | `WIFI_HOME_SET` → γραμμή SSID → γραμμή κωδικού |
| **C. Script** | `..\..\scripts\set-wifi-home.ps1 -Port COM3` |

Το `secrets/wifi_home.ini` και το `include/romeos_wifi_home_defaults.h` είναι στο `.gitignore`.

### Ρύθμιση secrets (νέο PC / νέο flash)

```powershell
copy secrets\wifi_home.ini.example secrets\wifi_home.ini
# Επεξεργασία wifi_home.ini — ssid και password (μόνο τοπικά)
pio run -e BOARD_VIEWE_OTHONI_7 -t upload --upload-port COM3
```

## Serial (115200, LF)

| Εντολή | Λειτουργία |
|--------|------------|
| `WIFI_HOME_HELP` | Βοήθεια |
| `WIFI_HOME_SET` | Αποθήκευση SSID + κωδικού (2 γραμμές μετά) |
| `WIFI_HOME_SHOW` | Εμφάνιση αποθηκευμένου SSID (όχι κωδικός) |
| `WIFI_HOME_CLR` | Διαγραφή home credentials |

Μετά το save, το νέο firmware **εφαρμόζει** STA χωρίς reboot (`romeos_display_link_on_home_wifi_saved_to_nvs`).

## Έλεγχος ότι δουλεύει

Στο Serial Monitor μετά το boot:

```
*** BUILD 2026-05-21-home-wifi-auto ***
[romeos_link] WiFi OK ssid="MERCUSYS_Romeos" ... ip=192.168.0.xxx
[romeos_link] HOME_ASSOC=yes
[romeos_ntp] SNTP started ...
```

Σφάλμα `last_disc_reason=201` → **δεν βρίσκει AP**: λάθος SSID, ή δίκτυο μόνο 5 GHz, ή router κλειστό.

## Μητρική (UDP, όταν υπάρχει)

Στο `platformio.ini` της οθόνης:

```ini
-DROMEOS_MB_LAN_IP=\"192.168.0.xxx\"
```

Η IP είναι της **μητρικής** στο ίδιο subnet — **όχι** η IP του PC (π.χ. όχι `.106`).

## Αρχεία που αφορούν Wi‑Fi

| Αρχείο | Ρόλος |
|--------|------|
| `scripts/wifi_home_build_flags.py` | Δημιουργεί `include/romeos_wifi_home_defaults.h` |
| `src/romeos_wifi_nvs.cpp` | NVS + Serial provisioning |
| `src/romeos_display_link.cpp` | STA, UDP, `HOME_ASSOC` |
| `src/romeos_ntp.cpp` | SNTP |
| `src/romeos_eez_runtime.cpp` | UI «Σπιτι», ρολόι |
