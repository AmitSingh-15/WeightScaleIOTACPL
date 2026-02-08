
#pragma once
#include <Arduino.h>

void scale_service_init();
void scale_service_start();

float scale_service_get_weight();
bool  scale_service_is_hold();

void scale_service_tare();
