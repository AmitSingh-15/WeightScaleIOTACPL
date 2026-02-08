#include "scale_service_v2.h"   // ✅ FIXED HEADER NAME
#include <HX711.h>
#include <math.h>

#define HX711_DOUT 19
#define HX711_SCK  20

static HX711 scale;

static float filtered_weight = 0;
static bool hold_state = false;

static TaskHandle_t scaleTaskHandle;

static float ema(float prev, float input, float alpha)
{
    return (alpha * input) + ((1.0f - alpha) * prev);
}

static void scale_task(void *)
{
    scale.begin(HX711_DOUT, HX711_SCK);
    scale.set_scale(2280.0f);
    delay(2000);
    scale.tare();

    float lastStable = 0;
    uint32_t stableStart = millis();

    while (true)
    {
        if(scale.is_ready())
        {
            float w = scale.get_units(3);
            if(w < 0) w = 0;

            filtered_weight = ema(filtered_weight, w, 0.25f);

            if (fabs(filtered_weight - lastStable) < 0.02f)
            {
                if(millis() - stableStart > 1200)
                    hold_state = true;
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
        1);
}

void scale_service_start(){}

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
