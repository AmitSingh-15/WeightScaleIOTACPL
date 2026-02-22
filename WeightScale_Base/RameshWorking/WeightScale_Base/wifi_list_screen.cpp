#include "wifi_list_screen.h"
#include "wifi_service.h"
#include "ui_styles.h"
#include "devlog.h"

static lv_obj_t *list;
static void (*back_cb)(void) = NULL;
static void (*select_cb)(const char*) = NULL;

void wifi_list_screen_register_back(void (*cb)(void)) { back_cb = cb; }
void wifi_list_screen_register_select(void (*cb)(const char*)) { select_cb = cb; }

static void back_evt(lv_event_t *e)
{
    if(back_cb) back_cb();
}

static void ssid_clicked(lv_event_t *e)
{
    if(!select_cb) return;

    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    String ssid = wifi_service_get_ssid(index);

    if(ssid.length() > 0)
    {
        /* FIX: Store SSID in persistent buffer before calling callback.
           String::c_str() returns pointer to temporary that goes out of scope. */
        static char ssid_buf[33];
        strncpy(ssid_buf, ssid.c_str(), sizeof(ssid_buf) - 1);
        ssid_buf[sizeof(ssid_buf) - 1] = 0;
        
        devlog_printf("[WIFILIST] SSID clicked index=%u name=%s", (unsigned)index, ssid_buf);
        select_cb(ssid_buf);
    }
}

void wifi_list_screen_create(lv_obj_t *parent)
{
    lv_obj_add_style(parent,&g_styles.screen,0);
    lv_obj_set_size(parent,800,480);

    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_add_style(header,&g_styles.card,0);
    lv_obj_set_size(header,800,80);
    lv_obj_align(header,LV_ALIGN_TOP_MID,0,0);

    lv_label_set_text(lv_label_create(header),"SELECT WIFI");

    lv_obj_t *back = lv_btn_create(header);
    lv_obj_add_style(back,&g_styles.btn_secondary,0);
    lv_obj_align(back,LV_ALIGN_RIGHT_MID,-10,0);
    lv_label_set_text(lv_label_create(back),"BACK");
    lv_obj_add_event_cb(back,back_evt,LV_EVENT_CLICKED,NULL);

    list = lv_list_create(parent);
    lv_obj_set_size(list,760,360);
    lv_obj_align(list,LV_ALIGN_BOTTOM_MID,0,-10);
}

void wifi_list_screen_refresh(void)
{
    lv_obj_clean(list);

    uint8_t count = wifi_service_get_ap_count();
    devlog_printf("[WIFILIST] Refreshing list, %u APs", (unsigned)count);

    for(uint8_t i=0;i<count;i++)
    {
        String ssid = wifi_service_get_ssid(i);
        if(ssid.length()==0) continue;

        lv_obj_t *btn = lv_list_add_btn(list,LV_SYMBOL_WIFI,ssid.c_str());
        lv_obj_add_event_cb(btn,ssid_clicked,LV_EVENT_CLICKED,(void*)(uintptr_t)i);
    }
}