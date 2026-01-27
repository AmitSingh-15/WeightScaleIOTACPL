#include "lvgl_port.h"
#include "gfx_conf.h"
#include <Ticker.h>

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[screenWidth * 40];
static lv_color_t buf2[screenWidth * 40];

static Ticker lv_tick;

/* LVGL tick every 1 ms */
static void tick_cb()
{
    lv_tick_inc(1);
}

/* Flush callback */
static void flush_cb(lv_disp_drv_t *disp,
                     const lv_area_t *area,
                     lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void lvgl_port_init(void)
{
    lv_init();
    tft.begin();

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * 40);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = screenWidth;
    disp_drv.ver_res  = screenHeight;
    disp_drv.flush_cb = flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* Start LVGL tick */
    lv_tick.attach_ms(1, tick_cb);
}

void lvgl_port_loop(void)
{
    lv_timer_handler();
    delay(5);
}
