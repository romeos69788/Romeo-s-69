# motherboard-fw

**Σημείο επαναφοράς συζήτησης (ξεκίνα από εδώ):** [`../HANDOFF_2026-04-20.md`](../HANDOFF_2026-04-20.md) · **Παλαιότερο handoff + §9:** [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) (Wi‑Fi home + failover, NVS, PlatformIO `package.json` / `build_src_filter`, **§7**, **§2.8** εφεδρικό = ίδιο firmware, link οθόνης).

Firmware για την **μητρική** (CONTROL BOARD, ESP32 **38 pin**), ξεχωριστά από το UI project **`thermostat-ui-demo`** (οθόνη Viewe).

## Χάρτης hardware

- **`include/pin_map.h`** — GPIO 1:1 με `thermostat-ui-demo/docs/romeos-design-notes.md` (**§12.2.2**, **§12.19.16**).
- **`../shared/include/romeos_link_types.h`** — κοινά πλαίσια με την οθόνη (Viewe project).
- Χειρόγραφο handoff: `../ξεκίνημα-συζήτησης/ξεκίνημα συζήτησης.md`.

## Wi‑Fi (οικία + ιδιωτικό link προς οθόνη)

- **Ιδιωτικό RF link:** SoftAP **`RomeosMB`** / κωδικός **`romeos2026`** (`shared/include/romeos_link_types.h` — άλλαξέ το με `-DROMEOS_MB_AP_PASS=\"...\"`).
- **Οικιακό STA (προαιρετικό):** NVS SSID/κωδικός μέσω `shared/` (`romeos_wifi_nvs_*`) — **ο κωδικός μπαίνει μόνο από Serial** (`WIFI_HOME_SET` → γραμμή SSID → γραμμή password), όχι σε `platformio.ini`. **Γρήγορα (Windows):** κλείσε Serial Monitor στο IDE, τρέξε [`../scripts/SET-WIFI-MOTHERBOARD-COM7.bat`](../scripts/SET-WIFI-MOTHERBOARD-COM7.bat) (ή `set-wifi-home.ps1 -Port COMx`). Βήμα-βήμα: [`../thermostat-ui-demo/README.md`](../thermostat-ui-demo/README.md) § «Πού μπαίνει ο κωδικός Wi‑Fi σπιτιού». Λίστα εντολών: [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) §9.3. Η μητρική δοκιμάζει πρώτα home· αν αποτύχει → SoftAP μόνο· αν το STA «πέσει» για ώρα → AP+STA (failover)· λεπτομέρειες: `src/mb_wifi_link.cpp`.
- **UDP:** **9000** λήψη εντολών, **9001** αποστολή κατάστασης· broadcast `.255` στο τρέχον subnet όταν υπάρχει STA στο LAN.
- **Setpoint σε NVS:** namespace `romeos_mb`, κλειδί `sp_x10` (δέκατα °C)· φορτώνεται στο boot, αποθηκεύεται όταν η οθόνη στέλνει *νέα* τιμή (`mb_wifi_link_get_setpoint_c_x10()`).

## MQTT (τηλεχειρισμός από κινητό εκτός σπιτιού)

Προαιρετικά: όταν ορίσεις `ROMEOS_MQTT_HOST`, `ROMEOS_MQTT_USER`, `ROMEOS_MQTT_PASS` στα `build_flags` (βλ. σχόλια στο `platformio.ini`), η μητρική συνδέεται με **MQTTS** στον broker και:

- δημοσιεύει JSON στο `romeos/<ROMEOS_MQTT_DEVICE_ID>/state` (~κάθε 2 s),
- ακούει εντολές στο `romeos/<ROMEOS_MQTT_DEVICE_ID>/cmd` (π.χ. `{"setpoint_c_x10": 190}`).

Η **web εφαρμογή** για iPhone βρίσκεται στο sibling φάκελο **[`../romeos-remote/`](../romeos-remote/)** (οδηγίες εκεί).

## Κοινό `shared/` στο build

- `platformio.ini`: `build_src_filter` → **`+<../../shared/src/romeos_wifi_nvs.cpp>`** (βάση: φάκελος **`src/`**).
- `build_flags`: `-I ../shared/include`
- **`MissingPackageManifestError` … `package.json`:** [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) §9.4 (`framework-arduinoespressif32-libs`).

## Build / upload

**Πρωτεύων και δευτερεύων (εφεδρικό module):** το **ίδιο** firmware (`esp32_motherboard`) — πλήρης λειτουργία: SoftAP `RomeosMB`, UDP προς/από την οθόνη, NVS setpoint. Στην πράξη **ένα** ESP32 είναι τοποθετημένο στη μητρική· σε βλάβη αφαιρείται και στη θέση του μπαίνει αμέσως το εφεδρικό, χωρίς αλλαγή είδους firmware.

```bash
cd motherboard-fw
pio run -e esp32_motherboard -t upload   # ίδια εντολή και για το spare (άλλο COM)
pio device monitor -b 115200
```

**Προτεινόμενη πρακτική:** κράτα το εφεδρικό **προ-φλασαρισμένο** με την ίδια έκδοση `.bin` όπως ο πρωτεύων. Μετά την ανταλλαγή, η οθόνη ξανασυνδέεται στο ίδιο SSID· το setpoint στο NVS του νέου module μπορεί να είναι προεπιλογή μέχρι να σταλεί ξανά από την οθόνη (ή να συγχρονίσεις χειροκίνητα αν χρειάζεται).

**Κρίσιμο:** μην τοποθετείς **δύο** ESP32 ταυτόχρονα πάνω στην **ίδια** μητρική (ίδια GPIO) — η αρχιτεκτονική είναι **ανταλλαγή module**, όχι δύο ενεργοί επεξεργαστές παράλληλα.

## Κατάσταση

- Σκελετός: Serial, I2C, SPI + CS, ρελέ σε HIGH, είσοδοι σε INPUT.
- **Οθόνη:** Wi‑Fi (home + `RomeosMB`) + UDP + NVS setpoint (`mb_wifi_link.cpp`) — δοκιμασμένο με `thermostat-ui-demo`· ενδείξεις Wi‑Fi στην οθόνη (HOME vs ESP32): [`../thermostat-ui-demo/README.md`](../thermostat-ui-demo/README.md), [`../HANDOFF_2026-04-13.md`](../HANDOFF_2026-04-13.md) §9.6.
- **Επόμενα firmware (πριν «κλείδωμα» PCB):** πραγματική ανάγνωση αισθητήρων στο `build_tx()`, λογική K1–K6 από setpoint/θερμοκρασίες, MCP41050 όπως στο doc.

---

## Προτεραιότητες & πλάνο (συμφωνημένο)

Σειρά που **κλειδώνει** τη μητρική πριν την εκτύπωση πλακέτας:

| # | Βήμα |
|---|------|
| 1 | **Breadboard / πλακέτα δοκιμής:** ESP38 στη δοκιμαστική, όλα τα εξαρτήματα και συνδέσεις όπως στο σχέδιο (`romeos-design-notes` §12.x). |
| 2 | **Αισθητήρες θερμοκρασίας:** κανονική σύνδεση (DS18B20, DHT, MAX31865 SPI, κ.λπ.) — έλεγχος τιμών στο Serial και στο πακέτο προς την οθόνη. |
| 3 | **Ρελέ / κυκλοφορητές:** για δοκιμή **LED + αντιστάσεις** (ή μόνο LED με περιοριστική) στις εξόδους ρελέ — ON = αναμμένο LED, OFF = σβηστό· χωρίς φορτία αντλίας μέχρι να επαληθευτεί η λογική. |
| 4 | **Είσοδοι:** FLOW, DEFROST, CT, backup, κ.λπ. — επαλήθευση επιπέδων όπως στο τελικό hardware. |
| 5 | **Λογική ελέγχου:** setpoint από οθόνη (`mb_wifi_link_get_setpoint_c_x10`) + θερμοκρασίες → εντολές K1–K6 (απλός υστερητής πρώτα). |
| 6 | **Κλείδωμα:** όταν όλα τα παραπάνω δοκιμάζονται σταθερά → **παγώνει** schematic/BOM/footprint και προχωρά η **εκτύπωση** μητρικής (χωρίς αναγκαστικές διορθώσεις μετά). |

**Συμφωνία:** Ναι — breadboard bring-up → πραγματικοί αισθητήρες θερμοκρασίας + **LED για οπτική επιβεβαίωση ρελέ** είναι σωστή πρακτική πριν το PCB final.

## Board

`board = esp32dev` (ESP32-WROOM-32). Αν χρειαστεί άλλο JSON για το ίδιο chip, προσθέτουμε custom board αργότερα.
