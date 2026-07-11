"""
Generate LVGL LV_IMG_CF_TRUE_COLOR_ALPHA C sources from
squareline-arc-import/5 Ikones/*.png into src/squareline/images/ui_img_day_p*.c.

Runtime uses convert_true_color_alpha_to_rgb565() in main_clean.cpp so the
image matches the Wi-Fi / background path on RGB565 MIPI.

Slot order matches day_phase_slot_from_h24() in main_clean.cpp.

After generating: add the five ui_img_day_p0..p4.c lines to platformio.ini
build_src_filter and remove +<squareline/images/ui_img_day_p_fallback_aliases.c>.
"""
from __future__ import annotations

from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SRC_DIR = ROOT / "squareline-arc-import" / "5 Ikones"
# Same folder + #include style as SquareLine exports (platformio build_src_filter).
OUT_DIR = ROOT / "src" / "squareline" / "images"

# Inset on THermostat AB artwork (user-measured window): same px as main_clean.cpp k_day_photo_clip_* /
# k_day_photo_asset_* — all five slot PNGs are resized to this exact size.
OUT_W = 250
OUT_H = 259

# Source files in slot order: 23-6, 6-8, 8-17, 17-20, 20-23
SLOTS = [
    ("23.00-6.00.png", "ui_img_day_p0"),
    ("6.00-8.00.png", "ui_img_day_p1"),
    ("8.00-17.00.png", "ui_img_day_p2"),
    ("17.00-20.00.png", "ui_img_day_p3"),
    ("20.00-23.00.png", "ui_img_day_p4"),
]


def rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def resize_cover(im: Image.Image, tw: int, th: int) -> Image.Image:
    """Scale uniformly to cover tw x th, center-crop."""
    im = im.convert("RGBA")
    sw, sh = im.size
    scale = max(tw / sw, th / sh)
    nw = max(1, int(round(sw * scale)))
    nh = max(1, int(round(sh * scale)))
    im = im.resize((nw, nh), Image.Resampling.LANCZOS)
    l = (nw - tw) // 2
    t = (nh - th) // 2
    return im.crop((l, t, l + tw, t + th))


def emit_c_alpha(name: str, w: int, h: int, data3: bytes, out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", encoding="ascii", newline="\n") as f:
        f.write(f"// Generated from 5 Ikones / {name}\n")
        f.write('#include "../ui.h"\n\n')
        f.write("#ifndef LV_ATTRIBUTE_MEM_ALIGN\n#define LV_ATTRIBUTE_MEM_ALIGN\n#endif\n\n")
        f.write(f"const LV_ATTRIBUTE_MEM_ALIGN uint8_t {name}_data[] = {{\n")
        for i in range(0, len(data3), 24):
            row = ", ".join(f"0x{v:02X}" for v in data3[i : i + 24])
            f.write("    " + row)
            if i + 24 < len(data3):
                f.write(",")
            f.write("\n")
        f.write("};\n\n")
        f.write(f"const lv_img_dsc_t {name} = {{\n")
        f.write("    .header.always_zero = 0,\n")
        f.write(f"    .header.w = {w},\n")
        f.write(f"    .header.h = {h},\n")
        f.write(f"    .data_size = sizeof({name}_data),\n")
        f.write("    .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,\n")
        f.write(f"    .data = {name}_data\n")
        f.write("};\n")


def main() -> None:
    tw, th = OUT_W, OUT_H
    for png_name, sym in SLOTS:
        src = SRC_DIR / png_name
        if not src.is_file():
            raise SystemExit(f"Missing source: {src}")
        im = Image.open(src)
        crop = resize_cover(im, tw, th)
        px = crop.load()
        blob = bytearray()
        for y in range(th):
            for x in range(tw):
                r, g, b, a = px[x, y]
                c = rgb565(r, g, b)
                blob.append(c & 0xFF)
                blob.append((c >> 8) & 0xFF)
                blob.append(a)
        out_c = OUT_DIR / f"{sym}.c"
        emit_c_alpha(sym, tw, th, bytes(blob), out_c)
        print(f"Wrote {out_c} ({tw}x{th} TRUE_COLOR_ALPHA, {len(blob)} bytes)")


if __name__ == "__main__":
    main()
