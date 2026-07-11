# Alpha — μητρική καυστήρα (rev A)

PlatformIO firmware για την **πλακέτα Alpha** στον καυστήρα.

## Build

```powershell
cd "D:\ROMEOS 69\alpha"
pio run -e alpha_rev_a_bench
pio run -t upload -e alpha_rev_a
```

## Credentials

Ρύθμισε WiFi/MQTT στο `..\secrets\` (δες `secrets/README.md`) ή μέσω `build_flags` στο `platformio.ini`.

## Docs

- [`../docs/ALPHA-REV-A.md`](../docs/ALPHA-REV-A.md)

## Αρχεία

| Αρχείο | Ρόλος |
|--------|-------|
| `src/alpha_pins.h` | Pin map (CT GPIO35, ρελέ, UART) |
| `src/mb_mqtt.cpp` | MQTT state/cmd |
| `src/main.cpp` | Main loop |
