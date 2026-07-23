"""Wire Levita_4 multi-screen export into panel/ (slim img_8 + img_10)."""
from __future__ import annotations

import re
import shutil
from pathlib import Path

SRC = Path(r"D:\ROMEOS 69\Othoni Levita\Othoni_Levita_1\src\ui")
DST = Path(r"D:\ROMEOS 69\panel\src\ui")

KEEP_IMGS = ("img_8", "img_10")


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


def map_name_for(img_src: Path) -> str:
    head = img_src.read_text(encoding="utf-8", errors="replace")[:4000]
    mm = re.search(r"uint8_t (\w+)\[\]", head)
    if not mm:
        raise SystemExit(f"no map name in {img_src}")
    return mm.group(1)


def main() -> None:
    screens = (SRC / "screens.c").read_text(encoding="utf-8")
    refs = sorted(set(re.findall(r"&img_\d+", screens)))
    print("IMG REFS:", refs)

    for p in DST.glob("ui_image_*.c"):
        p.unlink()
        print("removed", p.name)

    for name in KEEP_IMGS:
        num = name.split("_")[1]
        img_src = SRC / f"ui_image_{num}.c"
        if not img_src.exists():
            raise SystemExit(f"missing {img_src}")
        slim_image(img_src, DST / f"ui_image_{num}.c", map_name_for(img_src), name)

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

    sc = (DST / "screens.c").read_text(encoding="utf-8")
    sc2, n = re.subn(
        r"(?m)^\s*lv_obj_set_style_bg_img_src\(obj, &img_\d+, LV_PART_SCROLLBAR \| LV_STATE_DISABLED\);\r?\n",
        "",
        sc,
    )
    (DST / "screens.c").write_text(sc2, encoding="utf-8", newline="\n")
    print(f"removed scrollbar img refs: {n}")
    left = sorted(set(re.findall(r"&img_\d+", sc2)))
    print("IMG after patch:", left)
    expected = {f"&{x}" for x in KEEP_IMGS}
    if set(left) != expected:
        print("WARNING: unexpected img refs", left)

    images_c = '#include "images.h"\n\nconst ext_img_desc_t images[2] = {\n'
    for name in KEEP_IMGS:
        num = name.split("_")[1]
        images_c += f'    {{ "{num}", &{name} }},\n'
    images_c += "};\n"
    (DST / "images.c").write_text(images_c, encoding="utf-8", newline="\n")

    ih = (SRC / "images.h").read_text(encoding="utf-8")
    externs = "".join(f"extern const lv_img_dsc_t {n};\n" for n in KEEP_IMGS)
    ih = re.sub(r"(extern const lv_img_dsc_t img_\d+;\n)+", externs, ih, count=1)
    ih = re.sub(
        r"extern const ext_img_desc_t images\[\d+\];",
        "extern const ext_img_desc_t images[2];",
        ih,
    )
    (DST / "images.h").write_text(ih, encoding="utf-8", newline="\n")
    print("images.h/c ready for", KEEP_IMGS)

    # Extend ui.c/h with animated load (keeps EEZ currentScreen in sync)
    ui_c = (DST / "ui.c").read_text(encoding="utf-8")
    if "loadScreenAnim" not in ui_c:
        ui_c = ui_c.replace(
            "void loadScreen(enum ScreensEnum screenId) {\n"
            "    currentScreen = screenId - 1;\n"
            "    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);\n"
            "    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);\n"
            "}\n",
            "void loadScreenAnim(enum ScreensEnum screenId, lv_scr_load_anim_t anim, uint32_t time) {\n"
            "    currentScreen = screenId - 1;\n"
            "    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);\n"
            "    lv_scr_load_anim(screen, anim, time, 0, false);\n"
            "}\n"
            "\n"
            "void loadScreen(enum ScreensEnum screenId) {\n"
            "    loadScreenAnim(screenId, LV_SCR_LOAD_ANIM_FADE_IN, 200);\n"
            "}\n",
        )
        (DST / "ui.c").write_text(ui_c, encoding="utf-8", newline="\n")
        print("patched ui.c loadScreenAnim")

    ui_h = (DST / "ui.h").read_text(encoding="utf-8")
    if "loadScreenAnim" not in ui_h:
        ui_h = ui_h.replace(
            "void loadScreen(enum ScreensEnum screenId);",
            "void loadScreen(enum ScreensEnum screenId);\n"
            "void loadScreenAnim(enum ScreensEnum screenId, lv_scr_load_anim_t anim, uint32_t time);",
        )
        (DST / "ui.h").write_text(ui_h, encoding="utf-8", newline="\n")
        print("patched ui.h loadScreenAnim")


if __name__ == "__main__":
    main()
