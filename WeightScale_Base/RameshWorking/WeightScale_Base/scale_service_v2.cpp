#include "scale_service_v2.h"
#include <HX711.h>
#include <math.h>

#define HX711_DOUT 19
#define HX711_SCK  20

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


static TaskHandle_t scaleTaskHandle;

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

    float lastStable = 0;
    uint32_t stableStart = millis();

    while(true)
    {
        if(scale.is_ready())
        {
            float w = scale.get_units(3);
            if(w < 0) w = 0;

            filtered_weight = ema(
                filtered_weight,
                w,
                activeProfile.ema_alpha
            );

            if(fabs(filtered_weight - lastStable) <
               activeProfile.hold_threshold)
            {
                if(millis() - stableStart >
                   activeProfile.hold_time_ms)
                {
                    hold_state = true;
                }
            }
            else
            {
                hold_state = false;
                lastStable = filtered_weight;
                stableStart = millis();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

void scale_service_init()
{
    xTaskCreatePinnedToCore(
        scale_task,
        "scaleTask",
        4096,
        NULL,
        1,
        &scaleTaskHandle,
        0
    );
}

void scale_service_set_profile(const scale_profile_t *profile)
{
    if(!profile) return;

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
    if(!scale.is_ready()) return 0;
    return scale.read();
}
