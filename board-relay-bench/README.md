# board-relay-bench

Γρήγορο τεστ LED/ρελέ για **CONTROL BOARD** (ESP32 · **COM7**).

- Active **LOW** = ρελέ/LED ON  
- Default pins: GPIO 25,26,27,32,33,17 (παλιό K1–K6 map)  
- Flash **εκτός** μητρικής αν GPIO12 τραβάει flash σε 1.8 V (βλ. session 2026-08-05)

```powershell
cd "D:\ROMEOS 69\board-relay-bench"
pio run -t upload --upload-port COM7
```

Χωρίς 230 V στα contacts.
