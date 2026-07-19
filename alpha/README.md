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

## Wokwi (προσομοίωση χωρίς πλακέτα)

Πρώτη φορά; δες [`../docs/WOKWI-START.md`](../docs/WOKWI-START.md).

```powershell
cd "D:\ROMEOS 69\alpha"
pio run -e alpha_rev_a_bench
# Άνοιξε diagram.json → F1 → Wokwi: Start Simulator
# BOOT στο εικονικό ESP32 = toggle heater (LED K4)
```

## Docs

- [`../docs/ALPHA-REV-A.md`](../docs/ALPHA-REV-A.md)
- [`../docs/WOKWI-START.md`](../docs/WOKWI-START.md)
- [`../docs/SIMULATION-BENCH.md`](../docs/SIMULATION-BENCH.md)

## Αρχεία

| Αρχείο | Ρόλος |
|--------|-------|
| `src/alpha_pins.h` | Pin map (CT GPIO35, ρελέ, UART) |
| `src/mb_mqtt.cpp` | MQTT state/cmd |
| `src/main.cpp` | Main loop |
