SET(SOURCES components/ui_comp_label1.c
    components/ui_comp.c
    screens/ui_THERMOSTAT_3.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_chatgpt_az_png.c
    images/ui_img_476397486.c
    images/ui_img_1341533945.c
    images/ui_img_313400180.c
    images/ui_img_54340154.c
    images/ui_img_1311305648.c
    images/ui_img_1757177593.c
    images/ui_img_962555020.c
    images/ui_img_589449191.c
    images/ui_img_359196471.c
    fonts/ui_font_BigTemp12.c
    fonts/ui_font_BigTemp123.c
    fonts/ui_font_BigTemp1234.c
    fonts/ui_font_BigTemp22.c
    fonts/ui_font_BigTemp221.c
    fonts/ui_font_BigTemp221a.c
    fonts/ui_font_Font100.c
    fonts/ui_font_Font110.c
    fonts/ui_font_Font_40.c
    fonts/ui_font_Font_26.c
    fonts/ui_font_Font_10.c
    fonts/ui_font_Font_20.c
    fonts/ui_font_Font_15.c
    fonts/ui_font_Font_12.c
    fonts/ui_font_Font_55.c
    fonts/ui_font_Font_70.c)

add_library(ui ${SOURCES})
