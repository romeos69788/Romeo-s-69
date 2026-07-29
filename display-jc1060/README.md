# JC1060P470C — Romeos display bring-up

**Hardware:** ESP32-P4 · 1024×600 MIPI JD9165 · GT911 · **COM14**

## Factory backup (do not delete)

`../Othoni Levita/JC1060P470C_I_W_Y/factory_flash_backup/`

- `JC1060P470C_factory_dump_16MB_COM14.bin` — exact dump before Romeos flash  
- `official_burn_bins/` — Guition demo bins + `JC-C6-slave_*.bin`

Restore P4 dump:

```powershell
python -m esptool --chip esp32p4 --port COM14 write-flash 0x0 "D:\ROMEOS 69\Othoni Levita\JC1060P470C_I_W_Y\factory_flash_backup\JC1060P470C_factory_dump_16MB_COM14.bin"
```

## This firmware

Letterbox smoke test: full screen black · centered **800×480** stage (where v10 UI will sit).

```powershell
cd "D:\ROMEOS 69\display-jc1060"
pio run -e JC1060_P4 -t upload
```

**Note:** Viewe `romeos-display-v10` binary (ESP32-S3) cannot be flashed onto P4. This project is the P4 host; next step ports EEZ UI into the 800×480 stage (or redesign at 1024×600).
