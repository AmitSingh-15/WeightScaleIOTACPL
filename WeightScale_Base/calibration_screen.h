#pragma once
#include <lvgl.h>

void calibration_screen_create(lv_obj_t *parent);
void calibration_screen_set_weight(float weight);
void calibration_screen_register_back(void (*cb)(void));
//void calibration_screen_register_calibrate(void (*cb)(void));
void calibration_screen_register_offset(void (*cb)(void));
void calibration_screen_register_scale(void (*cb)(void));
void calibration_screen_register_both(void (*cb)(void));
