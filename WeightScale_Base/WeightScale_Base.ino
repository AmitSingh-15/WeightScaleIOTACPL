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

HX711 scale;
static float weight = 0.0f;

// 🔴 ADDED: persistent scale factor
static float scale_factor = 2280.0f;

// ---------------- App State ----------------
bool measure_enable = false; // ✅ ADDED
static uint32_t last_weight_read = 0;
const uint32_t WEIGHT_INTERVAL_MS = 200;
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

    // ✅ TOGGLE MEASUREMENT
    if (evt == UI_EVT_MEASURE)
    {
        measure_enable = !measure_enable;
        home_screen_set_measure_state(measure_enable); // ✅ update button label
        Serial.println(measure_enable ? "MEASURE ON" : "MEASURE OFF");

                if (measure_enable)
        {
          scale.begin(HX711_DOUT, HX711_SCK); 
            // simple delay to let HX711 become ready
            delay(2000); 
            if (!scale.is_ready())
            {
                Serial.println("[HX711] NOT READY, waiting a bit...");
                delay(200);
            }
            Serial.println("[HX711] Ready (or almost ready)");
        }

    }

    if (evt == UI_EVT_SAVE) {
        invoice_record_t rec;
        if (invoice_service_save(weight, qty, ENTRY_MANUAL, &rec)) {
            home_screen_set_invoice(invoice_service_current_id());
            qty = 1;
            home_screen_set_quantity(qty);
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

static void open_calibration()
{
  Serial.println("[CAL] Open calibration screen");
    lv_scr_load(cal_scr);
    // simple delay so HX711 stabilizes
    delay(200);
    
}

static void back_cb(void)
{
    lv_scr_load(home_scr);
}

/* ================= CALIBRATION ================= */

static void calib_offset()
{
    Serial.println("OFFSET calibration = tare()");
    delay(100);
    scale.tare();
    weight_offset = 0;
    Serial.println("[CAL] OFFSET DONE");
}

static void calib_scale()
{
    const float known = 100.00;
    
    Serial.println("[CAL] SCALE → place known weight 100kg");
    delay(100); // allow HX711 to stabilize
    
    Serial.print("Weight: ");
        Serial.println(weight, 2);

    float raw = weight;
    
    scale_factor = raw / known;

    scale.set_scale(scale_factor);
    storage_save_offset(scale_factor);

    Serial.print("New Scale factor: ");
    Serial.println(scale_factor);
     delay(2000);
}

static void calib_both()
{
   Serial.println("[CAL] BOTH calibration started");
    calib_offset();
    delay(200);
    calib_scale();
    Serial.println("[CAL] BOTH DONE");
}

/* ================= ARDUINO ================= */

static void update_weight()
{
    // ✅ MODIFIED: remove scale.begin() here
    if (!measure_enable)
    {
    Serial.println("stop mesurment");
    return;// stop measurement
    }
        scale.begin(HX711_DOUT, HX711_SCK); 
     if (millis() - last_weight_read < WEIGHT_INTERVAL_MS) return;
    last_weight_read = millis();
     
    delay(150);
    if (scale.is_ready())
    {
        float raw = scale.get_units(5); // average 5 readings
        weight = raw - weight_offset;
        if (weight < 0) weight = 0;
        
        home_screen_set_weight(weight);
        //if (lv_scr_act() == cal_scr) {
      
       // float w = scale.get_units(2);
       // if (w < 0) w = 0;
       // calibration_screen_set_weight(w);
       calibration_screen_set_weight(weight);
  //  }
        
        Serial.print("Weight: ");
        Serial.println(weight, 2);
    }
    else {
        Serial.println("[ERROR] HX711 NOT READY");
        delay(50);
    }
    
}

static void calibration_back_cb()
{
    lv_scr_load(settings_scr);
}

//static void calibration_do_cb()
//{/
  /////////////////////////////
  

  /////////////////////////////////////////
   //// weight_offset += weight;
//storage_save_offset(weight_offset);
    
// 🔴 ADDED: debug log
   // Serial.print("Calibration saved. Offset = ");
   // Serial.println(weight_offset);

    
//}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("=== SYSTEM START ===");

   
    // ------------------- INIT LVGL -------------------
    lvgl_port_init();

 // ------------------- INIT HX711 -------------------
    scale.begin(HX711_DOUT, HX711_SCK); // ✅ only once here
     delay(500); // small startup delay for HX711

   // scale.set_scale(2280.0f);
   
    //scale.tare();

    

    // 🔵 ADDED: restore scale factor
    scale_factor = storage_load_offset();
    if (scale_factor <= 0) scale_factor = 2280.0f;

    scale.set_scale(scale_factor);
    scale.tare();
    delay(200); // allow HX711 to stabilize

    Serial.print("[INIT] Scale factor = ");
    Serial.println(scale_factor);

    //weight_offset = storage_load_offset();
    storage_service_init();

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
    //calibration_screen_register_calibrate(calibration_do_cb);

    calibration_screen_register_offset(calib_offset);
    calibration_screen_register_scale(calib_scale);
    calibration_screen_register_both(calib_both);


    home_screen_set_quantity(qty);
    home_screen_set_weight(weight);
    home_screen_set_invoice(invoice_service_current_id());
    home_screen_update_history();

    // ✅ ADDED: initialize measure button text
    home_screen_set_measure_state(false);

    lv_scr_load(home_scr);
}

void loop()
{
    lvgl_port_loop();

  //  if (measure_enable)
   //// {
     //  scale.begin(HX711_DOUT, HX711_SCK); 
      //Serial.println("Added delay for measurement ready");
     // delay(150);
      update_weight(); // update weight live
     // }
     ////if (lv_scr_act() == cal_scr) {
      
       // float w = scale.get_units(2);
       // if (w < 0) w = 0;
       // calibration_screen_set_weight(w);
    //}

    wifi_service_loop();
    invoice_service_daily_reset_if_needed();
    storage_check_new_day_and_reset();

    if (lv_scr_act() == home_scr) {
        home_screen_set_sync_status(
            wifi_service_state() == WIFI_CONNECTED ? "Online" : "Offline"
        );
    }






   
}
