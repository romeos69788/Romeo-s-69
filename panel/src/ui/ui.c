#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"

#include <string.h>

static int16_t currentScreen = -1;

static lv_obj_t *getLvglObjectFromIndex(int32_t index) {
    if (index == -1) {
        return 0;
    }
    return ((lv_obj_t **)&objects)[index];
}

void loadScreenAnim(enum ScreensEnum screenId, lv_scr_load_anim_t anim, uint32_t time) {
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    lv_scr_load_anim(screen, anim, time, 0, false);
}

void loadScreen(enum ScreensEnum screenId) {
    loadScreenAnim(screenId, LV_SCR_LOAD_ANIM_FADE_IN, 200);
}

void ui_init() {
    create_screens();
    loadScreen(SCREEN_ID_MAIN);

}

void ui_tick() {
    tick_screen(currentScreen);
}