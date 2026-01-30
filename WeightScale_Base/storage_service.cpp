#include "storage_service.h"
#include <Preferences.h>
#include "time_service.h"


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

void storage_save_offset(float val)
{
    prefs.putFloat("offset", val);
}

float storage_load_offset(void)
{
    return prefs.getFloat("offset", 0.0f);
}

void storage_add_full_record(const invoice_record_t *rec)
{
    uint32_t count = prefs.getUInt("rec_count", 0);

    char key[16];
    snprintf(key, sizeof(key), "rec_%lu", count);

    prefs.putBytes(key, rec, sizeof(invoice_record_t));
    prefs.putUInt("rec_count", count + 1);
}

uint8_t storage_get_last_records(invoice_record_t *out, uint8_t max)
{
    uint32_t count = storage_get_record_count();
    if (count == 0) return 0;

    uint8_t loaded = 0;
    int32_t start = count - 1;

    for (int32_t i = start; i >= 0 && loaded < max; i--)
    {
        char key[16];
        snprintf(key, sizeof(key), "rec_%ld", i);

        prefs.getBytes(key, &out[loaded], sizeof(invoice_record_t));
        loaded++;
    }

    return loaded;
}

void storage_clear_all_records(void)
{
    uint32_t count = storage_get_record_count();

    for (uint32_t i = 0; i < count; i++)
    {
        char key[16];
        snprintf(key, sizeof(key), "rec_%lu", i);
        prefs.remove(key);
    }

    prefs.putUInt("rec_count", 0);
}

void storage_check_new_day_and_reset(void)
{
    uint32_t last_day = prefs.getUInt("day_epoch", 0);
    uint32_t today = time_service_today_epoch_day();

    if (last_day != today)
    {
        storage_clear_all_records();
        prefs.putUInt("day_epoch", today);
    }
}

uint32_t storage_get_record_count(void)
{
    return prefs.getUInt("rec_count", 0);
}
