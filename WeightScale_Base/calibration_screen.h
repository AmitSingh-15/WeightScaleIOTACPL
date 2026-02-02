#pragma once
#include <lvgl.h>

void calibration_screen_create(lv_obj_t *parent);
void calibration_screen_set_live_weight(float weight);
void calibration_register_back(void (*cb)(void));
