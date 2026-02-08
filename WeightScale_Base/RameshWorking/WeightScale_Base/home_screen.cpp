#include "home_screen.h"
#include "ui_styles.h"
#include "ui_events.h"
#include <stdlib.h>
#include <stdio.h>
#include "invoice_service.h"
#include "storage_service.h"

static lv_obj_t *lbl_weight;
static lv_obj_t *lbl_qty;
static lv_obj_t *lbl_total;
static lv_obj_t *lbl_invoice;
static lv_obj_t *lbl_sync;
static lv_obj_t *history_lbl[5];
static lv_obj_t *btn_measure;
static lv_obj_t *lbl_measure;

static void (*event_cb)(int evt) = NULL;

static void btn_event_cb(lv_event_t *e)
{
    if (!event_cb) return;
    uintptr_t id = (uintptr_t)lv_event_get_user_data(e);
    event_cb((int)id);
}

void home_screen_register_callback(void (*cb)(int evt))
{
    event_cb = cb;
}

void home_screen_create(lv_obj_t *parent)
{
    ui_styles_init();

    lv_obj_t *screen = parent;
    lv_obj_add_style(screen, &g_styles.screen, 0);
    lv_obj_set_size(screen, 800, 480);

    static lv_coord_t col[] = { LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t row[] = { 90, 160, 120, 80, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(screen, col, row);

    /* HEADER */
    lv_obj_t *header = lv_obj_create(screen);
    lv_obj_add_style(header, &g_styles.card, 0);
    lv_obj_set_grid_cell(header, LV_GRID_ALIGN_STRETCH, 0, 3,
                                   LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *brand1 = lv_label_create(header);
    lv_label_set_text(brand1, "ACPL delivering progress");
    lv_obj_align(brand1, LV_ALIGN_TOP_MID, 0, 2);

    lv_obj_t *brand2 = lv_label_create(header);
    lv_label_set_text(brand2, "powered by Vastotech");
    lv_obj_align(brand2, LV_ALIGN_TOP_MID, 0, 20);

    lbl_invoice = lv_label_create(header);
    lv_obj_align(lbl_invoice, LV_ALIGN_LEFT_MID, 10, 10);

    lbl_sync = lv_label_create(header);
    lv_obj_align(lbl_sync, LV_ALIGN_RIGHT_MID, -80, 10);

    lv_obj_t *settings_btn = lv_btn_create(header);
    lv_obj_add_style(settings_btn, &g_styles.btn_secondary, 0);
    lv_obj_set_size(settings_btn, 70, 40);
    lv_obj_align(settings_btn, LV_ALIGN_RIGHT_MID, -5, 10);
    lv_obj_add_event_cb(settings_btn, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_SETTINGS);
    lv_label_set_text(lv_label_create(settings_btn), "SET");

    /* WEIGHT */
    lv_obj_t *weight_card = lv_obj_create(screen);
    lv_obj_add_style(weight_card, &g_styles.card, 0);
    lv_obj_set_grid_cell(weight_card, LV_GRID_ALIGN_STRETCH, 0, 2,
                                      LV_GRID_ALIGN_STRETCH, 1, 1);

    lbl_weight = lv_label_create(weight_card);
    lv_obj_add_style(lbl_weight, &g_styles.value_big, 0);
    lv_obj_center(lbl_weight);

    /* INFO CARD */
    lv_obj_t *info_card = lv_obj_create(screen);
    lv_obj_add_style(info_card, &g_styles.card, 0);
    lv_obj_set_grid_cell(info_card, LV_GRID_ALIGN_STRETCH, 2, 1,
                                      LV_GRID_ALIGN_STRETCH, 1, 1);

    lbl_qty = lv_label_create(info_card);
    lv_obj_align(lbl_qty, LV_ALIGN_TOP_MID, 0, 20);

    lbl_total = lv_label_create(info_card);
    lv_obj_align(lbl_total, LV_ALIGN_TOP_MID, 0, 60);

    /* HISTORY */
    lv_obj_t *hist_card = lv_obj_create(screen);
    lv_obj_add_style(hist_card, &g_styles.card, 0);
    lv_obj_set_grid_cell(hist_card, LV_GRID_ALIGN_STRETCH, 2, 1,
                                    LV_GRID_ALIGN_STRETCH, 2, 1);

    lv_label_set_text(lv_label_create(hist_card), "Last 5");

    for(int i=0;i<5;i++){
        history_lbl[i]=lv_label_create(hist_card);
        lv_label_set_text(history_lbl[i],"-");
        lv_obj_align(history_lbl[i],LV_ALIGN_TOP_LEFT,5,25+i*20);
    }

    /* MEASURE BUTTON */
    btn_measure = lv_btn_create(screen);
    lv_obj_add_style(btn_measure, &g_styles.btn_secondary, 0);
    lv_obj_set_size(btn_measure, 150, 60);
    lv_obj_set_grid_cell(btn_measure,
                         LV_GRID_ALIGN_CENTER, 0, 1,
                         LV_GRID_ALIGN_CENTER, 3, 1);

    lv_obj_add_event_cb(btn_measure, btn_event_cb,
                        LV_EVENT_CLICKED, (void*)UI_EVT_MEASURE);

    lbl_measure = lv_label_create(btn_measure);
    lv_label_set_text(lbl_measure, "MEASURE");
    lv_obj_center(lbl_measure);
}

/* ===== SETTERS (ALL RESTORED) ===== */

void home_screen_set_weight(float weight)
{
    static char buf[16];
    int value = (int)(weight * 100);
    lv_snprintf(buf, sizeof(buf), "%d.%02d", value / 100, abs(value % 100));
    lv_label_set_text(lbl_weight, buf);
}

void home_screen_set_quantity(uint16_t qty)
{
    static char buf[16];
    lv_snprintf(buf, sizeof(buf), "Qty: %d", qty);
    lv_label_set_text(lbl_qty, buf);
}

void home_screen_set_total_weight(float total)
{
    static char buf[24];
    int value = (int)(total * 100);
    lv_snprintf(buf, sizeof(buf), "Total: %d.%02dkg", value/100, abs(value%100));
    lv_label_set_text(lbl_total, buf);
}

void home_screen_set_invoice(uint32_t invoice_id)
{
    static char buf[24];
    lv_snprintf(buf, sizeof(buf), "Invoice #%lu", invoice_id);
    lv_label_set_text(lbl_invoice, buf);
}

void home_screen_set_sync_status(const char *txt)
{
    lv_label_set_text(lbl_sync, txt);
}

void home_screen_set_measure_state(bool enabled)
{
    if (!btn_measure || !lbl_measure) return;

    if (enabled) {
        lv_label_set_text(lbl_measure, "STOP");
        lv_obj_add_style(btn_measure, &g_styles.btn_danger, 0);
    } else {
        lv_label_set_text(lbl_measure, "MEASURE");
        lv_obj_add_style(btn_measure, &g_styles.btn_secondary, 0);
    }
}

void home_screen_update_history(void)
{
    invoice_record_t recs[5];
    uint8_t count = storage_get_last_records(recs, 5);

    for (int i = 0; i < 5; i++) {
        if (i < count) {
            static char buf[32];
            snprintf(buf, sizeof(buf),
                     "#%lu  %.2fkg",
                     recs[i].invoice_id,
                     recs[i].weight);
            lv_label_set_text(history_lbl[i], buf);
        } else {
            lv_label_set_text(history_lbl[i], "-");
        }
    }
}
