
#include <Arduino.h>
#include "gfx_conf.h"
#include "lvgl_port.h"
#include "ui/home_screen.h"
#include "ui/settings_screen.h"
#include "services/invoice_service.h"
#include "network/wifi_service.h"
#include "ota/ota_service.h"

static lv_obj_t *home_scr;
static lv_obj_t *settings_scr;

static lv_obj_t *home_scr;
static lv_obj_t *settings_scr;
static uint16_t current_quantity = 1;
static float current_weight = 0.0; // TODO: from sensor

static void ui_event_handler(int evt)
{
    switch (evt) {
    case UI_EVT_SETTINGS:
        lv_scr_load(settings_scr);
        break;
    case UI_EVT_QTY_INC:
        if (current_quantity < 999) {
            current_quantity++;
            home_screen_set_quantity(current_quantity);
        }
        break;
    case UI_EVT_QTY_DEC:
        if (current_quantity > 1) {
            current_quantity--;
            home_screen_set_quantity(current_quantity);
        }
        break;
    case UI_EVT_SAVE:
        // TODO: get weight from sensor
        invoice_record_t rec;
        if (invoice_service_save(current_weight, current_quantity, ENTRY_MANUAL, &rec)) {
            invoice_service_next();
            home_screen_set_invoice(invoice_service_current_id());
            current_quantity = 1;
            home_screen_set_quantity(current_quantity);
            Serial.printf("Saved invoice %lu\n", rec.invoice_id);
        }
        break;
    }
}

static void settings_back_handler(void)
{
    lv_scr_load(home_scr);
}

void setup() {
  Serial.begin(115200);
  lvgl_port_init();
  invoice_service_init();
  wifi_service_init();
  ota_service_init();

  home_scr = lv_obj_create(NULL);
  settings_scr = lv_obj_create(NULL);

  home_screen_create(home_scr);
  home_screen_register_callback(ui_event_handler);
  home_screen_set_quantity(current_quantity);
  home_screen_set_weight(current_weight);
  home_screen_set_invoice(invoice_service_current_id());

  settings_screen_create(settings_scr);
  settings_screen_register_back_callback(settings_back_handler);

  lv_scr_load(home_scr);
}

void loop() {
  lvgl_port_loop();
  wifi_service_loop();
  invoice_service_daily_reset_if_needed();

  // Update UI statuses
  if (lv_scr_act() == settings_scr) {
    settings_screen_update_wifi_status();
  } else if (lv_scr_act() == home_scr) {
    wifi_state_t wifi = wifi_service_state();
    const char *sync = (wifi == WIFI_CONNECTED) ? "Online" : "Offline";
    home_screen_set_sync_status(sync);
  }
}
