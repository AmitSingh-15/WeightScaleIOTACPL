#include "settings_screen.h"
#include "ui_styles.h"
#include "network/wifi_service.h"
#include "ota/ota_service.h"


static lv_obj_t *wifi_status;
static void (*back_cb)(void) = NULL;

static void back_cb_wrapper(lv_event_t *e)
{
    if (back_cb) back_cb();
}

static void ota_cb(lv_event_t *e)
{
    ota_service_check_and_update();
}

static void scan_cb(lv_event_t *e)
{
    wifi_service_start_scan();
}

static void connect_cb(lv_event_t *e)
{
    wifi_service_connect("YourSSID", "YourPassword");
}

void settings_screen_register_back_callback(void (*cb)(void))
{
    back_cb = cb;
}

void settings_screen_create(lv_obj_t *parent)
{
    lv_obj_t *scr = lv_obj_create(parent);
    lv_obj_add_style(scr, &g_styles.screen, 0);
    lv_obj_set_size(scr, 800, 480);

    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_add_style(card, &g_styles.card, 0);
    lv_obj_set_size(card, 720, 400);
    lv_obj_center(card);

    lv_obj_t *back_btn = lv_btn_create(card);
    lv_obj_add_style(back_btn, &g_styles.btn_secondary, 0);
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -20, 10);
    lv_obj_add_event_cb(back_btn, back_cb_wrapper, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(back_btn), "Back");

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "SETTINGS");
    lv_obj_add_style(title, &g_styles.title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    wifi_status = lv_label_create(card);
    lv_label_set_text(wifi_status, "Wi-Fi: Offline");
    lv_obj_align(wifi_status, LV_ALIGN_TOP_LEFT, 20, 60);

    lv_obj_t *scan_btn = lv_btn_create(card);
    lv_obj_add_style(scan_btn, &g_styles.btn_secondary, 0);
    lv_obj_align(scan_btn, LV_ALIGN_TOP_LEFT, 20, 100);
    lv_obj_add_event_cb(scan_btn, scan_cb, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(scan_btn), "Scan Wi-Fi");

    lv_obj_t *connect_btn = lv_btn_create(card);
    lv_obj_add_style(connect_btn, &g_styles.btn_primary, 0);
    lv_obj_align(connect_btn, LV_ALIGN_TOP_LEFT, 200, 100);
    lv_obj_add_event_cb(connect_btn, connect_cb, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(connect_btn), "Connect");

    lv_obj_t *ota_btn = lv_btn_create(card);
    lv_obj_add_style(ota_btn, &g_styles.btn_primary, 0);
    lv_obj_align(ota_btn, LV_ALIGN_TOP_LEFT, 20, 160);
    lv_obj_add_event_cb(ota_btn, ota_cb, LV_EVENT_CLICKED, NULL);
}
void settings_screen_update_wifi_status(void)
{
    if (!wifi_status) return;

    wifi_state_t state = wifi_service_state();
    const char *status = "Wi-Fi: Offline";
    if (state == WIFI_CONNECTING) status = "Wi-Fi: Connecting...";
    else if (state == WIFI_CONNECTED) status = "Wi-Fi: Connected";

    lv_label_set_text(wifi_status, status);
}
