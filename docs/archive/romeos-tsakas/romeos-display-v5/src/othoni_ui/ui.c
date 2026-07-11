#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"
#include "romeos_eez_runtime.h"

#include <string.h>

static int16_t currentScreen = -1;

static lv_obj_t *getLvglObjectFromIndex(int32_t index)
{
    if (index == -1) {
        return 0;
    }
    return ((lv_obj_t **)&objects)[index];
}

void loadScreen(enum ScreensEnum screenId)
{
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    if (screen != NULL) {
        /* Άμεση αλλαγή — χωρίς lv_scr_load_anim (αποφεύγει scr_to_load / anim state σε RGB mode 3). */
        lv_scr_load(screen);
        romeos_eez_runtime_on_screen_loaded();
    }
}

void loadScreenAnim(enum ScreensEnum screenId, lv_scr_load_anim_t anim, uint32_t time_ms)
{
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    if (screen == NULL) {
        return;
    }
    if (anim == LV_SCR_LOAD_ANIM_NONE || time_ms == 0) {
        lv_scr_load(screen);
        return;
    }
    lv_scr_load_anim(screen, anim, time_ms, 0, false);
}

void ui_init()
{
    create_screens();
    loadScreen(SCREEN_ID_MAIN);
}

void ui_tick()
{
    tick_screen(currentScreen);
}
