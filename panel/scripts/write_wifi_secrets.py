"""Copy house WiFi into gitignored secrets for panel build."""
from pathlib import Path
import configparser

src = Path(
    r"D:\ROMEOS 69\docs\archive\romeos-tsakas\romeos-display-v10"
    r"\romeos-display-v10\secrets\wifi_home.ini"
)
cp = configparser.ConfigParser()
cp.read(src, encoding="utf-8")
ssid = cp["wifi"]["ssid"].strip().strip('"')
pwd = cp["wifi"]["password"].strip().strip('"')

sec = Path(r"D:\ROMEOS 69\secrets")
sec.mkdir(exist_ok=True)
(sec / "wifi.env").write_text(f"SSID={ssid}\nPASS={pwd}\n", encoding="utf-8", newline="\n")

# Avoid ${...} expansion issues: list only the secret flags;
# panel/platformio.ini uses build_flags that PIO merges when using extra_configs carefully.
# Safer: write a dedicated env overlay that only adds flags via board_build / custom.
pio = (
    "; AUTO-GENERATED — DO NOT COMMIT\n"
    "[env:BOARD_VIEWE_PANEL]\n"
    "build_flags =\n"
    f'    -DROMEOS_WIFI_SSID=\\"{ssid}\\"\n'
    f'    -DROMEOS_WIFI_PASS=\\"{pwd}\\"\n'
)
(sec / "platformio.local.ini").write_text(pio, encoding="utf-8", newline="\n")
print("SSID=", ssid)
print("PASS_LEN=", len(pwd))
print("ok secrets written")
