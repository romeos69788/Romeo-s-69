# Display — οθόνες θερμοστάτη

Firmware για **οθόνες / θερμοστάτες** (ESP32 + display).

## Σύνδεση με Alpha

- **Πρωτόκολλο:** UART / ESP-NOW
- **Pins (Alpha side):** UART2 TX=25, RX=33 · 115200 baud

## Κατάσταση

- [ ] PlatformIO project scaffold
- [ ] ESP-NOW / UDP sync setpoint από Alpha
- [ ] UI οθόνης (θερμοκρασίες, setpoint, Wi‑Fi/cloud icons)

## Remote (εκτός σπιτιού)

Setpoint αλλάζει και μέσω MQTT → [`../shared/romeos-remote/`](../shared/romeos-remote/)
