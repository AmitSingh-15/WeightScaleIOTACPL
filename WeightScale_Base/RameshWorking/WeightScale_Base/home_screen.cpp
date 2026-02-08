#include "home_screen.h"
#include "ui_styles.h"
#include "ui_events.h"
#include "invoice_service.h"
#include "storage_service.h"
#include <stdio.h>
#include <stdlib.h>

/* ================= OBJECTS ================= */

static lv_obj_t *lbl_weight;
static lv_obj_t *lbl_qty_main;
static lv_obj_t *lbl_total;
static lv_obj_t *lbl_invoice;
static lv_obj_t *history_lbl[10];
static lv_obj_t *lbl_sync;

static void (*event_cb)(int evt) = NULL;

/* ================= EVENTS ================= */

static void btn_event_cb(lv_event_t *e)
{
    if(!event_cb) return;
    uintptr_t id = (uintptr_t)lv_event_get_user_data(e);
    event_cb((int)id);
}

void home_screen_register_callback(void (*cb)(int evt))
{
    event_cb = cb;
}

/* =========================================================
   UI CREATE
=========================================================*/

void home_screen_create(lv_obj_t *parent)
{
    ui_styles_init();

    lv_obj_add_style(parent,&g_styles.screen,0);
    lv_obj_set_size(parent,800,480);

    /* =====================================================
       TOP HEADER 20%
    =====================================================*/
    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_add_style(header,&g_styles.card,0);
    lv_obj_set_size(header,800,96);     // 20% of 480
    lv_obj_align(header,LV_ALIGN_TOP_MID,0,0);

    /* brand left */
    lv_obj_t *brand = lv_label_create(header);
    lv_label_set_text(brand,"ACPL delivering progress");
    lv_obj_align(brand,LV_ALIGN_LEFT_MID,10,-10);

    lv_obj_t *power = lv_label_create(header);
    lv_label_set_text(power,"powered by Vastotech");
    lv_obj_align(power,LV_ALIGN_LEFT_MID,10,12);

    /* BIG CENTER INVOICE */
    lbl_invoice = lv_label_create(header);
    lv_obj_set_style_text_font(lbl_invoice,&lv_font_montserrat_20,0);
    lv_label_set_text(lbl_invoice,"Invoice #1");
    lv_obj_align(lbl_invoice,LV_ALIGN_CENTER,0,0);

    /* SETTINGS BUTTON RIGHT */
    lv_obj_t *settings_btn = lv_btn_create(header);
    lv_obj_add_style(settings_btn,&g_styles.btn_secondary,0);
    lv_obj_align(settings_btn,LV_ALIGN_RIGHT_MID,-15,0);
    lv_obj_add_event_cb(settings_btn,btn_event_cb,LV_EVENT_CLICKED,(void*)UI_EVT_SETTINGS);
    lv_label_set_text(lv_label_create(settings_btn),"SET");

    /* =====================================================
       MIDDLE AREA 60%
    =====================================================*/

    lv_obj_t *middle = lv_obj_create(parent);
    lv_obj_remove_style_all(middle);
    lv_obj_set_size(middle,800,288);  // 60%
    lv_obj_align(middle,LV_ALIGN_TOP_MID,0,96);

    /* ---------- LEFT WEIGHT (60% width) ---------- */

    lv_obj_t *left = lv_obj_create(middle);
    lv_obj_add_style(left,&g_styles.card,0);
    lv_obj_set_size(left,480,288); // 60%
    lv_obj_align(left,LV_ALIGN_LEFT_MID,0,0);

    lbl_weight = lv_label_create(left);
    lv_label_set_text(lbl_weight,"0.00 kg");
    lv_obj_set_style_text_font(lbl_weight,&lv_font_montserrat_48,0);
    lv_obj_align(lbl_weight,LV_ALIGN_TOP_MID,0,20);

    lv_obj_t *info_row = lv_obj_create(left);
    lv_obj_remove_style_all(info_row);
    lv_obj_set_width(info_row,LV_PCT(100));
    lv_obj_align(info_row,LV_ALIGN_BOTTOM_MID,0,-10);
    lv_obj_set_flex_flow(info_row,LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(info_row,20,0);

    lbl_qty_main = lv_label_create(info_row);
    lv_label_set_text(lbl_qty_main,"Qty: 1");

    lbl_total = lv_label_create(info_row);
    lv_label_set_text(lbl_total,"Total: 0.00 kg");

    /* ---------- RIGHT HISTORY (40% width) ---------- */

    lv_obj_t *hist = lv_obj_create(middle);
    lv_obj_add_style(hist,&g_styles.card,0);
    lv_obj_set_size(hist,320,288); // 40%
    lv_obj_align(hist,LV_ALIGN_RIGHT_MID,0,0);

    lv_label_set_text(lv_label_create(hist),"Last 10");

    for(int i=0;i<10;i++)
    {
        history_lbl[i] = lv_label_create(hist);
        lv_label_set_text(history_lbl[i],"-");
        lv_obj_align(history_lbl[i],LV_ALIGN_TOP_LEFT,10,30 + (i*22));
    }

    /* =====================================================
       BOTTOM ACTION BAR 20%
    =====================================================*/

    lv_obj_t *action = lv_obj_create(parent);
    lv_obj_add_style(action,&g_styles.card,0);
    lv_obj_set_size(action,800,96);   // 20%
    lv_obj_align(action,LV_ALIGN_BOTTOM_MID,0,0);

    lv_obj_set_flex_flow(action,LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(action,15,0);
    lv_obj_set_style_pad_all(action,10,0);

    const struct {
        const char* txt;
        int evt;
    } btns[] = {
        {"-1", UI_EVT_QTY_DEC},
        {"+1", UI_EVT_QTY_INC},
        {"+2", UI_EVT_QTY_X2},
        {"+5", UI_EVT_QTY_X5},
        {"+10",UI_EVT_QTY_X10},
        {"SAVE",UI_EVT_SAVE},
        {"RESET",UI_EVT_RESET}
    };

    for(int i=0;i<7;i++)
    {
        lv_obj_t *b = lv_btn_create(action);

        /* 🔥 wider industrial buttons */
        lv_obj_set_size(b,100,70);

        if(i>=5)
            lv_obj_add_style(b,&g_styles.btn_primary,0);
        else
            lv_obj_add_style(b,&g_styles.btn_secondary,0);

        lv_obj_add_event_cb(b,btn_event_cb,LV_EVENT_CLICKED,(void*)btns[i].evt);

        lv_label_set_text(lv_label_create(b),btns[i].txt);
    }
}

/* =====================================================
   SETTERS
=====================================================*/

void home_screen_set_weight(float w)
{
    static char buf[24];
    snprintf(buf,sizeof(buf),"%.2f kg",w);
    lv_label_set_text(lbl_weight,buf);
}

void home_screen_set_quantity(uint16_t qty)
{
    static char buf[16];
    snprintf(buf,sizeof(buf),"Qty: %d",qty);
    lv_label_set_text(lbl_qty_main,buf);
}

void home_screen_set_total(float total)
{
    static char buf[24];
    snprintf(buf,sizeof(buf),"Total: %.2f kg",total);
    lv_label_set_text(lbl_total,buf);
}

void home_screen_set_invoice(uint32_t id)
{
    static char buf[24];
    snprintf(buf,sizeof(buf),"Invoice #%lu",id);
    lv_label_set_text(lbl_invoice,buf);
}

void home_screen_set_sync_status(const char *txt)
{
    if(lbl_sync) lv_label_set_text(lbl_sync,txt);
}

void home_screen_update_history(void)
{
    invoice_record_t recs[10];
    uint8_t count = storage_get_last_records(recs,10);

    for(int i=0;i<10;i++)
    {
        if(i<count)
        {
            static char buf[48];
            snprintf(buf,sizeof(buf),
                     "#%lu  %.2fkg  x%d",
                     recs[i].invoice_id,
                     recs[i].weight,
                     recs[i].quantity);

            lv_label_set_text(history_lbl[i],buf);
        }
        else
        {
            lv_label_set_text(history_lbl[i],"-");
        }
    }
}
