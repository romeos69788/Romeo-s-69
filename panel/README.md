# Panel Alpha — Viewe 7″ hub

**ΚΛΕΙΔΩΜΕΝΟ οπτικό hub:** έκδοση **4** (`Othoni_Levita_4`) · 2026-07-23

## Upload

```powershell
cd "D:\ROMEOS 69\panel"
# Wi‑Fi: D:\ROMEOS 69\secrets\wifi.env  (gitignored · από wifi_home.ini σπιτιού)
pio run -e BOARD_VIEWE_PANEL -t upload
pio device monitor -e BOARD_VIEWE_PANEL
```

## Σημειώσεις

- Background: slim `ui_image_6.c` (RGB565)
- EEZ: `../Othoni Levita/Othoni_Levita_1/Othoni_Levita_4.eez-project`
- Docs: [`docs/panel-hub/HUB-VERSIONS.md`](../docs/panel-hub/HUB-VERSIONS.md)
- Live clock: home Wi‑Fi + NTP (Ελλάδα) → πάνω μπάρα
