#!/usr/bin/env python3
"""Μετατρέπει PNG/JPG από preview_album/images/ σε LVGL RGB565 C sources."""
from __future__ import annotations

import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("ERROR: pip install pillow", file=sys.stderr)
    raise SystemExit(1) from None

PROJECT_DIR = Path(__file__).resolve().parents[1]
SRC_DIR = PROJECT_DIR / "src" / "preview_album" / "generated"
INPUT_DIR = PROJECT_DIR / "preview_album" / "images"
TARGET_W = 800
TARGET_H = 480


def rgb888_to_rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def image_to_c_array(img: Image.Image, symbol: str) -> str:
    if img.size != (TARGET_W, TARGET_H):
        img = img.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)
    if img.mode != "RGB":
        img = img.convert("RGB")

    pixels = img.load()
    words = []
    for y in range(TARGET_H):
        for x in range(TARGET_W):
            r, g, b = pixels[x, y]
            v = rgb888_to_rgb565(r, g, b)
            # LVGL TRUE_COLOR RGB565: little-endian bytes στο uint8_t map[]
            words.append(f"0x{v & 0xFF:02x}")
            words.append(f"0x{(v >> 8) & 0xFF:02x}")

    data_symbol = f"{symbol}_map"
    lines = [
        "#include <lvgl.h>",
        "",
        f"const LV_ATTRIBUTE_MEM_ALIGN uint8_t {data_symbol}[] = {{",
    ]
    for i in range(0, len(words), 8):
        lines.append("    " + ", ".join(words[i : i + 8]) + ",")
    lines.extend(
        [
            "};",
            "",
            f"const lv_img_dsc_t {symbol} = {{",
            "    .header.cf = LV_IMG_CF_TRUE_COLOR,",
            f"    .header.w = {TARGET_W},",
            f"    .header.h = {TARGET_H},",
            f"    .data_size = sizeof({data_symbol}),",
            f"    .data = {data_symbol},",
            "};",
            "",
        ]
    )
    return "\n".join(lines)


def safe_symbol(_stem: str, index: int) -> str:
    """ASCII-only C filenames (Windows toolchain + ελληνικά ονόματα PNG)."""
    return f"preview_{index:02d}"


def main() -> int:
    SRC_DIR.mkdir(parents=True, exist_ok=True)
    INPUT_DIR.mkdir(parents=True, exist_ok=True)

    inputs = sorted(
        [
            p
            for p in INPUT_DIR.iterdir()
            if p.is_file() and p.suffix.lower() in (".png", ".jpg", ".jpeg", ".bmp")
        ],
        key=lambda p: p.name.lower(),
    )

    import shutil

    if SRC_DIR.is_dir():
        shutil.rmtree(SRC_DIR, ignore_errors=True)
    SRC_DIR.mkdir(parents=True, exist_ok=True)

    if not inputs:
        print(f"[preview_album] No images in {INPUT_DIR}")
        (SRC_DIR / "preview_album_data.c").write_text(
            '#include "preview_album_data.h"\n\n'
            "const lv_img_dsc_t *const preview_album_images[] = {0};\n"
            "const char *const preview_album_names[] = {0};\n"
            "const size_t preview_album_count = 0;\n",
            encoding="utf-8",
        )
        (SRC_DIR / "preview_album_data.h").write_text(
            "#pragma once\n#include <lvgl.h>\n#include <stddef.h>\n\n"
            "extern const lv_img_dsc_t *const preview_album_images[];\n"
            "extern const char *const preview_album_names[];\n"
            "extern const size_t preview_album_count;\n",
            encoding="utf-8",
        )
        return 0

    symbols: list[str] = []
    names: list[str] = []
    for i, path in enumerate(inputs):
        sym = safe_symbol(path.stem, i)
        symbols.append(sym)
        names.append(path.name)
        (SRC_DIR / f"{sym}.c").write_text(
            image_to_c_array(Image.open(path), sym), encoding="utf-8"
        )
        print(f"[preview_album] {path.name} -> {sym}.c")

    decls = "\n".join(f"extern const lv_img_dsc_t {s};" for s in symbols)
    (SRC_DIR / "preview_album_data.h").write_text(
        f"#pragma once\n#include <lvgl.h>\n#include <stddef.h>\n\n{decls}\n\n"
        "extern const lv_img_dsc_t *const preview_album_images[];\n"
        "extern const char *const preview_album_names[];\n"
        "extern const size_t preview_album_count;\n",
        encoding="utf-8",
    )
    img_ptrs = ",\n    ".join(f"&{s}" for s in symbols)
    name_ptrs = ",\n    ".join(f'"{n}"' for n in names)
    (SRC_DIR / "preview_album_data.c").write_text(
        f'#include "preview_album_data.h"\n\n'
        f"const lv_img_dsc_t *const preview_album_images[] = {{\n    {img_ptrs}\n}};\n\n"
        f"const char *const preview_album_names[] = {{\n    {name_ptrs}\n}};\n\n"
        f"const size_t preview_album_count = {len(symbols)};\n",
        encoding="utf-8",
    )
    order_path = PROJECT_DIR / "preview_album" / "IMAGE_ORDER.txt"
    lines = [
        "# Αυτόματα από convert_preview_album.py — σειρά αλφαβητική ονομάτων αρχείων",
        f"# Σύνολο: {len(symbols)} εικόνες (preview_00 … preview_{len(symbols) - 1:02d})",
        "",
    ]
    for i, name in enumerate(names):
        lines.append(f"{i + 1:02d}  preview_{i:02d}  {name}")
    order_path.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"[preview_album] {len(symbols)} image(s) ready for flash")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
