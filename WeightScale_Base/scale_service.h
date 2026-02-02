#pragma once
#include <stdbool.h>
#include <stdint.h>

void  scale_service_init(void);
void  scale_service_loop(void);

float scale_service_get_weight(void);

void  scale_service_tare(void);
void  scale_service_set_scale(float factor);
float scale_service_get_scale(void);
void  scale_service_save(void);

bool scale_service_calibrate(uint8_t known_kg);
