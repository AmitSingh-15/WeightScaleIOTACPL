#include <lvgl.h>
#include <string.h>
#include <ctype.h>
#include "ui_styles.h"
#include "wifi_service.h"
#include "devlog.h"

/* external critical flag */
extern bool wifi_critical_section;

static lv_obj_t *popup_scr = NULL;
static lv_obj_t *ta = NULL;
static char selected_ssid[33] = {0};

static bool caps_enabled = true;

/* Button text storage to keep pointers valid */
static char key_texts[50][16] = {0};


/* ================= SAFE CLOSE ================= */

static void close_async(void *p)
{
    if(popup_scr && lv_obj_is_valid(popup_scr))
        lv_obj_del(popup_scr);

    devlog_printf("[WIFIPOP] popup closed");

    popup_scr = NULL;
}

/* ================= KEY EVENT ================= */

static void key_event(lv_event_t *e)
{
    const char *txt = (const char*)lv_event_get_user_data(e);
    if(!txt || !ta) return;

    if(strcmp(txt, "BACK") == 0)
    {
        lv_textarea_del_char(ta);
        return;
    }

    if(strcmp(txt, "ENTER") == 0)
    {
        const char *pwd = lv_textarea_get_text(ta);

        if(pwd && pwd[0])
        {
            devlog_printf("[WIFIPOP] ENTER pressed for SSID='%s' pwd_len=%u", selected_ssid, (unsigned)strlen(pwd));
            wifi_service_connect(selected_ssid, pwd);
        }

        lv_async_call(close_async, NULL);
        return;
    }

    if(strcmp(txt, "CANCEL") == 0)
    {
        devlog_printf("[WIFIPOP] Cancel pressed");
        lv_async_call(close_async, NULL);
        return;
    }

    if(strcmp(txt, "CAPS") == 0)
    {
        caps_enabled = !caps_enabled;
        return;
    }

    /* LETTER OR SYMBOL INPUT */

    char buffer[2] = {0};

    if(strlen(txt) == 1 && isalpha(txt[0]))
        buffer[0] = caps_enabled ? toupper(txt[0]) : tolower(txt[0]);
    else
        buffer[0] = txt[0];

    lv_textarea_add_text(ta, buffer);
}

/* ================= CREATE BUTTON ================= */

static void create_key(lv_obj_t *parent,
                       const char *txt,
                       int x,int y,int w,int h,
                       int key_index)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_pos(btn, x, y);
    lv_obj_add_style(btn, &g_styles.btn_secondary, 0);

    /* Store text in persistent static buffer */
    if(key_index < 50)
    {
        strncpy(key_texts[key_index], txt, sizeof(key_texts[0])-1);
        key_texts[key_index][sizeof(key_texts[0])-1] = 0;
        
        lv_obj_add_event_cb(btn, key_event,
                            LV_EVENT_RELEASED,
                            (void*)key_texts[key_index]);
    }

    lv_label_set_text(lv_label_create(btn), txt);
}

/* ================= SHOW POPUP ================= */

void wifi_password_popup_show(const char *ssid)
{
    if(!ssid) return;

    caps_enabled = true;

    strncpy(selected_ssid, ssid, sizeof(selected_ssid)-1);
    selected_ssid[sizeof(selected_ssid)-1] = 0;
    devlog_printf("[WIFIPOP] Showing password popup for SSID='%s'", selected_ssid);

    popup_scr = lv_obj_create(NULL);
    lv_obj_add_style(popup_scr, &g_styles.screen, 0);
    lv_obj_set_size(popup_scr, 800, 480);

    lv_obj_t *title = lv_label_create(popup_scr);
    lv_label_set_text(title, "Enter WiFi Password");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    ta = lv_textarea_create(popup_scr);
    lv_obj_set_width(ta, 600);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_max_length(ta, 63);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 50);

    int start_x = 20;
    int start_y = 110;
    int key_w = 55;
    int key_h = 45;
    int gap = 5;

    const char *keys[] = {
        "A","B","C","D","E","F","G","H","I","J",
        "K","L","M","N","O","P","Q","R","S","T",
        "U","V","W","X","Y","Z",
        "0","1","2","3","4","5","6","7","8","9",
        "@","!","#"
    };

    int total = sizeof(keys)/sizeof(keys[0]);
    int col = 0, row = 0;
    int key_index = 0;

    for(int i=0;i<total;i++)
    {
        int x = start_x + col*(key_w+gap);
        int y = start_y + row*(key_h+gap);

        create_key(popup_scr, keys[i], x, y, key_w, key_h, key_index++);

        col++;
        if(col >= 10)
        {
            col = 0;
            row++;
        }
    }

    int ctrl_y = start_y + 5*(key_h+gap);

    create_key(popup_scr,"CAPS",20,ctrl_y,100,50,key_index++);
    create_key(popup_scr,"BACK",130,ctrl_y,100,50,key_index++);
    create_key(popup_scr,"CANCEL",240,ctrl_y,100,50,key_index++);
    create_key(popup_scr,"ENTER",350,ctrl_y,120,50,key_index++);

    lv_scr_load(popup_scr);
}