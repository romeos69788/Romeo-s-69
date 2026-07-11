# Οικιακό Wi‑Fi και NTP — romeos-display-v10

Ίδια πολιτική με `romeos-display-v5`. Το env είναι **`BOARD_VIEWE_V10`**.

## Σύνοψη (Μάιος 2026)

- Μόνο STA στο σπίτι (`ROMEOS_WIFI_HOME_ONLY=1`) — χωρίς SoftAP `RomeosMB` by default
- SSID παράδειγμα: **`MERCUSYS_Romeos`** (2.4 GHz)
- **Σπίτι**: γαλάζιο `#008cec` όταν `romeos_display_link_home_wifi_assoc()`
- **Λεβήτα** (status): γαλάζιο όταν `romeos_display_link_is_connected()` (UDP ζωντανό)
- **NTP**: μετά `WiFi OK`, ζώνη Ελλάδας

## Πού μπαίνει ο κωδικός

| Τρόπος | Περιγραφή |
|--------|-----------|
| **A. secrets** | `secrets/wifi_home.ini` → build script → NVS |
| **B. Serial** | `WIFI_HOME_SET` → SSID → κωδικός |
| **C. Script** | `..\..\scripts\SET-WIFI-DISPLAY-COM3.bat` |

```powershell
copy secrets\wifi_home.ini.example secrets\wifi_home.ini
# επεξεργασία ssid / password
pio run -e BOARD_VIEWE_V10 -t upload --upload-port COM3
```

## Serial (115200)

| Εντολή | Λειτουργία |
|--------|------------|
| `WIFI_HOME_HELP` | Βοήθεια |
| `WIFI_HOME_SET` | Αποθήκευση SSID + κωδικού |
| `WIFI_HOME_SHOW` | SSID (όχι κωδικός) |
| `WIFI_HOME_CLR` | Διαγραφή credentials |

## Έλεγχος

```
*** BUILD 2026-05-24-v10-mic-touch ***
[romeos_link] WiFi OK ssid="MERCUSYS_Romeos" ... ip=192.168.0.xxx
[romeos_link] HOME_ASSOC=yes
[romeos_ntp] SNTP started ...
```

`last_disc_reason=201` → AP δεν βρέθηκε (SSID / 2.4 GHz).

## Μητρική (UDP telemetry)

Στο `platformio.ini` (env `BOARD_VIEWE_V10`):

```ini
-DROMEOS_MB_LAN_IP=\"192.168.0.xxx\"
```

Πακέτο: `../../shared/include/romeos_link_types.h`

- `outdoor_c_x10` → εξωτερική θερμοκρασία UI (`obj8` / `obj31`)
- `flags & ROMEOS_MB_FLAG_TELEMETRY_VALID` απαιτείται για έγκυρες τιμές

Η οθόνη στέλνει setpoint + optional `room_from_display_c_x10` από SHT.

## UI widgets Wi‑Fi

| Κείμενο | Widget |
|---------|--------|
| Σπίτι | `obj26` (PRYMARY), `obj52` (MENU) |
| Λεβητα (label) | `obj27`, `obj53` |
| Εικονίδια | `wifi`, `wifi_1`, `boiler__`, `boiler_wifi` |
