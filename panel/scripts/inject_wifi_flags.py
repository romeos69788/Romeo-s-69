# PlatformIO pre-script: inject home Wi‑Fi from ../secrets/wifi.env (gitignored).
Import("env")  # type: ignore  # noqa: F821

from pathlib import Path

secret = Path(env["PROJECT_DIR"]).parent / "secrets" / "wifi.env"  # type: ignore
if not secret.is_file():
    print(f"[wifi_flags] missing {secret} — clock stays placeholder")
else:
    kv = {}
    for line in secret.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        k, v = line.split("=", 1)
        kv[k.strip()] = v.strip().strip('"').strip("'")
    ssid = kv.get("SSID", "")
    pwd = kv.get("PASS", "")
    if ssid:
        env.Append(  # type: ignore
            CPPDEFINES=[
                ("ROMEOS_WIFI_SSID", env.StringifyMacro(ssid)),  # type: ignore
                ("ROMEOS_WIFI_PASS", env.StringifyMacro(pwd)),  # type: ignore
            ]
        )
        print(f"[wifi_flags] SSID={ssid} (pass len={len(pwd)})")
    else:
        print("[wifi_flags] wifi.env has no SSID")
