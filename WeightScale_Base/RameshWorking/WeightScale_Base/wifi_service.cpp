#include "wifi_service.h"
#include <WiFi.h>

static wifi_state_t state = WIFI_DISCONNECTED;
static int scan_count = 0;
static String connected_ssid = "";

/* ===== deferred connect ===== */
static bool connect_request = false;
static char req_ssid[33] = {0};
static char req_pwd[65]  = {0};

/* =====================================================
   INIT
=====================================================*/

void wifi_service_init(void)
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    state = WIFI_DISCONNECTED;
}

/* =====================================================
   SCAN (blocking, stable)
=====================================================*/

void wifi_service_start_scan(void)
{
    Serial.println("[WIFI] Start scan");

    WiFi.scanDelete();
    scan_count = WiFi.scanNetworks(false);

    Serial.printf("[WIFI] Scan done: %d APs\n", scan_count);
}

/* =====================================================
   CONNECT (PUBLIC API — SAFE)
=====================================================*/

void wifi_service_connect(const char *ssid, const char *password)
{
    if(!ssid || !ssid[0]) return;

    strncpy(req_ssid, ssid, sizeof(req_ssid));
    strncpy(req_pwd, password ? password : "", sizeof(req_pwd));

    connect_request = true;

    Serial.println("[WIFI] Connect requested (deferred)");
}

/* =====================================================
   ACCESS POINT LIST
=====================================================*/

uint8_t wifi_service_get_ap_count(void)
{
    return scan_count > 0 ? (uint8_t)scan_count : 0;
}

String wifi_service_get_ssid(uint8_t index)
{
    if(index >= scan_count) return "";
    return WiFi.SSID(index);
}

/* =====================================================
   STATE
=====================================================*/

wifi_state_t wifi_service_state(void)
{
    return state;
}

/* =====================================================
   LOOP (ONLY place WiFi actually starts)
=====================================================*/

extern bool wifi_critical_section;

void wifi_service_loop(void)
{
    if(connect_request)
    {
        connect_request = false;

        Serial.println("[WIFI] Entering critical section");

        wifi_critical_section = true;
        delay(50);   // 🔥 let RGB DMA stop

        WiFi.setSleep(false);
        WiFi.begin(req_ssid, req_pwd);

        state = WIFI_CONNECTING;
    }

    if(state == WIFI_CONNECTING)
    {
        wl_status_t s = WiFi.status();

        if(s == WL_CONNECTED)
        {
            Serial.println("[WIFI] Connected");
            state = WIFI_CONNECTED;

            delay(50);   // 🔥 let WiFi stabilize
            wifi_critical_section = false;
        }
        else if(s == WL_CONNECT_FAILED)
        {
            Serial.println("[WIFI] Failed");
            state = WIFI_DISCONNECTED;

            wifi_critical_section = false;
        }
    }
}
