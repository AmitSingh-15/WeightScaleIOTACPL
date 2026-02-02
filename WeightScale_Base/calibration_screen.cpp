#include "calibration_screen.h"
#include "ui_styles.h"
#include "scale_service.h"
#include <stdio.h>
#include <stdint.h>

static lv_obj_t *lbl_info;
static lv_obj_t *lbl_weight;

static uint8_t selected_kg = 0;
static void (*cb_back)(void) = nullptr;

static void zero_evt(lv_event_t *)
{
    scale_service_tare();
    lv_label_set_text(lbl_info, "Select known weight");
}

static void kg_evt(lv_event_t *e)
{
    selected_kg = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    lv_label_set_text(lbl_info, "Place weight and press CALIBRATE");
}

static void cal_evt(lv_event_t *)
{
    if (selected_kg == 0) {
        lv_label_set_text(lbl_info, "Select weight first");
        return;
    }

    if (scale_service_calibrate(selected_kg)) {
        lv_label_set_text(lbl_info, "Calibration successful");
    } else {
        lv_label_set_text(lbl_info, "Calibration failed");
    }
}


static void back_evt(lv_event_t *)
{
    if (cb_back) cb_back();
}

    static lv_obj_t* mk_btn(lv_obj_t *parent,
                            const char *txt,
                            int x,
                            lv_event_cb_t cb,
                            void *ud)
    {
        lv_obj_t *b = lv_btn_create(parent);
        lv_obj_set_size(b, 120, 50);
        lv_obj_align(b, LV_ALIGN_CENTER, x, 40);
        lv_label_set_text(lv_label_create(b), txt);
        lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, ud);
        return b;
    }

void calibration_screen_create(lv_obj_t *parent)
{
    ui_styles_init();
    lv_obj_add_style(parent, &g_styles.screen, 0);

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_add_style(card, &g_styles.card, 0);
    lv_obj_set_size(card, 700, 400);
    lv_obj_center(card);

    lbl_weight = lv_label_create(card);
    lv_label_set_text(lbl_weight, "0.00 kg");
    lv_obj_set_style_text_font(lbl_weight, &lv_font_montserrat_48, 0);
    lv_obj_align(lbl_weight, LV_ALIGN_TOP_MID, 0, 20);

    lbl_info = lv_label_create(card);
    lv_label_set_text(lbl_info, "Remove all weight\nPress ZERO");
    lv_obj_align(lbl_info, LV_ALIGN_TOP_MID, 0, 100);




    mk_btn(card, "ZERO", -180, zero_evt, nullptr);
    mk_btn(card, "1 KG", -60, kg_evt, (void*)1);
    mk_btn(card, "5 KG", 60, kg_evt, (void*)5);
    mk_btn(card, "10 KG", 180, kg_evt, (void*)10);
    mk_btn(card, "CALIBRATE", 0, cal_evt, nullptr);


    lv_obj_t *back = lv_btn_create(card);
    lv_obj_align(back, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_label_set_text(lv_label_create(back), "BACK");
    lv_obj_add_event_cb(back, back_evt, LV_EVENT_CLICKED, nullptr);
}

void calibration_screen_set_live_weight(float w)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "%.2f kg", w);
    lv_label_set_text(lbl_weight, buf);
}

void calibration_register_back(void (*cb)(void))
{
    cb_back = cb;
}
