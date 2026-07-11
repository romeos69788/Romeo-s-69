# Ρωμαίος 69 — Αρχείο project & συζήτησης

Αυτός ο φάκελος περιέχει **όλα τα αρχεία** και **την καταγραφή της συζήτησης** (Cursor Cloud Agent, 11 Ιουλίου 2026).

## Περιεχόμενα

| Φάκελος / αρχείο | Περιγραφή |
|------------------|-----------|
| [`motherboard-fw/`](motherboard-fw/) | Firmware **Alpha rev A** (PlatformIO / ESP32) |
| [`romeos-remote/`](romeos-remote/) | Web app iPhone (React + MQTT) — snapshot από GitHub |
| [`ΣΥΖΗΤΗΣΗ.md`](ΣΥΖΗΤΗΣΗ.md) | Πλήρης καταγραφή της συζήτησης με τον agent |
| [`README-repo.md`](README-repo.md) | README του repo `Romeo-s-69` |
| [`ΣΥνδεσμοι.md`](ΣΥνδεσμοι.md) | GitHub repos, PR, cloud agent |

## Γρήγορη εκκίνηση

### Firmware (Alpha rev A)

```bash
cd motherboard-fw
pio run -e alpha_rev_a_bench
```

### Remote app (iPhone)

```bash
cd romeos-remote
npm install
npm run dev
```

## Hardware (Alpha rev A — σύνοψη)

- **CT:** μόνο SCT-013 20A/1V στη γραμμή HP → **GPIO35**
- **Όχι** δεύτερο CT (TUYA WiFi meter στον πίνακα καυστήρα)
- **Alpha ↔ Beta:** UART (TX=17, RX=16)
- **Οθόνες:** UART / ESP-NOW
- **RS485 (MAX485EPA):** reserved · όχι στο rev A

Λεπτομέρειες: [`motherboard-fw/docs/ALPHA-REV-A.md`](motherboard-fw/docs/ALPHA-REV-A.md)

## Επόμενα βήματα (TODO firmware)

- [ ] DS18B20 / NTC αισθητήρες
- [ ] UART Alpha ↔ Beta
- [ ] ESP-NOW / UDP sync προς οθόνες
- [ ] CT RMS calibration
- [ ] TUYA meter integration
- [ ] Λογική ελέγχου (pumps, HP, defrost)

---

*Δημιουργήθηκε αυτόματα από Cursor Cloud Agent · 2026-07-11*
