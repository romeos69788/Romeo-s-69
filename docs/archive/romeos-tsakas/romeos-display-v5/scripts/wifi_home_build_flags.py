"""Γράφει include/romeos_wifi_home_defaults.h από secrets/wifi_home.ini (αν υπάρχει)."""
Import("env")

from configparser import ConfigParser
from pathlib import Path


def escape_c_string(value: str) -> str:
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
    )


out_dir = Path(env["PROJECT_DIR"]) / "include"
out_dir.mkdir(parents=True, exist_ok=True)
out_path = out_dir / "romeos_wifi_home_defaults.h"

ini_path = Path(env["PROJECT_DIR"]) / "secrets" / "wifi_home.ini"
ssid = ""
password = ""

if ini_path.is_file():
    cfg = ConfigParser()
    cfg.read(ini_path, encoding="utf-8")
    if cfg.has_section("wifi"):
        ssid = cfg.get("wifi", "ssid", fallback="").strip()
        password = cfg.get("wifi", "password", fallback="").strip()

if ssid:
    print(f'[wifi_home] build defaults for SSID "{ssid}"')
else:
    print("[wifi_home] no home SSID in secrets — empty defaults header")

content = f"""#pragma once
/* Αυτόματα από scripts/wifi_home_build_flags.py — μην επεξεργάζεσαι χειροκίνητα. */
#define ROMEOS_WIFI_HOME_SSID_DEFAULT "{escape_c_string(ssid)}"
#define ROMEOS_WIFI_HOME_PASS_DEFAULT "{escape_c_string(password)}"
"""
out_path.write_text(content, encoding="utf-8")
env.Append(CPPPATH=[str(out_dir)])
