#!/usr/bin/env python3
"""
Remove light 'pill' shapes on the right status bars by inpainting from the dark track.

Uses per-row reference luminance from a column inside the track (right of the pills), so
dimmer pills and the bottom row are caught — not only pixels above a fixed global threshold.

If ui_reference.png is missing, pixels are decoded from src/ui_reference_img.c (RGB565 LE).
"""
from __future__ import annotations

import argparse
import re
import struct
import subprocess
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    raise SystemExit("pip install Pillow")

ROOT = Path(__file__).resolve().parent.parent
PNG = ROOT / "ui_reference.png"
LVGL_C = ROOT / "src" / "ui_reference_img.c"
PNG_TO_LVGL = ROOT / "scripts" / "png_to_lvgl_true_color.py"


def rgb565_le_to_rgb(lo: int, hi: int) -> tuple[int, int, int]:
    v = lo | (hi << 8)
    r = ((v >> 11) & 0x1F) << 3
    g = ((v >> 5) & 0x3F) << 2
    b = (v & 0x1F) << 3
    return r, g, b


def load_rgb_from_lvgl_c(path: Path) -> Image.Image:
    text = path.read_text(encoding="utf-8", errors="replace")
    m = re.search(r"\.w\s*=\s*(\d+)", text)
    w = int(m.group(1)) if m else 800
    m = re.search(r"\.h\s*=\s*(\d+)", text)
    h = int(m.group(1)) if m else 480
    # Only the pixel array uses 0xNN tokens; keep parse deterministic.
    start = text.find("ui_reference_img_map[]")
    if start == -1:
        raise SystemExit(f"Could not find ui_reference_img_map[] in {path}")
    block = text[start:]
    end = block.find("\n};")
    if end == -1:
        raise SystemExit(f"Could not find end of byte array in {path}")
    block = block[:end]
    bytes_iter = re.findall(r"0x([0-9a-fA-F]{2})", block)
    raw = bytes(int(b, 16) for b in bytes_iter)
    expected = w * h * 2
    if len(raw) < expected:
        raise SystemExit(f"Expected at least {expected} bytes in {path}, got {len(raw)}")
    raw = raw[:expected]
    im = Image.new("RGB", (w, h))
    px = im.load()
    i = 0
    for y in range(h):
        for x in range(w):
            lo, hi = raw[i], raw[i + 1]
            i += 2
            px[x, y] = rgb565_le_to_rgb(lo, hi)
    return im


def lum(rgb: tuple[int, int, int]) -> float:
    r, g, b = rgb
    return 0.299 * r + 0.587 * g + 0.114 * b


def erase_pills(
    im: Image.Image,
    *,
    x0: int,
    x1: int,
    y0: int,
    y1: int,
    ref_x: int,
    delta_lum: float,
    abs_lum_cap: float | None,
) -> Image.Image:
    im = im.convert("RGB")
    px = im.load()
    w, h = im.size
    ref_x = min(max(0, ref_x), w - 1)

    for y in range(y0, min(y1, h)):
        tr, tg, tb = px[ref_x, y]
        ref_l = lum((tr, tg, tb))
        for x in range(x0, min(x1, w)):
            r, g, b = px[x, y]
            l = lum((r, g, b))
            bright_vs_track = l - ref_l
            if bright_vs_track > delta_lum:
                px[x, y] = (tr, tg, tb)
            elif abs_lum_cap is not None and l > abs_lum_cap:
                px[x, y] = (tr, tg, tb)
    return im


def main() -> None:
    ap = argparse.ArgumentParser(description="Inpaint right-bar pills from track color.")
    ap.add_argument(
        "--no-lvgl",
        action="store_true",
        help="Only update ui_reference.png; do not run png_to_lvgl_true_color.py",
    )
    ap.add_argument("--delta-lum", type=float, default=14.0, help="Lum above track row to replace")
    ap.add_argument(
        "--abs-lum",
        type=float,
        default=88.0,
        help="Also replace pixels above this absolute luminance (None to disable)",
    )
    args = ap.parse_args()
    abs_cap: float | None = args.abs_lum

    if PNG.exists():
        im = Image.open(PNG).convert("RGB")
    elif LVGL_C.exists():
        print(f"{PNG} missing — decoding from {LVGL_C}", file=sys.stderr)
        im = load_rgb_from_lvgl_c(LVGL_C)
    else:
        raise SystemExit(f"Need {PNG} or {LVGL_C}")

    w, h = im.size
    # Slightly wider / taller than first pass: pills and row spacing vary; last row sits lower.
    x0, x1 = 418, 578
    y0, y1 = 78, min(412, h)
    ref_x = min(698, w - 1)

    im = erase_pills(
        im,
        x0=x0,
        x1=x1,
        y0=y0,
        y1=y1,
        ref_x=ref_x,
        delta_lum=args.delta_lum,
        abs_lum_cap=abs_cap,
    )

    im.save(PNG)
    print(f"Updated {PNG} (pills inpainted, ref column x={ref_x}, region y={y0}..{y1})")

    if not args.no_lvgl:
        cmd = [sys.executable, str(PNG_TO_LVGL), str(PNG), str(ROOT / "src" / "ui_reference_img.c")]
        print("Running:", " ".join(cmd))
        subprocess.check_call(cmd)


if __name__ == "__main__":
    main()
