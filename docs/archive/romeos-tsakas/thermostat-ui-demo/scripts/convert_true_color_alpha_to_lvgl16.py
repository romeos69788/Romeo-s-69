"""
Convert SquareLine LV_IMG_CF_TRUE_COLOR_ALPHA exports from 4 bytes/pixel (BGRA)
to 3 bytes/pixel (RGB565 LE + A) required when LV_COLOR_DEPTH == 16.

See LVGL lv_img_buf.h: LV_IMG_PX_SIZE_ALPHA_BYTE == 3 for depth 16.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

BYTES_PER_LINE = 24


def bgra_to_rgb565_le_a8(b: int, g: int, r: int, a: int) -> tuple[int, int, int]:
    full = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
    return (full & 0xFF, (full >> 8) & 0xFF, a & 0xFF)


def extract_hex_bytes(text: str) -> list[int]:
    return [int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]+)", text)]


def format_c_array(data: bytes, indent: str = "    ") -> str:
    lines = []
    for i in range(0, len(data), BYTES_PER_LINE):
        chunk = data[i : i + BYTES_PER_LINE]
        parts = ", ".join(f"0x{b:02X}" for b in chunk)
        lines.append(f"{indent}{parts},")
    return "\n".join(lines)


def convert_file(path: Path) -> None:
    text = path.read_text(encoding="utf-8")
    m = re.search(
        r"(const\s+LV_ATTRIBUTE_MEM_ALIGN\s+uint8_t\s+\w+_data\[\]\s*=\s*\{)([\s\S]*?)(\}\s*;)",
        text,
    )
    if not m:
        print(f"skip (no data array): {path}", file=sys.stderr)
        return

    raw = extract_hex_bytes(m.group(2))
    if len(raw) % 4 != 0:
        raise SystemExit(f"{path}: byte count {len(raw)} not multiple of 4")

    out = bytearray()
    for i in range(0, len(raw), 4):
        b, g, r, a = raw[i], raw[i + 1], raw[i + 2], raw[i + 3]
        lo, hi, aa = bgra_to_rgb565_le_a8(b, g, r, a)
        out.extend((lo, hi, aa))

    new_body = format_c_array(bytes(out))
    new_text = text[: m.start(2)] + "\n" + new_body + "\n" + text[m.end(2) :]

    if new_text != text:
        path.write_text(new_text, encoding="utf-8")
        print(f"OK {path.name}: {len(raw)} -> {len(out)} bytes")
    else:
        print(f"unchanged {path}")


def main() -> None:
    root = Path(__file__).resolve().parents[1] / "src" / "squareline" / "images"
    for p in sorted(root.glob("ui_img_*.c")):
        convert_file(p)


if __name__ == "__main__":
    main()
