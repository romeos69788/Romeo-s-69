# JC1060P470C — Romeos room thermostat

**Hardware:** ESP32-P4 · 1024×600 MIPI JD9165 (Old_Panel) · GT911 · Wi‑Fi via **ESP32-C6** (ESP-Hosted SDIO) · **COM14**

**Role:** room thermostat (not Alpha hub). Panel Viewe stays on **COM4**.

## Flash

```powershell
cd "D:\ROMEOS 69\display-jc1060"
# Wi‑Fi from ../secrets/wifi.env (gitignored)
pio run -e JC1060_P4 -t upload --upload-port COM14
```

Partition: `partitions_14MB_factory.csv` (factory app ~15 MB for images + C6 slave).

## Wi‑Fi / C6

- Pins: CLK=18, CMD=19, D0–D3=14–17, RESET=54  
- Host expects ESP-Hosted **2.12.11** · `jc1060_c6_ensure_slave()` can OTA from `data/c6_slave_2.12.11.bin`  
- SSID/PASS: `../secrets/wifi.env` via `scripts/inject_wifi_flags.py`  
- NTP Athens when connected · green Wi‑Fi icon on home

## UI

Custom LVGL (not raw EEZ C): `src/ui/screens.c` · backgrounds `data/img_*.bin` · Greek fonts `ui_font_*`.

- Home: temps, date/clock, setpoint arc (mid visual = 20 °C), MENU  
- Menu: tiles → detail screens 3–6 · BACK  
- Arc knob: `LV_PART_KNOB` pad=**10** + shadow

## Factory backup

`../Othoni Levita/JC1060P470C_I_W_Y/factory_flash_backup/` — P4 16 MB dump + Guition bins (local).

```powershell
python -m esptool --chip esp32p4 --port COM14 write-flash 0x0 "D:\ROMEOS 69\Othoni Levita\JC1060P470C_I_W_Y\factory_flash_backup\JC1060P470C_factory_dump_16MB_COM14.bin"
```
