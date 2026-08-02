#include "ui.h"
#include "screens.h"

static int16_t current_screen = -1;

int ui_current_screen_index(void)
{
    return (int)current_screen;
}

void loadScreen(enum ScreensEnum screenId)
{
    const int idx = (int)screenId - 1;
    if (idx < 0 || idx >= SCREEN_COUNT) {
        return;
    }
    lv_obj_t *target = objects.screens[idx];
    if (!target) {
        return;
    }

    lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_NONE;
    if (current_screen >= 0) {
        if (idx > current_screen) {
            anim = LV_SCR_LOAD_ANIM_MOVE_LEFT;   // next (swipe L←)
        } else if (idx < current_screen) {
            anim = LV_SCR_LOAD_ANIM_MOVE_RIGHT;  // back (swipe →R)
        }
    }

    current_screen = (int16_t)idx;
    lv_scr_load_anim(target, anim, 280, 0, false);
}

void ui_init(void)
{
    create_screens();
    loadScreen(SCREEN_ID_1);
}

void ui_tick(void)
{
    tick_screen(current_screen);
}
