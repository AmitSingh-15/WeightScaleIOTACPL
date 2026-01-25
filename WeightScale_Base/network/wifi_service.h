#pragma once
#include <Arduino.h>

typedef enum {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED
} wifi_state_t;

void wifi_service_init(void);
void wifi_service_loop(void);

wifi_state_t wifi_service_state(void);

void wifi_service_start_scan(void);
uint8_t wifi_service_get_ap_count(void);
String wifi_service_get_ssid(uint8_t index);

void wifi_service_connect(const char *ssid, const char *password);
