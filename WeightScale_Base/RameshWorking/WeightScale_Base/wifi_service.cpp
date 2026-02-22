#include "wifi_service.h"
#include <WiFi.h>
#include "scale_service_v2.h"
#include "devlog.h"

static wifi_state_t state = WIFI_DISCONNECTED;
static int scan_count = 0;
static String connected_ssid = "";

/* ===== deferred connect ===== */
static bool connect_request = false;
static char req_ssid[33] = {0};
static char req_pwd[65]  = {0};
static uint32_t connect_timeout = 0;

/* =====================================================
   INIT
=====================================================*/

void wifi_service_init(void)
{
    Serial.println("[WIFI] Initializing WiFi service");
    devlog_printf("[WIFI] Initializing WiFi service");
    
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(100);
    WiFi.disconnect(true);
    delay(100);
    
    state = WIFI_DISCONNECTED;
    Serial.println("[WIFI] WiFi service initialized");
    devlog_printf("[WIFI] WiFi service initialized");
}

/* =====================================================
   SCAN (blocking, stable)
=====================================================*/

void wifi_service_start_scan(void)
{
    Serial.println("[WIFI] Start scan");
    devlog_printf("[WIFI] Start scan");

    WiFi.scanDelete();
    scan_count = WiFi.scanNetworks(false);

    Serial.printf("[WIFI] Scan done: %d APs\n", scan_count);
    devlog_printf("[WIFI] Scan done: %d APs", scan_count);
}

/* =====================================================
   CONNECT (PUBLIC API — SAFE)
=====================================================*/

void wifi_service_connect(const char *ssid, const char *password)
{
    if(!ssid || !ssid[0]) return;

    strncpy(req_ssid, ssid, sizeof(req_ssid) - 1);
    req_ssid[sizeof(req_ssid) - 1] = 0;
    
    strncpy(req_pwd, password ? password : "", sizeof(req_pwd) - 1);
    req_pwd[sizeof(req_pwd) - 1] = 0;

    connect_request = true;

    /* Debug: show lengths (password masked) */
    Serial.printf("[WIFI] Connect requested (deferred) SSID='%s' len=%u pwd_len=%u\n",
                  req_ssid,
                  (unsigned)strlen(req_ssid),
                  (unsigned)strlen(req_pwd));
    devlog_printf("[WIFI] Connect requested (deferred) SSID='%s' len=%u pwd_len=%u",
                  req_ssid,
                  (unsigned)strlen(req_ssid),
                  (unsigned)strlen(req_pwd));

    /* Print first few pwd bytes as hex to detect non-printable/encoding issues (masked) */
    if(strlen(req_pwd) > 0)
    {
        Serial.print("[WIFI] pwd hex: ");
        for(size_t i=0;i<strlen(req_pwd) && i<8;i++)
        {
            Serial.printf("%02X ", (uint8_t)req_pwd[i]);
        }
        Serial.println();
        devlog_printf("[WIFI] pwd hex (first 8 bytes) %02X %02X %02X %02X %02X %02X %02X %02X",
                      (unsigned)(req_pwd[0] & 0xFF), (unsigned)(req_pwd[1] & 0xFF), (unsigned)(req_pwd[2] & 0xFF), (unsigned)(req_pwd[3] & 0xFF),
                      (unsigned)(req_pwd[4] & 0xFF), (unsigned)(req_pwd[5] & 0xFF), (unsigned)(req_pwd[6] & 0xFF), (unsigned)(req_pwd[7] & 0xFF));
    }
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
        connect_timeout = millis() + 30000;  /* 30 second timeout */

            Serial.println("[WIFI] Entering critical section");
            devlog_printf("[WIFI] Entering critical section");
            scale_service_suspend();
            wifi_critical_section = true;
            vTaskDelay(pdMS_TO_TICKS(50)); // 🔥 let RGB DMA stop
            
            /* Feed watchdog before WiFi operations */
            esp_task_wdt_reset();

            /* DEFENSIVE: Validate buffers before WiFi.begin() */
            if(strlen(req_ssid) == 0 || strlen(req_ssid) > 32)
            {
                Serial.printf("[WIFI] ERROR: Invalid SSID length=%u\n", (unsigned)strlen(req_ssid));
                devlog_printf("[WIFI] ERROR: Invalid SSID length=%u", (unsigned)strlen(req_ssid));
                scale_service_resume();
                wifi_critical_section = false;
                state = WIFI_DISCONNECTED;
                return;
            }
            
            if(strlen(req_pwd) > 63)
            {
                Serial.printf("[WIFI] ERROR: Invalid PWD length=%u\n", (unsigned)strlen(req_pwd));
                devlog_printf("[WIFI] ERROR: Invalid PWD length=%u", (unsigned)strlen(req_pwd));
                scale_service_resume();
                wifi_critical_section = false;
                state = WIFI_DISCONNECTED;
                return;
            }

            /* Verify null termination */
            if(req_ssid[sizeof(req_ssid)-1] != 0 || req_pwd[sizeof(req_pwd)-1] != 0)
            {
                Serial.println("[WIFI] ERROR: Buffers not null-terminated");
                devlog_printf("[WIFI] ERROR: Buffers not null-terminated");
                scale_service_resume();
                wifi_critical_section = false;
                state = WIFI_DISCONNECTED;
                return;
            }

            WiFi.setSleep(false);
            /* Ensure any previous connection attempt is cleared */
            Serial.println("[WIFI] Disconnecting before begin");
            devlog_printf("[WIFI] Disconnecting before begin");
            WiFi.disconnect(true);
            delay(50);
            
            /* Let popup fully close and LVGL stabilize */
            Serial.println("[WIFI] Waiting for UI to stabilize...");
            devlog_printf("[WIFI] Waiting for UI to stabilize");
            delay(200);
            
            Serial.printf("[WIFI] Starting WiFi.begin with SSID='%s' pwd_len=%u\n", req_ssid, (unsigned)strlen(req_pwd));
            devlog_printf("[WIFI] Starting WiFi.begin with SSID='%s' pwd_len=%u", req_ssid, (unsigned)strlen(req_pwd));
            
            /* DEBUG: Dump buffer contents before WiFi.begin */
            Serial.print("[WIFI] SSID buffer: ");
            for(int i = 0; i < 16 && i < (int)strlen(req_ssid); i++)
                Serial.printf("%02X ", (unsigned)(req_ssid[i] & 0xFF));
            Serial.println();
            
            Serial.print("[WIFI] PWD buffer: ");
            for(int i = 0; i < 16 && i < (int)strlen(req_pwd); i++)
                Serial.printf("%02X ", (unsigned)(req_pwd[i] & 0xFF));
            Serial.println();
            
            /* Call WiFi.begin with fresh copies to be extra safe */
            const char *ssid_ptr = req_ssid;
            const char *pwd_ptr = req_pwd;
            
            Serial.println("[WIFI] About to call WiFi.begin()");
            devlog_printf("[WIFI] About to call WiFi.begin()");
            
            /* Final sanity check right before WiFi.begin */
            if(!ssid_ptr || ssid_ptr[0] == 0)
            {
                Serial.println("[WIFI] ERROR: SSID pointer invalid at WiFi.begin");
                devlog_printf("[WIFI] ERROR: SSID pointer invalid at WiFi.begin");
                scale_service_resume();
                wifi_critical_section = false;
                state = WIFI_DISCONNECTED;
                return;
            }
            
            Serial.printf("[WIFI] Calling WiFi.begin('%s', pwd_len=%u)\n", ssid_ptr, strlen(pwd_ptr));
            WiFi.begin(ssid_ptr, pwd_ptr);
            
            Serial.println("[WIFI] WiFi.begin() returned successfully");

            /* Resume scale and clear critical section immediately so UI remains responsive
               while the connection proceeds in background. */
            scale_service_resume();
            wifi_critical_section = false;

            state = WIFI_CONNECTING;
            Serial.printf("[WIFI] WiFi.begin() called with SSID: %s\n", req_ssid);
            devlog_printf("[WIFI] WiFi.begin() called with SSID: %s", req_ssid);
    }

    if(state == WIFI_CONNECTING)
    {
        static uint32_t last_dbg = 0;
        uint32_t now = millis();
        if(now - last_dbg > 1000)
        {
            last_dbg = now;
            wl_status_t ss = WiFi.status();
            int32_t rem = (int32_t)(connect_timeout - now);
            if(rem < 0) rem = 0;
            Serial.printf("[WIFI] status=%d, timeout in %ld ms\n", (int)ss, (long)rem);
            devlog_printf("[WIFI] status=%d, timeout in %ld ms", (int)ss, (long)rem);
        }

        wl_status_t s = WiFi.status();

        if(s == WL_CONNECTED)
        {
            Serial.println("[WIFI] Connected");
            devlog_printf("[WIFI] Connected");
            state = WIFI_CONNECTED;

            vTaskDelay(pdMS_TO_TICKS(50));    // 🔥 let WiFi stabilize
            scale_service_resume();
            wifi_critical_section = false;
        }
        else if(s == WL_CONNECT_FAILED)
        {
            Serial.println("[WIFI] Failed");
            devlog_printf("[WIFI] Failed");
            state = WIFI_DISCONNECTED;
            scale_service_resume();

            wifi_critical_section = false;
        }
        else if(millis() > connect_timeout)
        {
            Serial.println("[WIFI] Connection timeout");
            devlog_printf("[WIFI] Connection timeout");
            state = WIFI_DISCONNECTED;
            WiFi.disconnect(true);
            wifi_critical_section = false;
            scale_service_resume();
        }
    }
}
