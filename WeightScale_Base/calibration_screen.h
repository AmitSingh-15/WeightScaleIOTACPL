#pragma once
#include <lvgl.h>

void calibration_screen_create(lv_obj_t *parent);
void calibration_screen_set_weight(float w);
void calibration_screen_register_back(void (*cb)(void));
void calibration_screen_register_calibrate(void (*cb)(void));
