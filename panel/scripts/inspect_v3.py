import re
from pathlib import Path

ui = Path(r"D:\ROMEOS 69\Othoni Levita\Othoni_Levita_1\src\ui")
screens = (ui / "screens.c").read_text(encoding="utf-8")
labels = re.findall(r'lv_label_set_text_static\(obj, "([^"]+)"\)', screens)
print("LABELS:")
for lab in labels:
    print(" ", lab)
print("IMG:", sorted(set(re.findall(r"&img_\d+", screens))))
print("--- screens.h ---")
print((ui / "screens.h").read_text(encoding="utf-8")[:900])
img4 = (ui / "ui_image_4.c").read_text(encoding="utf-8", errors="replace")
for pat in ["uint8_t ", "const lv_img_dsc_t", "header.w", "header.h"]:
    for line in img4.splitlines():
        if pat in line and "map" in line.lower() or (pat.startswith("const") and "img_4" in line) or ("header." in line and "w" in line or "h" in line):
            if pat in line:
                print(line.strip()[:120])
                break
# map name
m = re.search(r"uint8_t (\w+)\[\]", img4)
print("MAP:", m.group(1) if m else None)
