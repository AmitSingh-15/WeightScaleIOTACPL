#include "storage_service.h"
#include <Preferences.h>

static Preferences prefs;

void storage_service_init(void)
{
    prefs.begin("weights", false);
}

void storage_save_invoice(uint32_t id)
{
    prefs.putUInt("invoice_id", id);
}

void storage_load_invoice(uint32_t *id)
{
    *id = prefs.getUInt("invoice_id", 1);
}

void storage_save_last_day(uint32_t day)
{
    prefs.putUInt("last_day", day);
}

uint32_t storage_load_last_day(void)
{
    return prefs.getUInt("last_day", 0);
}

/*
 * Offline queue hook
 * STEP-4 will attach Wi-Fi + sync
 */
bool storage_enqueue_record(const invoice_record_t *rec)
{
    // For now: persist minimal record count
    uint32_t count = prefs.getUInt("pending", 0);
    prefs.putUInt("pending", count + 1);

    // Real SPIFFS/JSON batching comes in Step-4
    return true;
}
