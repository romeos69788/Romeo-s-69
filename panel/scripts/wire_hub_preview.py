"""Inspect Levita_4 export and wire slim preview into panel/."""
from __future__ import annotations

import re
import shutil
from pathlib import Path

SRC = Path(r"D:\ROMEOS 69\Othoni Levita\Othoni_Levita_1\src\ui")
DST = Path(r"D:\ROMEOS 69\panel\src\ui")


def find_bg_img(screens: str) -> str:
    refs = re.findall(r"&img_(\d+)", screens)
    # prefer MAIN default bg
    m = re.search(
        r"lv_obj_set_style_bg_img_src\(obj, &(img_\d+), LV_PART_MAIN \| LV_STATE_DEFAULT\)",
        screens,
    )
    if m:
        return m.group(1)
    if not refs:
        raise SystemExit("no img refs")
    return f"img_{refs[0]}"


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
    screens = (SRC / "screens.c").read_text(encoding="utf-8")
    labels = re.findall(r'lv_label_set_text_static\(obj, "([^"]+)"\)', screens)
    print("LABELS:", labels)
    print("IMG REFS:", sorted(set(re.findall(r"&img_\d+", screens))))
    bg = find_bg_img(screens)
    num = bg.split("_")[1]
    print("BG:", bg)

    img_src = SRC / f"ui_image_{num}.c"
    if not img_src.exists():
        raise SystemExit(f"missing {img_src}")

    # discover map name
    head = img_src.read_text(encoding="utf-8", errors="replace")[:4000]
    mm = re.search(r"uint8_t (\w+)\[\]", head)
    if not mm:
        raise SystemExit("no map name")
    map_name = mm.group(1)
    print("MAP:", map_name)

    # remove old slim images in panel
    for p in DST.glob("ui_image_*.c"):
        p.unlink()
        print("removed", p.name)

    slim_image(img_src, DST / f"ui_image_{num}.c", map_name, bg)

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

    # drop bogus scrollbar img refs (any img used only on SCROLLBAR)
    sc = (DST / "screens.c").read_text(encoding="utf-8")
    sc2, n = re.subn(
        r"(?m)^\s*lv_obj_set_style_bg_img_src\(obj, &img_\d+, LV_PART_SCROLLBAR \| LV_STATE_DISABLED\);\r?\n",
        "",
        sc,
    )
    (DST / "screens.c").write_text(sc2, encoding="utf-8", newline="\n")
    print(f"removed scrollbar img refs: {n}")

    # verify only bg remains
    left = sorted(set(re.findall(r"&img_\d+", sc2)))
    print("IMG after patch:", left)
    if left != [f"&{bg}"]:
        print("WARNING: unexpected img refs remain — check screens")

    (DST / "images.c").write_text(
        '#include "images.h"\n\n'
        "const ext_img_desc_t images[1] = {\n"
        f'    {{ "{num}", &{bg} }},\n'
        "};\n",
        encoding="utf-8",
        newline="\n",
    )
    ih = (SRC / "images.h").read_text(encoding="utf-8")
    # keep only the bg extern
    ih = re.sub(
        r"(extern const lv_img_dsc_t img_\d+;\n)+",
        f"extern const lv_img_dsc_t {bg};\n",
        ih,
        count=1,
    )
    ih = re.sub(
        r"extern const ext_img_desc_t images\[\d+\];",
        "extern const ext_img_desc_t images[1];",
        ih,
    )
    (DST / "images.h").write_text(ih, encoding="utf-8", newline="\n")

    # labels for docs
    (Path(r"D:\ROMEOS 69\panel\scripts\v4_labels.txt")).write_text(
        "\n".join(labels), encoding="utf-8"
    )
    print("images.h/c ready for", bg)
    print("screens.h main?", "objects.main" in (DST / "screens.h").read_text(encoding="utf-8"))


if __name__ == "__main__":
    main()
