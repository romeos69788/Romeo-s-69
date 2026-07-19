# Panel Alpha — Viewe 7″ hub preview

**Σκοπός:** Flash το EEZ hub (`Othoni_Levita`) στο **COM4** για να δεις πώς φαίνεται στην οθόνη. **Όχι** κλείδωμα οπτικού.

## Upload

```powershell
cd "D:\ROMEOS 69\panel"
pio run -e BOARD_VIEWE_PANEL -t upload
pio device monitor -e BOARD_VIEWE_PANEL
```

## Σημειώσεις

- Background: slim `ui_image_2.c` (RGB565 only) — τα πλήρη EEZ exports (~27 MB) δεν χωράνε στο flash.
- Χωρίς Wi‑Fi / mic / detail screens.
- Source EEZ: `../Othoni Levita/Othoni_Levita/`
