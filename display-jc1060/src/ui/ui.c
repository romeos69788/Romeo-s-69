#include "ui.h"
#include "screens.h"

static int16_t current_screen = -1;

void loadScreen(enum ScreensEnum screenId)
{
    const int idx = (int)screenId - 1;
    lv_obj_t *target = (idx == 0) ? objects.screen_1 : objects.screen_2;
    if (!target) {
        return;
    }

    lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_NONE;
    if (current_screen >= 0) {
        if (idx > current_screen) {
            anim = LV_SCR_LOAD_ANIM_MOVE_LEFT;   // swipe LTR content → next
        } else if (idx < current_screen) {
            anim = LV_SCR_LOAD_ANIM_MOVE_RIGHT;  // back
        }
    }

    current_screen = idx;
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
