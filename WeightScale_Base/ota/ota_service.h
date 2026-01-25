#pragma once
#include <Arduino.h>

void ota_service_init(void);
void ota_service_check_and_update(void);

const char* ota_service_current_version(void);
