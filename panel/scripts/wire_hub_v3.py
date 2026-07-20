"""Wire Othoni_Levita_2 (hub V3) into panel/ — slim img_4 only."""
from __future__ import annotations

import re
import shutil
from pathlib import Path

SRC = Path(r"D:\ROMEOS 69\Othoni Levita\Othoni_Levita_1\src\ui")
DST = Path(r"D:\ROMEOS 69\panel\src\ui")
SLIM = Path(r"D:\ROMEOS 69\panel\scripts\slim_eez_image.py")


def slim_image(src: Path, dst: Path, map_name: str, img_name: str) -> None:
    text = src.read_text(encoding="utf-8", errors="replace")
    pattern = r"#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0\n(.*?)#endif"
    m = re.search(pattern, text, re.DOTALL)
    if not m:
        raise SystemExit(f"No 16-bit block in {src}")
    pixel_block = m.group(1)
    dsc_m = re.search(
        rf"(const lv_img_dsc_t {re.escape(img_name)} = \{{.*?\n\}}\;)",
        text,
        re.DOTALL,
    )
    if not dsc_m:
        raise SystemExit(f"No descriptor for {img_name}")
    dsc = dsc_m.group(1)
    needle = f"uint8_t {map_name}[] = {{"
    arr_start = text.index(needle)
    preamble = text[: arr_start + len(needle)]
    slim = preamble + "\n" + pixel_block.rstrip() + "\n};\n\n" + dsc + "\n"
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_text(slim, encoding="utf-8", newline="\n")
    print(f"slim {src.name}: {src.stat().st_size} -> {dst.stat().st_size}")


def main() -> None:
    slim_image(SRC / "ui_image_4.c", DST / "ui_image_4.c", "img_4_map", "img_4")

    copy_names = [
        "actions.h",
        "fonts.h",
        "screens.c",
        "screens.h",
        "structs.h",
        "styles.c",
        "styles.h",
        "ui.c",
        "ui.h",
        "vars.h",
        "ui_font_25.c",
        "ui_font_26.c",
        "ui_font_28.c",
        "ui_font_30.c",
    ]
    for name in copy_names:
        shutil.copy2(SRC / name, DST / name)
        print("copied", name)

    for old in ("ui_image_2.c", "ui_image_3.c"):
        p = DST / old
        if p.exists():
            p.unlink()
            print("removed", old)

    sc = (DST / "screens.c").read_text(encoding="utf-8")
    sc2, n = re.subn(
        r"(?m)^\s*lv_obj_set_style_bg_img_src\(obj, &img_2, LV_PART_SCROLLBAR \| LV_STATE_DISABLED\);\r?\n",
        "",
        sc,
    )
    (DST / "screens.c").write_text(sc2, encoding="utf-8", newline="\n")
    print(f"removed img_2 scrollbar refs: {n}")

    labels = re.findall(r'lv_label_set_text_static\(obj, "([^"]+)"\)', sc2)
    (Path(r"D:\ROMEOS 69\panel\scripts\v3_labels.txt")).write_text(
        "\n".join(labels), encoding="utf-8"
    )
    print("labels:", labels)

    (DST / "images.c").write_text(
        '#include "images.h"\n\n'
        "const ext_img_desc_t images[1] = {\n"
        '    { "4", &img_4 },\n'
        "};\n",
        encoding="utf-8",
        newline="\n",
    )
    ih = (SRC / "images.h").read_text(encoding="utf-8")
    ih = re.sub(
        r"extern const lv_img_dsc_t img_1;\n"
        r"extern const lv_img_dsc_t img_2;\n"
        r"extern const lv_img_dsc_t img_3;\n"
        r"extern const lv_img_dsc_t img_4;",
        "extern const lv_img_dsc_t img_4;",
        ih,
    )
    ih = re.sub(
        r"extern const ext_img_desc_t images\[\d+\];",
        "extern const ext_img_desc_t images[1];",
        ih,
    )
    (DST / "images.h").write_text(ih, encoding="utf-8", newline="\n")
    print("images.h/c ready")
    for p in sorted(DST.iterdir()):
        print(f"  {p.name:20} {p.stat().st_size/1e6:.2f} MB")


if __name__ == "__main__":
    main()
