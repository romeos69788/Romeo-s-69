# Shared — κοινός κώδικας

## Περιεχόμενα

| Φάκελος | Περιγραφή |
|---------|-----------|
| [`romeos-remote/`](romeos-remote/) | Web app iPhone (React + MQTT) |

## MQTT schema

Topics (συμβατά Alpha ↔ remote):

- `romeos/<DEVICE_ID>/state` — JSON τηλεμετρίας
- `romeos/<DEVICE_ID>/cmd` — `setpoint_c_x10`, `heater`

Ορισμός πεδίων: `romeos-remote/src/App.tsx` → `RomeosState`

## GitHub Pages

https://romeos69788.github.io/romeos-remote/
