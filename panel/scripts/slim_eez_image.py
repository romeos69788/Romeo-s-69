"""Extract LV_COLOR_DEPTH==16 && SWAP==0 pixel block from EEZ multi-depth image .c"""
import re
import sys
from pathlib import Path


def slim_image(src_path: Path, dst_path: Path, map_name: str, img_name: str) -> None:
    text = src_path.read_text(encoding="utf-8", errors="replace")
    pattern = r"#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0\n(.*?)#endif"
    m = re.search(pattern, text, re.DOTALL)
    if not m:
        raise SystemExit(f"No 16-bit block in {src_path}")
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
    dst_path.parent.mkdir(parents=True, exist_ok=True)
    dst_path.write_text(slim, encoding="utf-8", newline="\n")
    print(f"{src_path.name}: {src_path.stat().st_size} -> {dst_path.stat().st_size}")


def main() -> None:
    if len(sys.argv) != 5:
        raise SystemExit(
            "usage: slim_eez_image.py SRC DST MAP_NAME IMG_NAME"
        )
    slim_image(Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3], sys.argv[4])


if __name__ == "__main__":
    main()
