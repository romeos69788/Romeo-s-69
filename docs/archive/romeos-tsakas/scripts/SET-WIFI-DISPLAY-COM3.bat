@echo off
chcp 65001 >nul
cd /d "%~dp0.."
echo Close Cursor Serial Monitor first, then press a key.
pause
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0set-wifi-home.ps1" -Port COM3 -Ssid "MERCUSYS_Romeos"
pause
