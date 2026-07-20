# Panel Alpha — Viewe 7″ hub

**ΚΛΕΙΔΩΜΕΝΟ οπτικό hub:** έκδοση **3** (`Othoni_Levita_2`) · 2026-07-20

Flash COM4 για το κλειδωμένο hub.

## Upload

```powershell
cd "D:\ROMEOS 69\panel"
pio run -e BOARD_VIEWE_PANEL -t upload
pio device monitor -e BOARD_VIEWE_PANEL
```

## Σημειώσεις

- Background: slim `ui_image_4.c` (RGB565) — τα πλήρη EEZ exports (~27 MB) δεν χωράνε στο flash.
- EEZ: `../Othoni Levita/Othoni_Levita_1/Othoni_Levita_2.eez-project`
- Docs: [`docs/panel-hub/HUB-VERSIONS.md`](../docs/panel-hub/HUB-VERSIONS.md)
