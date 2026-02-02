#include "scale_service.h"
#include <HX711.h>
#include <Preferences.h>
#include <Arduino.h>
#include <math.h>

#define HX711_DOUT 19
#define HX711_SCK  20

static HX711 scale;
static Preferences prefs;

static float scale_factor = 2280.0f;
static float last_weight  = 0.0f;

static bool  ready        = false;
static uint32_t start_ms  = 0;
static uint32_t last_read = 0;

#define HX711_WARMUP_MS     1500
#define HX711_INTERVAL_MS  200
#define HX711_SAMPLES      5
#define MAX_JUMP_KG        5.0f   // noise guard

void scale_service_init(void)
{
    scale.begin(HX711_DOUT, HX711_SCK);
    prefs.begin("scale", false);

    scale_factor = prefs.getFloat("factor", 2280.0f);
    scale.set_scale(scale_factor);

    start_ms = millis();
    ready = false;
}

void scale_service_loop(void)
{
    if (!ready) {
        if (millis() - start_ms >= HX711_WARMUP_MS && scale.is_ready()) {
            scale.tare();
            ready = true;
        }
        return;
    }

    if (millis() - last_read < HX711_INTERVAL_MS) return;
    last_read = millis();

    if (!scale.is_ready()) return;

    float v = scale.get_units(HX711_SAMPLES);
    if (v < 0) v = 0;

    // noise / spike rejection
    if (fabs(v - last_weight) > MAX_JUMP_KG) return;

    last_weight = v;
}

float scale_service_get_weight(void)
{
    return last_weight;
}

void scale_service_tare(void)
{
    if (!ready) return;
    scale.tare();
    last_weight = 0;
}

void scale_service_set_scale(float factor)
{
    scale_factor = factor;
    scale.set_scale(scale_factor);
}

float scale_service_get_scale(void)
{
    return scale_factor;
}

void scale_service_save(void)
{
    prefs.putFloat("factor", scale_factor);
}

bool scale_service_calibrate(uint8_t known_kg)
{
    if (!ready || known_kg == 0) return false;

    scale.set_scale(1.0f);           // 🔥 raw units
    float raw = scale.get_units(15);
    if (raw <= 0) return false;

    scale_factor = raw / (float)known_kg;
    scale.set_scale(scale_factor);
    prefs.putFloat("factor", scale_factor);

    return true;
}
