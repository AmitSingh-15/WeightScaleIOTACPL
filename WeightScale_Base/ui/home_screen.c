#include "home_screen.h"
#include "ui_styles.h"

static lv_obj_t *lbl_weight;
static lv_obj_t *lbl_qty;
static lv_obj_t *lbl_invoice;
static lv_obj_t *lbl_sync;
static void (*event_cb)(int evt) = NULL;

static void btn_event_cb(lv_event_t *e)
{
    if (!event_cb) return;
    int id = (int)lv_event_get_user_data(e);
    event_cb(id);
}

void home_screen_register_callback(void (*cb)(int evt))
{
    event_cb = cb;
}

void home_screen_create(lv_obj_t *parent)
{
    ui_styles_init();

    lv_obj_t *screen = lv_obj_create(parent);
    lv_obj_add_style(screen, &g_styles.screen, 0);
    lv_obj_set_size(screen, 800, 480);

    static lv_coord_t col[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t row[] = { 80, 160, 120, 80, LV_GRID_TEMPLATE_LAST };

    lv_obj_set_grid_dsc_array(screen, col, row);

    /* Header */
    lv_obj_t *header = lv_obj_create(screen);
    lv_obj_add_style(header, &g_styles.card, 0);
    lv_obj_set_grid_cell(header, LV_GRID_ALIGN_STRETCH, 0, 3,
                                   LV_GRID_ALIGN_STRETCH, 0, 1);

    lbl_invoice = lv_label_create(header);
    lv_label_set_text(lbl_invoice, "Invoice #1");
    lv_obj_add_style(lbl_invoice, &g_styles.title, 0);
    lv_obj_align(lbl_invoice, LV_ALIGN_LEFT_MID, 0, 0);

    lbl_sync = lv_label_create(header);
    lv_label_set_text(lbl_sync, "Offline");
    lv_obj_add_style(lbl_sync, &g_styles.value, 0);
    lv_obj_align(lbl_sync, LV_ALIGN_RIGHT_MID, -60, 0);

    lv_obj_t *settings_btn = lv_btn_create(header);
    lv_obj_add_style(settings_btn, &g_styles.btn_secondary, 0);
    lv_obj_set_size(settings_btn, 40, 30);
    lv_obj_align(settings_btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_add_event_cb(settings_btn, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_SETTINGS);
    lv_label_set_text(lv_label_create(settings_btn), "⚙");

    /* Weight Card */
    lv_obj_t *weight_card = lv_obj_create(screen);
    lv_obj_add_style(weight_card, &g_styles.card, 0);
    lv_obj_set_grid_cell(weight_card, LV_GRID_ALIGN_STRETCH, 0, 3,
                                      LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *wt_title = lv_label_create(weight_card);
    lv_label_set_text(wt_title, "WEIGHT (kg)");
    lv_obj_add_style(wt_title, &g_styles.title, 0);
    lv_obj_align(wt_title, LV_ALIGN_TOP_MID, 0, 0);

    lbl_weight = lv_label_create(weight_card);
    lv_label_set_text(lbl_weight, "0.00");
    lv_obj_add_style(lbl_weight, &g_styles.value_big, 0);
    lv_obj_align(lbl_weight, LV_ALIGN_CENTER, 0, 20);

    /* Quantity */
    lv_obj_t *qty_card = lv_obj_create(screen);
    lv_obj_add_style(qty_card, &g_styles.card, 0);
    lv_obj_set_grid_cell(qty_card, LV_GRID_ALIGN_STRETCH, 0, 1,
                                    LV_GRID_ALIGN_STRETCH, 2, 1);

    lv_label_create(qty_card);
    lbl_qty = lv_label_create(qty_card);
    lv_label_set_text(lbl_qty, "Qty: 1");
    lv_obj_add_style(lbl_qty, &g_styles.value, 0);
    lv_obj_center(lbl_qty);

    lv_obj_t *btn_plus = lv_btn_create(qty_card);
    lv_obj_add_style(btn_plus, &g_styles.btn_secondary, 0);
    lv_obj_align(btn_plus, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_add_event_cb(btn_plus, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_QTY_INC);

    lv_obj_t *btn_minus = lv_btn_create(qty_card);
    lv_obj_add_style(btn_minus, &g_styles.btn_danger, 0);
    lv_obj_align(btn_minus, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_event_cb(btn_minus, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_QTY_DEC);

    /* Save */
    lv_obj_t *save_btn = lv_btn_create(screen);
    lv_obj_add_style(save_btn, &g_styles.btn_primary, 0);
    lv_obj_set_grid_cell(save_btn, LV_GRID_ALIGN_CENTER, 1, 1,
                                      LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_add_event_cb(save_btn, btn_event_cb, LV_EVENT_CLICKED, (void*)UI_EVT_SAVE);

    lv_label_set_text(lv_label_create(save_btn), "SAVE");
}

void home_screen_set_weight(float weight)
{
    static char buf[16];
    lv_snprintf(buf, sizeof(buf), "%.2f", weight);
    lv_label_set_text(lbl_weight, buf);
}

void home_screen_set_quantity(uint16_t qty)
{
    static char buf[16];
    lv_snprintf(buf, sizeof(buf), "Qty: %d", qty);
    lv_label_set_text(lbl_qty, buf);
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
