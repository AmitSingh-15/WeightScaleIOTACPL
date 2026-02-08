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

    /* ===== MAIN FLEX ROW ===== */
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_ROW);

    /* ============================================================
       LEFT SIDE — BIG WEIGHT (60%)
       ============================================================ */

    lv_obj_t *left = lv_obj_create(screen);
    lv_obj_add_style(left, &g_styles.card, 0);
    lv_obj_set_size(left, 480, 460);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *wt_title = lv_label_create(left);
    lv_label_set_text(wt_title, "WEIGHT (kg)");
    lv_obj_add_style(wt_title, &g_styles.title, 0);
    lv_obj_align(wt_title, LV_ALIGN_TOP_MID, 0, 5);

    lbl_weight = lv_label_create(left);

    /* HUGE FONT */
    lv_obj_set_style_text_font(lbl_weight, &lv_font_montserrat_48, 0);

    lv_label_set_text(lbl_weight, "0.00");
    lv_obj_center(lbl_weight);

    /* ============================================================
       RIGHT SIDE — CONTROL PANEL (40%)
       ============================================================ */

    lv_obj_t *right = lv_obj_create(screen);
    lv_obj_add_style(right, &g_styles.card, 0);
    lv_obj_set_size(right, 300, 460);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(right, 8, 0);

    /* BRANDING */
    lv_obj_t *brand1 = lv_label_create(right);
    lv_label_set_text(brand1, "ACPL delivering progress");
    lv_obj_add_style(brand1, &g_styles.title, 0);

    lv_obj_t *brand2 = lv_label_create(right);
    lv_label_set_text(brand2, "powered by Vastotech");

    /* INVOICE + SYNC */
    lbl_invoice = lv_label_create(right);
    lv_label_set_text(lbl_invoice, "Invoice #1");

    lbl_sync = lv_label_create(right);
    lv_label_set_text(lbl_sync, "Offline");

    /* QTY + TOTAL */
    lbl_qty = lv_label_create(right);
    lv_label_set_text(lbl_qty, "Qty: 1");

    lbl_total = lv_label_create(right);
    lv_label_set_text(lbl_total, "Total: 0.00kg");

    /* QTY BUTTON ROW */
    lv_obj_t *qty_row = lv_obj_create(right);
    lv_obj_set_size(qty_row, LV_PCT(100), 50);
    lv_obj_set_flex_flow(qty_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(qty_row, 0, 0);

    lv_obj_t *btn_minus = lv_btn_create(qty_row);
    lv_obj_add_style(btn_minus, &g_styles.btn_danger, 0);
    lv_obj_set_size(btn_minus, 70, 40);
    lv_obj_add_event_cb(btn_minus, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_QTY_DEC);
    lv_label_set_text(lv_label_create(btn_minus), "-");

    lv_obj_t *btn_plus = lv_btn_create(qty_row);
    lv_obj_add_style(btn_plus, &g_styles.btn_secondary, 0);
    lv_obj_set_size(btn_plus, 70, 40);
    lv_obj_add_event_cb(btn_plus, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_QTY_INC);
    lv_label_set_text(lv_label_create(btn_plus), "+");

    /* MULTIPLIERS */
    lv_obj_t *multi_row = lv_obj_create(right);
    lv_obj_set_size(multi_row, LV_PCT(100), 45);
    lv_obj_set_flex_flow(multi_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(multi_row, 0, 0);

    lv_obj_t *x2 = lv_btn_create(multi_row);
    lv_obj_add_style(x2, &g_styles.btn_secondary, 0);
    lv_obj_add_event_cb(x2, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_QTY_X2);
    lv_label_set_text(lv_label_create(x2), "*2");

    lv_obj_t *x5 = lv_btn_create(multi_row);
    lv_obj_add_style(x5, &g_styles.btn_secondary, 0);
    lv_obj_add_event_cb(x5, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_QTY_X5);
    lv_label_set_text(lv_label_create(x5), "*5");

    lv_obj_t *x10 = lv_btn_create(multi_row);
    lv_obj_add_style(x10, &g_styles.btn_secondary, 0);
    lv_obj_add_event_cb(x10, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_QTY_X10);
    lv_label_set_text(lv_label_create(x10), "*10");

    /* SAVE BUTTON */
    lv_obj_t *save_btn = lv_btn_create(right);
    lv_obj_add_style(save_btn, &g_styles.btn_primary, 0);
    lv_obj_add_event_cb(save_btn, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_SAVE);
    lv_label_set_text(lv_label_create(save_btn), "SAVE");

    /* MEASURE BUTTON */
    btn_measure = lv_btn_create(right);
    lv_obj_add_style(btn_measure, &g_styles.btn_secondary, 0);
    lv_obj_add_event_cb(btn_measure, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_MEASURE);

    lbl_measure = lv_label_create(btn_measure);
    lv_label_set_text(lbl_measure, "MEASURE");

    /* HISTORY */
    lv_obj_t *hist = lv_obj_create(right);
    lv_obj_set_size(hist, LV_PCT(100), 110);

    for(int i=0;i<5;i++){
        history_lbl[i]=lv_label_create(hist);
        lv_label_set_text(history_lbl[i],"-");
        lv_obj_align(history_lbl[i],LV_ALIGN_TOP_LEFT,5,5+i*18);
    }
}

/* ================= SETTERS ================= */

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
