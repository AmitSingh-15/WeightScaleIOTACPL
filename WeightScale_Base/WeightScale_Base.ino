#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "gfx_conf.h"
#include "lvgl_port.h"

#include "home_screen.h"
#include "settings_screen.h"
#include "ui_events.h"
#include "invoice_service.h"
#include "storage_service.h"
#include "wifi_service.h"
//#include "ota_service.h"
#include "calibration_screen.h"
#include "scale_service.h"




/* ================= GLOBAL STATE ================= */

static lv_obj_t *home_scr = NULL;
static lv_obj_t *settings_scr = NULL;
static lv_obj_t *cal_scr = NULL;

static uint16_t qty = 1;

/* ================= CALLBACKS ================= */

static void ui_event(int evt)
{
    switch (evt)
    {
        case UI_EVT_SETTINGS:
            lv_scr_load(settings_scr);
            break;

        case UI_EVT_QTY_INC:
            qty += 1;
            break;

        case UI_EVT_QTY_DEC:
            if (qty > 1) qty -= 1;
            break;

        case UI_EVT_QTY_ADD_2:
            qty += 2;
            break;

        case UI_EVT_QTY_ADD_5:
            qty += 5;
            break;

        case UI_EVT_QTY_ADD_10:
            qty += 10;
            break;

        case UI_EVT_SAVE:
        {
            invoice_record_t rec;
            if (invoice_service_save(scale_service_get_weight(), qty, ENTRY_MANUAL, &rec)) {
                qty = 1;
                home_screen_set_invoice(invoice_service_current_id());
                home_screen_update_history();
            }
            break;
        }

        case UI_EVT_RESET:
            storage_clear_all_records();
            invoice_service_init();
            home_screen_update_history();
            break;
    }

    home_screen_set_quantity(qty);
    home_screen_set_total(scale_service_get_weight() * qty);
}


static void open_calibration()
{
  Serial.println("[CAL] Open calibration screen");
    lv_scr_load(cal_scr);    
}

static void back_cb(void)
{
    lv_scr_load(home_scr);
}


/* ================= ARDUINO ================= */

static void calibration_back_cb()
{
    lv_scr_load(settings_scr);
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("=== SYSTEM START ===");

    lvgl_port_init();

    storage_service_init();
    invoice_service_init();
    wifi_service_init();
    //ota_service_init();

    scale_service_init();   // 🔥 REQUIRED

    home_scr = lv_obj_create(NULL);
    settings_scr = lv_obj_create(NULL);
    cal_scr = lv_obj_create(NULL);

    home_screen_create(home_scr);
    home_screen_register_callback(ui_event);

    settings_screen_create(settings_scr);
    settings_screen_register_back_callback(back_cb);
    settings_screen_register_calibration_callback(open_calibration);

    calibration_screen_create(cal_scr);
    calibration_register_back(calibration_back_cb);

    home_screen_set_quantity(qty);
    home_screen_set_invoice(invoice_service_current_id());
    home_screen_update_history();

    lv_scr_load(home_scr);
}

void loop()
{
    lvgl_port_loop();

    scale_service_loop();

    float w = scale_service_get_weight();
    home_screen_set_weight(w);
    home_screen_set_total(w * qty);

    if (lv_scr_act() == cal_scr) {
        calibration_screen_set_live_weight(w);
    }

    wifi_service_loop();
    invoice_service_daily_reset_if_needed();
    storage_check_new_day_and_reset(); 
}

