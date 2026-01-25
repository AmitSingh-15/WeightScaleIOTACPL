#pragma once
#include "lvgl.h"

void settings_screen_create(lv_obj_t *parent);
void settings_screen_register_back_callback(void (*cb)(void));
void settings_screen_update_wifi_status(void);
