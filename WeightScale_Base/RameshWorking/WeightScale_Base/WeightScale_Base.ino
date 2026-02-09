#define ARDUINO_USB_CDC_ON_BOOT 1
#define LV_CONF_INCLUDE_SIMPLE

#include <Arduino.h>
#include <lvgl.h>

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
#include "ui_styles.h"
/* =========================================================
   GLOBAL STATE
=========================================================*/

static lv_obj_t *home_scr = NULL;
static lv_obj_t *settings_scr = NULL;
static lv_obj_t *cal_scr = NULL;

static uint16_t qty = 1;
static float weight = 0.0f;
static bool ui_frozen = false;
static bool reset_pending = false;

/* Industrial scale profiles */

static const scale_profile_t PROFILE_1KG =
{
    "1KG",
    1.0f,
    9000.0f,
    0.35f,
    0.002f,
    500
};

static const scale_profile_t PROFILE_100KG =
{
    "100KG",
    100.0f,
    2280.0f,
    0.25f,
    0.02f,
    1200
};

static const scale_profile_t PROFILE_500KG =
{
    "500KG",
    500.0f,
    450.0f,
    0.15f,
    0.08f,
    1800
};

/* =========================================================
   RESET CONFIRMATION POPUP (INDUSTRIAL)
=========================================================*/

static lv_obj_t *reset_msgbox = NULL;



static void reset_confirm_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const char *txt = lv_msgbox_get_active_btn_text(obj);

    // Close msgbox FIRST and safely
    lv_msgbox_close(obj);
    reset_msgbox = NULL;

    if (txt && strcmp(txt, "YES") == 0)
    {
        Serial.println("[RESET] Confirmed");
        ui_frozen = true;
        reset_pending = true;
    }
}



static void show_reset_confirm_popup()
{
    if(reset_msgbox) return;

    static const char *btns[] = {"YES","NO",""};

    reset_msgbox = lv_msgbox_create(lv_scr_act(),
        "CONFIRM RESET",
        "Do you want to clear today's complete history?",
        btns,
        true);

    lv_obj_center(reset_msgbox);
    lv_obj_add_event_cb(reset_msgbox, reset_confirm_cb,
                        LV_EVENT_VALUE_CHANGED, NULL);
}

/* =========================================================
   UI EVENTS
=========================================================*/

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

    /* 🔥 INDUSTRIAL RESET FLOW */
    if (evt == UI_EVT_RESET)
    {
        show_reset_confirm_popup();
    }
}

/* =========================================================
   SCREEN NAVIGATION
=========================================================*/

static void open_calibration()
{
    Serial.println("[CAL] Open calibration screen");
    lv_scr_load(cal_scr);
}

static void back_cb(void)
{
    lv_scr_load(home_scr);
}

static void calibration_wizard_event(int evt)
{
    switch (evt)
    {
        case CAL_EVT_BACK:
            lv_scr_load(settings_scr);
            break;

        case CAL_EVT_CAPTURE_ZERO:
            scale_service_tare();
            Serial.println("[CAL] Tare");
            break;

        case CAL_EVT_CAPTURE_LOAD:
            Serial.println("[CAL] Capture load (TODO)");
            break;

        case CAL_EVT_SAVE:
            Serial.println("[CAL] Save calibration (TODO)");
            break;
    }
}


/* =========================================================
   CALIBRATION CALLBACKS
=========================================================*/

static void calib_offset()
{
    Serial.println("[CAL] OFFSET (tare)");
    scale_service_tare();
}

static void calib_scale()
{
    Serial.println("[CAL] SCALE requested");
}

static void calib_both()
{
    calib_offset();
    calib_scale();
}

/* =========================================================
   WEIGHT UPDATE LOOP
=========================================================*/

static void update_weight()
{
    if (ui_frozen) return;   // 🔥 ABSOLUTE RULE

    weight = scale_service_get_weight();

    if (lv_scr_act() == home_scr)
    {
        home_screen_set_weight(weight);
        home_screen_set_total(weight * qty);
    }

    if (lv_scr_act() == cal_scr)
    {
        calibration_screen_set_live(
            weight,
            scale_service_get_raw()
        );
    }
}



/* =========================================================
   SETUP
=========================================================*/

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("=== INDUSTRIAL SCALE START ===");

    lvgl_port_init();

    ui_styles_init();
    storage_service_init();
    invoice_service_init();
    wifi_service_init();
    ota_service_init();

    /* START RTOS SCALE SERVICE */
    scale_service_init();
    scale_service_set_profile(&PROFILE_1KG);

    /* CREATE SCREENS */

    home_scr     = lv_obj_create(NULL);
    settings_scr = lv_obj_create(NULL);
    cal_scr      = lv_obj_create(NULL);

    home_screen_create(home_scr);
    home_screen_register_callback(ui_event);

    settings_screen_create(settings_scr);
    settings_screen_register_back_callback(back_cb);
    settings_screen_register_calibration_callback(open_calibration);

    calibration_screen_create(cal_scr);
    calibration_screen_register_callback(calibration_wizard_event);


    /* INITIAL UI */

    home_screen_set_quantity(qty);
    home_screen_set_weight(0);
    home_screen_set_total(0);
    home_screen_set_invoice(invoice_service_current_id());
    home_screen_update_history();

    lv_scr_load(home_scr);
}

/* =========================================================
   LOOP
=========================================================*/

void loop()
{
    if (reset_pending)
{
    reset_pending = false;

    storage_clear_all_records();

    uint32_t id = 1;
    storage_save_invoice(id);
    invoice_service_init();

    home_screen_update_history();
    home_screen_set_invoice(invoice_service_current_id());

    ui_frozen = false;

    Serial.println("[RESET] Completed safely");
}

    lvgl_port_loop();

    update_weight();

    wifi_service_loop();
    invoice_service_daily_reset_if_needed();
    storage_check_new_day_and_reset();

    if (lv_scr_act() == home_scr)
    {
        home_screen_set_sync_status(
            wifi_service_state() == WIFI_CONNECTED ?
            "Online" : "Offline"
        );
    }
}
