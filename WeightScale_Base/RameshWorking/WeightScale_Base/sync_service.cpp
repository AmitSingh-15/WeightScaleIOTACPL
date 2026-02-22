#include "sync_service.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "storage_service.h"
#include "wifi_service.h"
#include "devlog.h"

// Health and bulk endpoints
static const char *HEALTH_URL = "https://dev.etranscargo.in/weightscale/health";
static const char *BULK_URL = "https://dev.etranscargo.in/api/WeightIngestion/bulk";

void sync_service_init(void)
{
    // nothing for now
}

static String build_payload(void)
{
    uint32_t count = storage_get_record_count();
    if(count == 0) return String("{}");

    char devname[64] = {0};
    storage_load_device_name(devname, sizeof(devname));

    String s = "{";
    s += "\"deviceId\":0,";
    s += "\"deviceName\":\"" + String(devname) + "\",";
    s += "\"weightsInKg\": [";

    bool first = true;
    for(uint32_t i = 0; i < count; i++)
    {
        invoice_record_t rec;
        if(storage_get_record_by_index(i, &rec))
        {
            if(rec.synced) continue; /* skip already-synced records */
            if(!first) s += ",";
            s += String(rec.total_weight, 3);
            first = false;
        }
    }

    s += "]}";
    return s;
}

void sync_service_loop(void)
{
    // Only attempt sync if WiFi is connected and there are pending records
    if(wifi_service_state() != WIFI_CONNECTED) return;

    uint32_t pending = storage_get_pending_count();
    uint32_t rec_count = storage_get_record_count();

    if(pending == 0 || rec_count == 0) return;

    HTTPClient http;
    // Health check
    http.begin(HEALTH_URL);
    int code = http.GET();
    String payload = http.getString();
    http.end();
    devlog_printf("[SYNC] Health check code=%d payload=%s", code, payload.c_str());

    if(code >= 200 && code < 300)
    {
        String body = payload;
        body.trim();
        if(body.indexOf("Healthy") >= 0)
        {
            // Build JSON payload and POST
            String post = build_payload();
            devlog_printf("[SYNC] Posting payload: %s", post.c_str());

            http.begin(BULK_URL);
            http.addHeader("Content-Type", "application/json");
            int postCode = http.POST(post);

            String resp = http.getString();
            http.end();

            if(postCode >= 200 && postCode < 300)
            {
                devlog_printf("[SYNC] POST success code=%d resp=%s", postCode, resp.c_str());
                // Success — mark uploaded records as synced and reset pending counter
                for(uint32_t i = 0; i < rec_count; i++)
                {
                    invoice_record_t rec;
                    if(storage_get_record_by_index(i, &rec))
                    {
                        if(rec.synced == 0)
                        {
                            rec.synced = 1;
                            storage_update_record(i, &rec);
                        }
                    }
                }
                storage_reset_pending();
            }
        }
    }
}
