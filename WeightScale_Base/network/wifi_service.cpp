#include "wifi_service.h"
#include <WiFi.h>

static wifi_state_t state = WIFI_DISCONNECTED;
static int scan_count = 0;

void wifi_service_init(void)
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
}

void wifi_service_start_scan(void)
{
    scan_count = WiFi.scanNetworks(true);
}

uint8_t wifi_service_get_ap_count(void)
{
    return (scan_count < 0) ? 0 : scan_count;
}

String wifi_service_get_ssid(uint8_t index)
{
    return WiFi.SSID(index);
}

void wifi_service_connect(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);
    state = WIFI_CONNECTING;
}

wifi_state_t wifi_service_state(void)
{
    return state;
}

void wifi_service_loop(void)
{
    if (state == WIFI_CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            state = WIFI_CONNECTED;
        } else if (WiFi.status() == WL_CONNECT_FAILED) {
            state = WIFI_DISCONNECTED;
        }
    }

    int res = WiFi.scanComplete();
    if (res >= 0) {
        scan_count = res;
    }
}

