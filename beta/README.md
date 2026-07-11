# Beta — πλακέτα εξωτερικού (outdoor relay board)

Firmware / hardware για την **πλακέτα Beta** στον εξωτερικό χώρο (HP / inverter outdoor unit).

## Ρόλος rev A

**Κυρίως ρελέ** — contactors, OEM heat/cool lines, pump/servo τροφοδοσία HP.

**Όχι στην Beta (μεταφέρθηκαν / μένουν στην Alpha):**

| Λειτουργία | Πού rev A |
|------------|-----------|
| DEFROST | **Alpha** — **CN_DEFROST** → GPIO14 |
| DS18 νερού, SHT40, MCP41050 | **Alpha** |
| Λογική ελέγχου / MQTT | **Alpha** ESP32 |

## Σύνδεση με Alpha

- **Πρωτόκολλο:** UART (προαιρετικό status / εντολές)
- **Pins (Alpha side):** TX=17, RX=16 · 115200 baud
- **Όχι** τροφοδοσία Beta από Alpha — η Beta έχει δικό της 5 V IN

## Κατάσταση

- [ ] PlatformIO project scaffold
- [ ] UART πρωτόκολλο Alpha ↔ Beta (αν χρειαστεί)
- [ ] Ρελέ HP / OEM wiring

## Docs

- [`../docs/ALPHA-REV-A.md`](../docs/ALPHA-REV-A.md)
- [`../docs/ALPHA-REV-A-PCB-REVIEW.md`](../docs/ALPHA-REV-A-PCB-REVIEW.md)
