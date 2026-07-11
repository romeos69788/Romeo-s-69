# Romeo-s-69

**ROMEOS 69** — σύστημα ελέγχου θέρμανσης / αντλίας θερμότητας.

## Repos

| Repo | Ρόλος |
|------|-------|
| **Romeo-s-69** (αυτό) | Firmware μητρικής **Alpha rev A** |
| [romeos-remote](https://github.com/romeos69788/romeos-remote) | Web app iPhone (MQTT remote) |

## Alpha rev A

Μητρική ESP32 στον καυστήρα · firmware στο `motherboard-fw/`.

```bash
cd motherboard-fw
pio run -e alpha_rev_a_bench
```

Λεπτομέρειες hardware: [`motherboard-fw/docs/ALPHA-REV-A.md`](motherboard-fw/docs/ALPHA-REV-A.md)

## Αρχιτεκτονική

```
┌─────────────┐   UART    ┌─────────────┐
│ Alpha (MB)  │◄─────────►│ Beta        │
│ boiler room │           │ outdoor     │
└──────┬──────┘           └─────────────┘
       │ UART / ESP-NOW
       ▼
┌─────────────┐   MQTT      ┌─────────────┐
│ Displays    │             │ romeos-     │
│ (thermostat)│             │ remote      │
└─────────────┘             └──────┬──────┘
                                   │ HiveMQ Cloud
                                   ▼
                            ┌─────────────┐
                            │ Alpha (MB)  │
                            └─────────────┘
```

## Hardware notes

- **CT:** μόνο SCT-013 στη γραμμή HP → GPIO35
- **TUYA WiFi meter** στον πίνακα καυστήρα (όχι CT)
- **RS485 / MAX485EPA:** reserved για future HP outdoor board
