#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include "ota_service.h"

#define OTA_VERSION      "1.0.0"
#define OTA_VERSION_URL  "https://your-server.com/firmware/version.txt"
#define OTA_BIN_URL      "https://your-server.com/firmware/weights.bin"

static WiFiClientSecure client;

const char* ota_service_current_version(void)
{
    return OTA_VERSION;
}

static bool is_new_version_available(String remote)
{
    return remote != OTA_VERSION;
}

void ota_service_init(void)
{
    client.setInsecure(); // replace with cert later if needed
}

void ota_service_check_and_update(void)
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[OTA] Wi-Fi not connected");
        return;
    }

    HTTPClient http;
    http.begin(OTA_VERSION_URL);
    int code = http.GET();

    if (code != 200) {
        Serial.println("[OTA] Version check failed");
        http.end();
        return;
    }

    String remote_version = http.getString();
    remote_version.trim();
    http.end();

    if (!is_new_version_available(remote_version)) {
        Serial.println("[OTA] Already latest version");
        return;
    }

    Serial.printf("[OTA] Updating from %s → %s\n",
                  OTA_VERSION, remote_version.c_str());

    t_httpUpdate_return ret = httpUpdate.update(client, OTA_BIN_URL);

    switch (ret) {
        case HTTP_UPDATE_OK:
            Serial.println("[OTA] Update success, rebooting");
            break;

        case HTTP_UPDATE_FAILED:
            Serial.printf("[OTA] Update failed (%d): %s\n",
                          httpUpdate.getLastError(),
                          httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("[OTA] No update available");
            break;
    }
}
