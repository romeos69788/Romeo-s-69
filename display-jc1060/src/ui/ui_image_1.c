#include "lvgl.h"

extern const uint8_t img_1_map[] asm("_binary_data_img_1_bin_start");

const lv_img_dsc_t img_1 = {
    .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 1024,
    .header.h = 600,
    .data_size = 1024 * 600 * LV_IMG_PX_SIZE_ALPHA_BYTE,
    .data = img_1_map,
};
