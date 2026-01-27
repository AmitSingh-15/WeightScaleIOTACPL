#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>
#include <Arduino.h>

#include "gfx_conf.h"
#include "lvgl_port.h"

#include "home_screen.h"
#include "settings_screen.h"

#include "invoice_service.h"
#include "storage_service.h"
#include "wifi_service.h"
#include "ota_service.h"

/* ================= GLOBAL STATE ================= */

static lv_obj_t *home_scr = NULL;
static lv_obj_t *settings_scr = NULL;

static uint16_t qty = 1;
static float weight = 0.0f;

/* ================= CALLBACKS ================= */

static void ui_event(int evt)
{
    if (evt == UI_EVT_SETTINGS)
        lv_scr_load(settings_scr);

    if (evt == UI_EVT_QTY_INC)
        home_screen_set_quantity(++qty);

    if (evt == UI_EVT_QTY_DEC && qty > 1)
        home_screen_set_quantity(--qty);

    if (evt == UI_EVT_SAVE) {
        invoice_record_t rec;
        if (invoice_service_save(weight, qty, ENTRY_MANUAL, &rec)) {
            home_screen_set_invoice(invoice_service_current_id());
            qty = 1;
            home_screen_set_quantity(qty);
        }
    }
}

static void back_cb(void)
{
    lv_scr_load(home_scr);
}

/* ================= ARDUINO ================= */

void setup()
{
    Serial.begin(115200);

    lvgl_port_init();

    storage_service_init();
    invoice_service_init();
    wifi_service_init();
    ota_service_init();

    home_scr = lv_obj_create(NULL);
    settings_scr = lv_obj_create(NULL);

    home_screen_create(home_scr);
    home_screen_register_callback(ui_event);

    settings_screen_create(settings_scr);
    settings_screen_register_back_callback(back_cb);

    home_screen_set_quantity(qty);
    home_screen_set_weight(weight);
    home_screen_set_invoice(invoice_service_current_id());

    lv_scr_load(home_scr);
}

void loop()
{
    lvgl_port_loop();
    wifi_service_loop();
    invoice_service_daily_reset_if_needed();

    if (lv_scr_act() == home_scr) {
        home_screen_set_sync_status(
            wifi_service_state() == WIFI_CONNECTED ? "Online" : "Offline"
        );
    }
}
