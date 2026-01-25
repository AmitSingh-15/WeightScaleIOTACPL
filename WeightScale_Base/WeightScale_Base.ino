
#include <Arduino.h>
#include "gfx_conf.h"
#include "lvgl_port.h"
#include "ui/home_screen.h"
#include "ui/settings_screen.h"
#include "services/invoice_service.h"
#include "network/wifi_service.h"
#include "ota/ota_service.h"

void setup() {
  Serial.begin(115200);
  lvgl_port_init();
  invoice_service_init();
  wifi_service_init();
  ota_service_init();
  home_screen_create(lv_scr_act());
}

void loop() {
  lvgl_port_loop();
  wifi_service_loop();
  invoice_service_daily_reset_if_needed();
}
