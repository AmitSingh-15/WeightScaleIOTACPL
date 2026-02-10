#include <lvgl.h>
#include "ui_styles.h"
#include "wifi_service.h"

static lv_obj_t *kb = NULL;
static lv_obj_t *ta = NULL;
static lv_obj_t *popup_scr = NULL;

static char selected_ssid[33] = {0};

/* =========================================================
   ASYNC SAFE CLOSE
=========================================================*/

static void popup_close_async(void *param)
{
    lv_obj_t *scr = (lv_obj_t *)param;

    if(scr && lv_obj_is_valid(scr))
    {
        lv_obj_del(scr);
    }

    popup_scr = NULL;
}

/* =========================================================
   KEYBOARD EVENT
=========================================================*/

static void kb_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_READY)
    {
        const char *pwd = lv_textarea_get_text(ta);

        Serial.printf("[WIFI] Connecting to %s\n", selected_ssid);

        wifi_service_connect(selected_ssid, pwd);

        /* 🔥 DEFER UI DELETION */
        lv_async_call(popup_close_async, popup_scr);
    }
    else if(code == LV_EVENT_CANCEL)
    {
        lv_async_call(popup_close_async, popup_scr);
    }
}

/* =========================================================
   SHOW PASSWORD POPUP
=========================================================*/

void wifi_password_popup_show(const char *ssid)
{
    if(!ssid) return;

    strncpy(selected_ssid, ssid, sizeof(selected_ssid));
    selected_ssid[sizeof(selected_ssid)-1] = 0;

    popup_scr = lv_obj_create(NULL);
    lv_obj_add_style(popup_scr, &g_styles.screen, 0);

    /* TITLE */
    lv_obj_t *title = lv_label_create(popup_scr);

    static char buf[64];
    snprintf(buf, sizeof(buf), "Password for %s", selected_ssid);

    lv_label_set_text(title, buf);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    /* TEXTAREA */
    ta = lv_textarea_create(popup_scr);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_width(ta, 520);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 80);

    /* KEYBOARD */
    kb = lv_keyboard_create(popup_scr);
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_add_event_cb(kb, kb_event, LV_EVENT_ALL, NULL);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_scr_load(popup_scr);
}
