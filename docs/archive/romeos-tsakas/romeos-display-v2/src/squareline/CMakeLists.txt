SET(SOURCES components/ui_comp_label1.c
    components/ui_comp.c
    screens/ui_THERMOSTAT_1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_thermostat_a_png.c
    images/ui_img_1757177593.c
    fonts/ui_font_BigTemp12.c
    fonts/ui_font_BigTemp123.c
    fonts/ui_font_BigTemp1234.c
    fonts/ui_font_BigTemp22.c
    fonts/ui_font_BigTemp221.c
    fonts/ui_font_BigTemp221a.c)

add_library(ui ${SOURCES})
