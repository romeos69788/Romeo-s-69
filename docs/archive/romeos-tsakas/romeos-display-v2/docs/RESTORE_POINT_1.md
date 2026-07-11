# Restore Point 1

Date: 2026-04-28  
Project: `romeos-display-v2`  
Board target: `BOARD_VIEWE_UEDX80480070E_WB_A`

## Locked Baseline

This restore point captures the first stable visual milestone:

- UI renders cleanly on the display (no noisy veil / no major flicker).
- Main background is restored and visible.
- Core controls are working:
  - setpoint `+` / `-`
  - clock/date edit selection and adjustment
  - navigation button handlers (HEATING/LIGHTS/HEAT PUMP/BLINDS/ALARM hooks)
- Runtime marker is visible on-screen (`CLEAN-V2` with modification timestamp).

## If a future change breaks the UI

Use this restore point as the rollback reference and compare against:

- `src/main_clean.cpp`
- `src/squareline/screens/ui_THERMOSTAT_1.c`
- `src/squareline/ui.h`
- `src/lv_conf.h`

Recommended recovery flow:

1. Rebuild with current files.
2. If rendering is broken, restore the files above from this checkpoint state.
3. Rebuild and flash again.
4. Re-apply newer changes one by one.

**Newer checkpoint:** [`RESTORE_POINT_2.md`](./RESTORE_POINT_2.md) (2026-05-01 — v2 theme / press / focus / folder map).

