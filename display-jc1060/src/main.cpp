/*
 * JC1060P470C — show EEZ thermostat mockup (static full-screen image)
 */

#include <Arduino.h>
#include <lvgl.h>
#include <esp_heap_caps.h>
#include "driver/i2c_master.h"

#include "pins_config.h"
#include "jd9165_lcd.h"
#include "gt911_touch.h"
#include "ui.h"

static bsp_lcd_handles_t lcd_panels;
static jd9165_lcd lcd(LCD_RST);
static gt911_touch touch(TP_I2C_SDA, TP_I2C_SCL, TP_RST, TP_INT);

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf0 = nullptr;
static lv_color_t *buf1 = nullptr;

static bool on_dpi_flush_done(esp_lcd_panel_handle_t, esp_lcd_dpi_panel_event_data_t *, void *user_ctx)
{
    lv_disp_flush_ready(static_cast<lv_disp_drv_t *>(user_ctx));
    return false;
}

static void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    lcd.lcd_draw_bitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, &color_p->full);
}

static void touch_read(lv_indev_drv_t *, lv_indev_data_t *data)
{
    uint16_t x = 0, y = 0;
    if (touch.getTouch(&x, &y)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(400);
    Serial.println("*** JC1060P4 EEZ mockup · 2 pages · swipe ***");

    i2c_master_bus_handle_t i2c_handle = nullptr;
    i2c_master_bus_config_t i2c_bus_conf = {};
    i2c_bus_conf.i2c_port = I2C_NUM_1;
    i2c_bus_conf.sda_io_num = (gpio_num_t)TP_I2C_SDA;
    i2c_bus_conf.scl_io_num = (gpio_num_t)TP_I2C_SCL;
    i2c_bus_conf.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_bus_conf.glitch_ignore_cnt = 7;
    i2c_bus_conf.flags.enable_internal_pullup = 1;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_conf, &i2c_handle));

    lcd.begin();
    touch.begin();
    lcd.get_handle(&lcd_panels);

    lv_init();

    const size_t px = (size_t)LCD_H_RES * (size_t)LCD_V_RES;
    buf0 = (lv_color_t *)heap_caps_malloc(px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    buf1 = (lv_color_t *)heap_caps_malloc(px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    if (!buf0 || !buf1) {
        Serial.println("ERROR: PSRAM draw buffers");
        return;
    }
    lv_disp_draw_buf_init(&draw_buf, buf0, buf1, px);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = true;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read;
    lv_indev_drv_register(&indev_drv);

    esp_lcd_dpi_panel_event_callbacks_t cbs = {};
    cbs.on_color_trans_done = on_dpi_flush_done;
    esp_lcd_dpi_panel_register_event_callbacks(lcd_panels.panel, &cbs, &disp_drv);

    ui_init();
    Serial.println("[jc1060] EEZ mockup loaded");
}

void loop()
{
    lv_timer_handler();
    ui_tick();
    delay(5);
}
