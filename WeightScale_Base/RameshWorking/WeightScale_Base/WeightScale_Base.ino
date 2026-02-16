
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
#include "device_name_screen.h"
#include "history_screen.h"


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
bool wifi_critical_section = false;
static lv_obj_t *history_scr = NULL;


/* Industrial scale profiles */

static const scale_profile_t PROFILE_1KG =
{
    "RAW",
    1.0f,
    1.0,
    0.35f,
    0.002f,
    500
};

static const scale_profile_t PROFILE_100KG =
{
    "1KG",
   // 100.0f,
    //2280.0f,
    //0.25f,
    //0.02f,
    //1200
    1.0f,
    58281.3,
    0.35f,
    0.002f,
    500
};

static const scale_profile_t PROFILE_500KG =
{
    "500KG",
   // 500.0f,
    //49000.0f,
    //0.15f,
    //0.08f,
    //1800
     1.0f,
    //61287.5,
      58281.3,
    0.35f,
    0.002f,
    500
};

static lv_obj_t *device_scr = NULL;
static char device_name[64] = {0};


/* =========================================================
   RESET CONFIRMATION POPUP (INDUSTRIAL)
=========================================================*/

static lv_obj_t *reset_msgbox = NULL;

/* =========================================================
   UI EVENTS
=========================================================*/

static void ui_event(int evt)
{
    if (evt == UI_EVT_SETTINGS)
        lv_scr_load(settings_scr);

    if (evt == UI_EVT_HISTORY)
    {
        history_screen_refresh();
        lv_scr_load(history_scr);
    }

    if (evt == UI_EVT_QTY_INC)
    {
        qty++;
        home_screen_set_quantity(qty);
    }

    if (evt == UI_EVT_QTY_DEC && qty > 1)
    {
        qty--;
        home_screen_set_quantity(qty);
    }

    if (evt == UI_EVT_SAVE)
    {
        if(invoice_session_add(weight, qty))
        {
            home_screen_refresh_invoice_details();
            qty = 1;
            home_screen_set_quantity(qty);
        }
    }

    if (evt == UI_EVT_RESET)
    {
        invoice_session_commit();
        invoice_service_next();
        home_screen_set_invoice(invoice_service_current_id());
        home_screen_refresh_invoice_details();
    }

    if(evt == UI_EVT_RESET_ALL)
    {
        invoice_session_clear();
        storage_clear_all_records();

        /* RESET INVOICE TO 1 */
        storage_save_invoice(1);
        invoice_service_init();

        home_screen_set_invoice(invoice_service_current_id());
        home_screen_refresh_invoice_details();
    }

    if(evt == 1002)
    {
        qty *= 2;
        home_screen_set_quantity(qty);
    }

    if(evt == 1005)
    {
        qty *= 5;
        home_screen_set_quantity(qty);
    }

    if(evt == 1010)
    {
        qty *= 10;
        home_screen_set_quantity(qty);
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

static void device_name_saved(int evt, const char *name)
{
    if(evt != DEVNAME_EVT_SAVE) return;

    Serial.printf("[DEV] Saved: %s\n", name);

    storage_save_device_name(name);

    /* ✅ CRITICAL — update UI NOW */
    home_screen_set_device(name);

    lv_scr_load(home_scr);
}



static void calibration_wizard_event(int evt)
{
    switch (evt)
    {
        case CAL_EVT_BACK:
            lv_scr_load(settings_scr);
            break;
        case CAL_EVT_PROFILE_1KG:
            scale_service_set_profile(&PROFILE_1KG);
            break;

        case CAL_EVT_PROFILE_100KG:
            scale_service_set_profile(&PROFILE_100KG);
            break;

        case CAL_EVT_PROFILE_500KG:
            scale_service_set_profile(&PROFILE_500KG);
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
    }

    if (lv_scr_act() == cal_scr)
    {
        calibration_screen_set_live(
            weight,
            scale_service_get_raw()
        );
    }
    static float last_weight = 0;

    static float stable_weight = 0;
    static uint32_t stable_start = 0;

    last_weight = weight;

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
    invoice_session_init();

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
    home_screen_set_invoice(invoice_service_current_id());
    device_scr = lv_obj_create(NULL);
    device_name_screen_create(device_scr);
    device_name_screen_register_callback(device_name_saved);


    if(storage_load_device_name(device_name,sizeof(device_name)))
    {
        Serial.println("[DEVICE] Existing name found");
        home_screen_set_device(device_name);
        lv_scr_load(home_scr);
    }
    else
    {
        Serial.println("[DEVICE] First boot — asking name");
        lv_scr_load(device_scr);
    }
    history_scr = lv_obj_create(NULL);
    history_screen_create(history_scr);
    history_screen_register_back(back_cb);


}

/* =========================================================
   LOOP
=========================================================*/

void loop()
{
    if(!wifi_critical_section)
    {
        lvgl_port_loop();
    }

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
