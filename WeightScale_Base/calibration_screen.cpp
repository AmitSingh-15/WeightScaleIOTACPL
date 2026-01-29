#include "calibration_screen.h"
#include "ui_styles.h"

static lv_obj_t *lbl_live;
static void (*back_cb)(void) = NULL;
static void (*cal_cb)(void) = NULL;

static void back_evt(lv_event_t *e){ if(back_cb) back_cb(); }
static void cal_evt(lv_event_t *e){ if(cal_cb) cal_cb(); }

void calibration_screen_register_back(void (*cb)(void)){ back_cb = cb; }
void calibration_screen_register_calibrate(void (*cb)(void)){ cal_cb = cb; }

void calibration_screen_create(lv_obj_t *parent)
{
    ui_styles_init();
    lv_obj_t *scr = parent;
    lv_obj_add_style(scr, &g_styles.screen, 0);

    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_add_style(card, &g_styles.card, 0);
    lv_obj_set_size(card, 600, 300);
    lv_obj_center(card);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "CALIBRATION");
    lv_obj_add_style(title, &g_styles.title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lbl_live = lv_label_create(card);
    lv_label_set_text(lbl_live, "0.00");
    lv_obj_add_style(lbl_live, &g_styles.value_big, 0);
    lv_obj_align(lbl_live, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *btn_cal = lv_btn_create(card);
    lv_obj_add_style(btn_cal, &g_styles.btn_primary, 0);
    lv_obj_align(btn_cal, LV_ALIGN_CENTER, 0, 40);
    lv_label_set_text(lv_label_create(btn_cal), "CALIBRATE");
    lv_obj_add_event_cb(btn_cal, cal_evt, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_back = lv_btn_create(card);
    lv_obj_add_style(btn_back, &g_styles.btn_secondary, 0);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_label_set_text(lv_label_create(btn_back), "BACK");
    lv_obj_add_event_cb(btn_back, back_evt, LV_EVENT_CLICKED, NULL);
}

void calibration_screen_set_weight(float w)
{
    static char buf[16];
    lv_snprintf(buf, sizeof(buf), "%.2f", w);
    lv_label_set_text(lbl_live, buf);
}
