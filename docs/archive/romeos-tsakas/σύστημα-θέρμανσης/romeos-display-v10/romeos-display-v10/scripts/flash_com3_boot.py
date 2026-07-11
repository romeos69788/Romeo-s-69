#!/usr/bin/env python3
"""Put Viewe ESP32-S3 (CH340) in bootloader and run esptool upload."""
from __future__ import annotations

import subprocess
import sys
import time

try:
    import serial
except ImportError:
    print("pip install pyserial", file=sys.stderr)
    sys.exit(1)

PORT = "COM3"
BAUD = 115200
FIRMWARE = ".pio/build/BOARD_VIEWE_V10/firmware.bin"


def enter_bootloader(port: str) -> None:
    """Classic ESP32 auto-reset: RTS=GPIO0, DTR=EN."""
    ser = serial.Serial(port, BAUD, timeout=0.1)
    ser.dtr = False
    ser.rts = True
    time.sleep(0.05)
    ser.dtr = True
    time.sleep(0.05)
    ser.dtr = False
    time.sleep(0.15)
    ser.close()


def main() -> int:
    print(f"[flash] boot sequence on {PORT} …")
    enter_bootloader(PORT)
    cmd = [
        sys.executable,
        "-m",
        "esptool",
        "--chip",
        "esp32s3",
        "--port",
        PORT,
        "--baud",
        str(BAUD),
        "--before",
        "no_reset",
        "--after",
        "hard_reset",
        "write_flash",
        "-z",
        "--flash_mode",
        "qio",
        "--flash_freq",
        "80m",
        "--flash_size",
        "16MB",
        "0x0",
        FIRMWARE,
    ]
    print("[flash]", " ".join(cmd))
    return subprocess.call(cmd)


if __name__ == "__main__":
    raise SystemExit(main())
