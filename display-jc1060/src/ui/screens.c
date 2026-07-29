#include "screens.h"
#include "images.h"
#include "ui.h"

objects_t objects;

static lv_coord_t swipe_start_x;
static bool swipe_active;

static void on_swipe(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_indev_get_act();
    if (!indev) {
        return;
    }

    lv_point_t p;
    lv_indev_get_point(indev, &p);

    if (code == LV_EVENT_PRESSED) {
        swipe_start_x = p.x;
        swipe_active = true;
        return;
    }

    if (code != LV_EVENT_RELEASED || !swipe_active) {
        return;
    }
    swipe_active = false;

    const lv_coord_t dx = p.x - swipe_start_x;
    const lv_coord_t threshold = 80;  // ~8% of 1024

    if (dx <= -threshold) {
        // finger right → left : go to page 2
        loadScreen(SCREEN_ID_2);
    } else if (dx >= threshold) {
        // finger left → right : go to page 1
        loadScreen(SCREEN_ID_1);
    }
}

static void attach_swipe(lv_obj_t *scr)
{
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(scr, on_swipe, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(scr, on_swipe, LV_EVENT_RELEASED, NULL);
}

static lv_obj_t *make_image_screen(const lv_img_dsc_t *img)
{
    lv_obj_t *obj = lv_obj_create(NULL);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_img_src(obj, img, LV_PART_MAIN | LV_STATE_DEFAULT);
    attach_swipe(obj);
    return obj;
}

void create_screens(void)
{
    objects.screen_1 = make_image_screen(&img_1);
    objects.screen_2 = make_image_screen(&img_2);
}

void tick_screen(int screen_index)
{
    (void)screen_index;
}
