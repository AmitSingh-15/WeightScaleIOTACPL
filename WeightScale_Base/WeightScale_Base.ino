#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>
#include <Arduino.h>
#include <HX711.h>
#include <Wire.h>
#include <SPI.h>


#include "gfx_conf.h"
#include "lvgl_port.h"

#include "home_screen.h"
#include "settings_screen.h"

#include "invoice_service.h"
#include "storage_service.h"
#include "wifi_service.h"
#include "ota_service.h"
#include "calibration_screen.h"

/* ================= GLOBAL STATE ================= */

static lv_obj_t *home_scr = NULL;
static lv_obj_t *settings_scr = NULL;
static lv_obj_t *cal_scr = NULL;


static uint16_t qty = 1;
#define HX711_DOUT 19
#define HX711_SCK  20

// ---------------- Touch ----------------
uint16_t tx, ty;

HX711 scale;
static float weight = 0.0f;
// ---------------- App State ----------------
bool measure_enable = false;
// Non-blocking timing
static uint32_t last_weight_read = 0;
const uint32_t WEIGHT_INTERVAL_MS = 200; // 5 Hz update
static float weight_offset = 0.0f;
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

static void open_calibration()
{
    lv_scr_load(cal_scr);
}

static void back_cb(void)
{
    lv_scr_load(home_scr);
}

/* ================= ARDUINO ================= */

static void update_weight()
{
   // static uint32_t last_read = 0;
    if (millis() - last_weight_read  < WEIGHT_INTERVAL_MS) return;  // Read every 200 ms (~5 Hz)
    last_weight_read  = millis();
    // delay(2000);
    if (scale.is_ready())
    {
        float raw = scale.get_units(5);// Average 5 readings
        weight = raw - weight_offset;
        if (weight < 0) weight = 0;  
        home_screen_set_weight(weight);  // Update your LVGL display with the new weight
        Serial.println("Weight: ");
        Serial.print(weight, 2); // 2 decimal places
        Serial.println(" kg");
         //delay(300);
    }
    else {
          Serial.println("[ERROR] HX711 NOT READY");
       
        }
      //  delay(500);

        
}

static void calibration_back_cb()
{
    lv_scr_load(settings_scr);
}

static void calibration_do_cb()
{
    // make current raw weight zero
    weight_offset += weight;
    storage_save_offset(weight_offset);
}


void setup()
{
    Serial.begin(115200);
    
 // ------------------- INIT HX711 -------------------
    scale.begin(HX711_DOUT, HX711_SCK);
    scale.set_scale(2280.0f); // <-- adjust this calibration value
    scale.tare();              // reset scale to 0

    // ------------------- INIT LVGL -------------------
    lvgl_port_init();

    storage_service_init();
    weight_offset = storage_load_offset();

    invoice_service_init();
    wifi_service_init();
    ota_service_init();
    // ------------------- CREATE SCREENS -------------------

    home_scr = lv_obj_create(NULL);
    settings_scr = lv_obj_create(NULL);

    home_screen_create(home_scr);
    home_screen_register_callback(ui_event);

    settings_screen_create(settings_scr);
    settings_screen_register_back_callback(back_cb);
    settings_screen_register_calibration_callback(open_calibration);

    cal_scr = lv_obj_create(NULL);
    calibration_screen_create(cal_scr);
    calibration_screen_register_back(calibration_back_cb);
    calibration_screen_register_calibrate(calibration_do_cb);


    home_screen_set_quantity(qty);
    home_screen_set_weight(weight);
    home_screen_set_invoice(invoice_service_current_id());

    lv_scr_load(home_scr);

   
}

void loop()
{
    lvgl_port_loop();
    
    update_weight();  // <-- Add this line to update the weight live

    wifi_service_loop();
    invoice_service_daily_reset_if_needed();
    measure_enable = true;
  

    if (lv_scr_act() == home_scr) {
        home_screen_set_sync_status(
            wifi_service_state() == WIFI_CONNECTED ? "Online" : "Offline"
        );
    }
    if (lv_scr_act() == cal_scr) {
    calibration_screen_set_weight(weight);
}

}
