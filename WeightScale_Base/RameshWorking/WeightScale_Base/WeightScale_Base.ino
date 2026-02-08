#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "gfx_conf.h"
#include "lvgl_port.h"

#include "home_screen.h"
#include "settings_screen.h"
#include "calibration_screen.h"

#include "invoice_service.h"
#include "storage_service.h"
#include "wifi_service.h"
#include "ota_service.h"
#include "scale_service_v2.h"
#include "ui_events.h"

/* ================= GLOBAL STATE ================= */

static lv_obj_t *home_scr = NULL;
static lv_obj_t *settings_scr = NULL;
static lv_obj_t *cal_scr = NULL;

static uint16_t qty = 1;
static float weight = 0.0f;

/* ================= UI EVENTS ================= */

static void ui_event(int evt)
{
    if (evt == UI_EVT_SETTINGS)
        lv_scr_load(settings_scr);

    if (evt == UI_EVT_QTY_INC) qty++;
    if (evt == UI_EVT_QTY_DEC && qty > 1) qty--;

    if (evt == UI_EVT_QTY_X2)  qty += 2;
    if (evt == UI_EVT_QTY_X5)  qty += 5;
    if (evt == UI_EVT_QTY_X10) qty += 10;

    if (evt == UI_EVT_QTY_INC ||
        evt == UI_EVT_QTY_DEC ||
        evt == UI_EVT_QTY_X2  ||
        evt == UI_EVT_QTY_X5  ||
        evt == UI_EVT_QTY_X10)
    {
        home_screen_set_quantity(qty);
        home_screen_set_total(weight * qty);
    }

    if (evt == UI_EVT_SAVE)
    {
        invoice_record_t rec;

        if (invoice_service_save(weight, qty, ENTRY_MANUAL, &rec))
        {
            home_screen_set_invoice(invoice_service_current_id());

            qty = 1;
            home_screen_set_quantity(qty);
            home_screen_set_total(weight * qty);

            home_screen_update_history();
        }
    }

    if (evt == UI_EVT_RESET)
    {
        storage_clear_all_records();
        invoice_service_init();
        home_screen_update_history();
        home_screen_set_invoice(invoice_service_current_id());
    }
}

/* ================= SCREEN NAVIGATION ================= */

static void open_calibration()
{
    Serial.println("[CAL] Open calibration screen");
    lv_scr_load(cal_scr);
}

static void back_cb(void)
{
    lv_scr_load(home_scr);
}

static void calibration_back_cb()
{
    lv_scr_load(settings_scr);
}

/* ================= CALIBRATION CALLBACKS ================= */

static void calib_offset()
{
    Serial.println("[CAL] OFFSET (tare)");
    scale_service_tare();
}

static void calib_scale()
{
    // Industrial flow:
    // scale factor logic should live inside scale_service later
    Serial.println("[CAL] SCALE requested (handled in service layer later)");
}

static void calib_both()
{
    calib_offset();
    calib_scale();
}

/* ================= WEIGHT UPDATE ================= */

static void update_weight()
{
    weight = scale_service_get_weight();

    home_screen_set_weight(weight);
    home_screen_set_total(weight * qty);
    calibration_screen_set_weight(weight);
}

/* ================= SETUP ================= */

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("=== INDUSTRIAL SCALE START ===");

    lvgl_port_init();

    storage_service_init();
    invoice_service_init();
    wifi_service_init();
    ota_service_init();

    /* ===== START SCALE RTOS TASK ===== */
    scale_service_init();

    /* ===== CREATE SCREENS ===== */

    home_scr = lv_obj_create(NULL);
    settings_scr = lv_obj_create(NULL);
    cal_scr = lv_obj_create(NULL);

    home_screen_create(home_scr);
    home_screen_register_callback(ui_event);

    settings_screen_create(settings_scr);
    settings_screen_register_back_callback(back_cb);
    settings_screen_register_calibration_callback(open_calibration);

    calibration_screen_create(cal_scr);
    calibration_screen_register_back(calibration_back_cb);
    calibration_screen_register_offset(calib_offset);
    calibration_screen_register_scale(calib_scale);
    calibration_screen_register_both(calib_both);

    /* ===== INITIAL UI STATE ===== */

    home_screen_set_quantity(qty);
    home_screen_set_weight(0);
    home_screen_set_total(0);
    home_screen_set_invoice(invoice_service_current_id());
    home_screen_update_history();

    lv_scr_load(home_scr);
}

/* ================= LOOP ================= */

void loop()
{
    lvgl_port_loop();

    update_weight();

    wifi_service_loop();
    invoice_service_daily_reset_if_needed();
    storage_check_new_day_and_reset();

    if (lv_scr_act() == home_scr)
    {
        home_screen_set_sync_status(
            wifi_service_state() == WIFI_CONNECTED ? "Online" : "Offline"
        );
    }
}
