#include "scale_service_v2.h"
#include <HX711.h>
#include <math.h>

#define HX711_DOUT 43
#define HX711_SCK  44

static HX711 scale;

static scale_profile_t activeProfile =
{
    "DEFAULT",
    100.0f,
    2280.0f,
    0.25f,
    0.02f,
    1200
};

static float filtered_weight = 0;
static bool hold_state = false;

static TaskHandle_t scaleTaskHandle = NULL;

static float ema(float prev, float input, float alpha)
{
    return (alpha * input) + ((1.0f - alpha) * prev);
}

static void scale_task(void *p)
{
    scale.begin(HX711_DOUT, HX711_SCK);
    delay(2000);
    scale.set_scale(activeProfile.scale);
    scale.tare();

    while (true)
    {
        if (scale.is_ready())
        {
            float w = scale.get_units(3);
            if (w < 0) w = 0;

            filtered_weight = ema(
                filtered_weight,
                w,
                activeProfile.ema_alpha
            );
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

void scale_service_init()
{
    // Delay scale task startup so LVGL + RGB panel fully stabilize
    xTaskCreatePinnedToCore(
        scale_task,
        "scaleTask",
        8192,
        NULL,
        1,
        &scaleTaskHandle,
        1   // 🔥 MOVE TO CORE 1
    );
}


void scale_service_set_profile(const scale_profile_t *profile)
{
    if (!profile) return;

    activeProfile = *profile;
    scale.set_scale(activeProfile.scale);

    filtered_weight = 0;
    hold_state = false;
}

float scale_service_get_weight()
{
    return filtered_weight;
}

bool scale_service_is_hold()
{
    return hold_state;
}

void scale_service_tare()
{
    scale.tare();
}

long scale_service_get_raw()
{
    if (!scale.is_ready()) return 0;
    return scale.read();
}

const scale_profile_t* scale_service_get_profile()
{
    return &activeProfile;
}
