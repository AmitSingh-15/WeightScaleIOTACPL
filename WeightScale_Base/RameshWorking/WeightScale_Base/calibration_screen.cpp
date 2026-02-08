#include "calibration_screen.h"
#include "ui_styles.h"

static lv_obj_t *lbl_live;
static void (*back_cb)(void) = NULL;
static void (*offset_cb)(void) = NULL;
static void (*scale_cb)(void)  = NULL;
static void (*both_cb)(void)   = NULL;

static void back_evt(lv_event_t *e){ if(back_cb) back_cb(); }
static void offset_evt(lv_event_t *e){ if(offset_cb) offset_cb(); }
static void scale_evt(lv_event_t *e){ if(scale_cb) scale_cb(); }
static void both_evt(lv_event_t *e){ if(both_cb) both_cb(); }

void calibration_screen_register_back(void (*cb)(void)){ back_cb = cb; }
void calibration_screen_register_offset(void (*cb)(void)) { offset_cb = cb; }
void calibration_screen_register_scale(void (*cb)(void))  { scale_cb = cb; }
void calibration_screen_register_both(void (*cb)(void))   { both_cb = cb; }

void calibration_screen_create(lv_obj_t *parent)
{
    ui_styles_init();
    lv_obj_t *scr = parent;
    lv_obj_add_style(scr, &g_styles.screen, 0);

    /* HEADER */
    lv_obj_t *header = lv_obj_create(scr);
    lv_obj_add_style(header, &g_styles.card, 0);
    lv_obj_set_size(header, 780, 60);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 5);

    lv_label_set_text(lv_label_create(header), "CALIBRATION");

    /* BIG VALUE */
    lbl_live = lv_label_create(scr);
    lv_obj_add_style(lbl_live, &g_styles.value_big, 0);
    lv_obj_align(lbl_live, LV_ALIGN_CENTER, 0, -40);

    /* BUTTON BAR */
    lv_obj_t *bar = lv_obj_create(scr);
    lv_obj_add_style(bar, &g_styles.card, 0);
    lv_obj_set_size(bar, 780, 80);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_t *btn_back = lv_btn_create(bar);
    lv_obj_add_style(btn_back, &g_styles.btn_secondary, 0);
    lv_obj_align(btn_back, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_event_cb(btn_back, back_evt, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(btn_back), "BACK");

    lv_obj_t *btn_offset = lv_btn_create(bar);
    lv_obj_add_style(btn_offset, &g_styles.btn_secondary, 0);
    lv_obj_align(btn_offset, LV_ALIGN_CENTER, -120, 0);
    lv_obj_add_event_cb(btn_offset, offset_evt, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(btn_offset), "OFFSET");

    lv_obj_t *btn_scale = lv_btn_create(bar);
    lv_obj_add_style(btn_scale, &g_styles.btn_secondary, 0);
    lv_obj_align(btn_scale, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_scale, scale_evt, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(btn_scale), "SCALE");

    lv_obj_t *btn_both = lv_btn_create(bar);
    lv_obj_add_style(btn_both, &g_styles.btn_secondary, 0);
    lv_obj_align(btn_both, LV_ALIGN_CENTER, 120, 0);
    lv_obj_add_event_cb(btn_both, both_evt, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(btn_both), "BOTH");
}

void calibration_screen_set_weight(float weight)
{
    static char buf[16];
    int value = (int)(weight * 100);
    lv_snprintf(buf, sizeof(buf), "%d.%02d", value / 100, abs(value % 100));
    lv_label_set_text(lbl_live, buf);
}
