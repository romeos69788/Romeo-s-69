@echo off
chcp 65001 >nul
cd /d "%~dp0.."
echo romeos-display-v5 (Viewe S3) is usually COM3 — use SET-WIFI-DISPLAY-COM3.bat
echo Close Cursor Serial Monitor first, then press a key.
pause
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0set-wifi-home.ps1" -Port COM4 -Ssid "MERCUSYS_Romeos"
pause
