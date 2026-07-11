from pathlib import Path
from PIL import Image

SRC = Path(r"d:\Romeos Tsakas\romeos-display-v2\squareline-arc-import\ring_template.png")
OUT = Path(r"d:\Romeos Tsakas\romeos-display-v2\squareline-arc-import\images\ui_img_ring_template_png.c")


def main() -> None:
    im = Image.open(SRC).convert("RGBA")
    w, h = im.size
    px = im.load()
    data = []

    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            c = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            data.extend((c & 0xFF, (c >> 8) & 0xFF, a))

    with OUT.open("w", encoding="ascii") as f:
        f.write("// Generated from ring_template.png\n")
        f.write("#include \"lvgl.h\"\n\n")
        f.write("#ifndef LV_ATTRIBUTE_MEM_ALIGN\n#define LV_ATTRIBUTE_MEM_ALIGN\n#endif\n\n")
        f.write("const LV_ATTRIBUTE_MEM_ALIGN uint8_t ui_img_ring_template_png_data[] = {\n")
        for i in range(0, len(data), 24):
            row = ", ".join(f"0x{v:02X}" for v in data[i:i + 24])
            f.write("    " + row)
            if i + 24 < len(data):
                f.write(",")
            f.write("\n")
        f.write("};\n\n")
        f.write("const lv_img_dsc_t ui_img_ring_template_png = {\n")
        f.write("    .header.always_zero = 0,\n")
        f.write(f"    .header.w = {w},\n")
        f.write(f"    .header.h = {h},\n")
        f.write("    .data_size = sizeof(ui_img_ring_template_png_data),\n")
        f.write("    .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,\n")
        f.write("    .data = ui_img_ring_template_png_data\n")
        f.write("};\n")

    print(f"Generated: {OUT}")


if __name__ == "__main__":
    main()
