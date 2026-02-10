#include "settings_screen.h"
#include "ui_styles.h"
#include "wifi_service.h"
#include "ota_service.h"
#include "wifi_list_screen.h"

extern void wifi_password_popup_show(const char *ssid);

/* =========================================================
   GLOBALS
=========================================================*/

static lv_obj_t *wifi_status;
static void (*back_cb)(void) = NULL;
static void (*calibration_cb)(void) = NULL;

/* 🔥 KEEP REFERENCE TO SETTINGS SCREEN */
static lv_obj_t *settings_scr_ref = NULL;

/* WiFi list screen */
static lv_obj_t *wifi_list_scr = NULL;

/* =========================================================
   INTERNAL EVENTS
=========================================================*/

static void back_cb_wrapper(lv_event_t *e)
{
    if (back_cb) back_cb();
}

static void ota_cb(lv_event_t *e)
{
    ota_service_check_and_update();
}

static void wifi_selected(const char *ssid)
{
    wifi_password_popup_show(ssid);
}

/* 🔥 FIXED BACK — LOAD SETTINGS SCREEN */
static void wifi_list_back(void)
{
    if(settings_scr_ref)
        lv_scr_load(settings_scr_ref);
}

/* =========================================================
   SCAN WIFI
=========================================================*/

static void scan_cb(lv_event_t *e)
{
    Serial.println("[WIFI] Scan requested");

    /* 🔥 blocking scan now */
    wifi_service_start_scan();

    if (wifi_list_scr)
    {
        wifi_list_screen_refresh();
        lv_scr_load(wifi_list_scr);
    }
}

/* DEBUG CONNECT */
static void connect_cb(lv_event_t *e)
{
    wifi_service_connect("YourSSID", "YourPassword");
}

/* =========================================================
   REGISTRATION
=========================================================*/

void settings_screen_register_back_callback(void (*cb)(void))
{
    back_cb = cb;
}

void settings_screen_register_calibration_callback(void (*cb)(void))
{
    calibration_cb = cb;
}

/* =========================================================
   CREATE SCREEN
=========================================================*/

void settings_screen_create(lv_obj_t *parent)
{
    ui_styles_init();

    settings_scr_ref = parent;   // 🔥 CRITICAL

    lv_obj_t *scr = parent;
    lv_obj_add_style(scr, &g_styles.screen, 0);
    lv_obj_set_size(scr, 800, 480);

    /* CARD */

    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_add_style(card, &g_styles.card, 0);
    lv_obj_set_size(card, 720, 400);
    lv_obj_center(card);

    /* BACK BUTTON */

    lv_obj_t *back_btn = lv_btn_create(card);
    lv_obj_add_style(back_btn, &g_styles.btn_secondary, 0);
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -20, 10);
    lv_obj_add_event_cb(back_btn, back_cb_wrapper, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(back_btn), "Back");

    /* TITLE */

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "SETTINGS");
    lv_obj_add_style(title, &g_styles.title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    /* WIFI STATUS */

    wifi_status = lv_label_create(card);
    lv_label_set_text(wifi_status, "Wi-Fi: Offline");
    lv_obj_align(wifi_status, LV_ALIGN_TOP_LEFT, 20, 60);

    /* SCAN BUTTON */

    lv_obj_t *scan_btn = lv_btn_create(card);
    lv_obj_add_style(scan_btn, &g_styles.btn_secondary, 0);
    lv_obj_align(scan_btn, LV_ALIGN_TOP_LEFT, 20, 100);
    lv_obj_add_event_cb(scan_btn, scan_cb, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(scan_btn), "Scan Wi-Fi");

    /* CONNECT BUTTON */

    lv_obj_t *connect_btn = lv_btn_create(card);
    lv_obj_add_style(connect_btn, &g_styles.btn_primary, 0);
    lv_obj_align(connect_btn, LV_ALIGN_TOP_LEFT, 200, 100);
    lv_obj_add_event_cb(connect_btn, connect_cb, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(connect_btn), "Connect");

    /* CALIBRATION */

    lv_obj_t *cal_btn = lv_btn_create(card);
    lv_obj_add_style(cal_btn, &g_styles.btn_primary, 0);
    lv_obj_align(cal_btn, LV_ALIGN_TOP_LEFT, 20, 220);
    lv_label_set_text(lv_label_create(cal_btn), "Calibration");

    lv_obj_add_event_cb(cal_btn, [](lv_event_t *e){
        if (calibration_cb) calibration_cb();
    }, LV_EVENT_CLICKED, NULL);

    /* OTA */

    lv_obj_t *ota_btn = lv_btn_create(card);
    lv_obj_add_style(ota_btn, &g_styles.btn_primary, 0);
    lv_obj_align(ota_btn, LV_ALIGN_TOP_LEFT, 20, 160);
    lv_obj_add_event_cb(ota_btn, ota_cb, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(ota_btn), "OTA Update");

    /* WIFI LIST SCREEN */

    wifi_list_scr = lv_obj_create(NULL);

    wifi_list_screen_create(wifi_list_scr);
    wifi_list_screen_register_back(wifi_list_back);
    wifi_list_screen_register_select(wifi_selected);
}

/* =========================================================
   WIFI STATUS UPDATE
=========================================================*/

void settings_screen_update_wifi_status(void)
{
    if (!wifi_status) return;

    wifi_state_t state = wifi_service_state();

    const char *status = "Wi-Fi: Offline";

    if (state == WIFI_CONNECTING)
        status = "Wi-Fi: Connecting...";
    else if (state == WIFI_CONNECTED)
        status = "Wi-Fi: Connected";

    lv_label_set_text(wifi_status, status);
}
