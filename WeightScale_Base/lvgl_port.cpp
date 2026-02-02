#include "lvgl_port.h"
#include "gfx_conf.h"

#include "esp_heap_caps.h"
#include "esp_timer.h"

/* ================== FORWARD DECLARATIONS ================== */
static void touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
static void lv_tick_cb(void *arg);

/* ================== LVGL BUFFERS ================== */
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = nullptr;
static lv_color_t *buf2 = nullptr;

/* ================== LVGL FLUSH ================== */
static void flush_cb(lv_disp_drv_t *disp,
                     const lv_area_t *area,
                     lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((uint16_t *)color_p, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/* ================== LVGL INIT ================== */
void lvgl_port_init(void)
{
    /* 1️⃣ Init LVGL core */
    lv_init();

    /* 2️⃣ Init display */
    tft.begin();

    /* 3️⃣ Allocate draw buffers (double buffering) */
    buf1 = (lv_color_t *)heap_caps_malloc(
        screenWidth * 60 * sizeof(lv_color_t),
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    buf2 = (lv_color_t *)heap_caps_malloc(
        screenWidth * 60 * sizeof(lv_color_t),
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (!buf1 || !buf2) {
        Serial.println("[LVGL] Buffer allocation failed");
        while (1) delay(100);
    }

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * 60);

    /* 4️⃣ Register display driver */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = screenWidth;
    disp_drv.ver_res  = screenHeight;
    disp_drv.flush_cb = flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* 5️⃣ Register touch input */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read;
    lv_indev_drv_register(&indev_drv);

    /* 6️⃣ Start LVGL tick using esp_timer (ISR-safe) */
    static esp_timer_handle_t lvgl_timer;
    const esp_timer_create_args_t lvgl_timer_args = {
        .callback = &lv_tick_cb,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lvgl_tick"
    };

    esp_timer_create(&lvgl_timer_args, &lvgl_timer);
    esp_timer_start_periodic(lvgl_timer, 1000); // 1 ms tick

    Serial.println("[LVGL] Initialized successfully");
}

/* ================== LVGL LOOP ================== */
void lvgl_port_loop(void)
{
    lv_timer_handler();
    delay(5);  // 🔥 REQUIRED for ESP32 stability
}

/* ================== LVGL TICK ================== */
static void lv_tick_cb(void *arg)
{
    lv_tick_inc(1);
}

/* ================== TOUCH READ ================== */
static void touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint16_t x, y;

    if (tft.getTouch(&x, &y)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}
