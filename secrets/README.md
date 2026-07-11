# Secrets — μην κάνεις commit

Αυτός ο φάκελος μένει **μόνο τοπικά** στο `D:\ROMEOS 69\secrets\`.

## Τι μπαίνει εδώ

| Αρχείο (παράδειγμα) | Περιεχόμενο |
|---------------------|-------------|
| `mqtt.env` | HiveMQ host, user, pass, device ID |
| `wifi.env` | SSID, password οικιακού Wi‑Fi |
| `platformio.local.ini` | Build flags με credentials (optional) |

## Πρότυπα

Αντίγραψε τα `.example` → αφαίρεσε το `.example` και συμπλήρωσε:

```powershell
cd "D:\ROMEOS 69\secrets"
copy mqtt.env.example mqtt.env
copy wifi.env.example wifi.env
```

## PlatformIO (προαιρετικά)

Δημιούργησε `secrets/platformio.local.ini`:

```ini
[env:alpha_rev_a]
build_flags =
    ${env:alpha_rev_a.build_flags}
    -DROMEOS_MQTT_HOST=\"xxxx.s1.eu.hivemq.cloud\"
    -DROMEOS_MQTT_USER=\"...\"
    -DROMEOS_MQTT_PASS=\"...\"
    -DROMEOS_WIFI_SSID=\"...\"
    -DROMEOS_WIFI_PASS=\"...\"
```

Και στο `alpha/platformio.ini` πρόσθεσε:

```ini
extra_configs = ../secrets/platformio.local.ini
```

(Το `platformio.local.ini` **δεν** πρέπει να μπει στο git.)
