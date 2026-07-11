# Quick ref — romeos-display-v3

```text
Οθόνη: COM3   |   Μητρική: COM7 (άλλο project)
```

```bash
pio run -e BOARD_VIEWE_UEDX80480070E_WB_A -t upload
pio device monitor -e BOARD_VIEWE_UEDX80480070E_WB_A
```

- Έλεγχος ότι τρέχει v3: στο **Serial** boot εμφανίζεται `*** ROMEOS-DISPLAY-V3 ... ***` (αν βλέπεις `CLEAN-V2` στην οθόνη, είναι παλιό flash σε άλλο COM).
- Λεπτομέρειες: **`ROMEOS_DISPLAY_V3_STATUS.md`**.
- Τι κάναμε στο UI (για επόμενο export v4): **`ROMEOS_DISPLAY_V3_UI_SESSION.md`**.
