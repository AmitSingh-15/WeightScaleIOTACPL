#pragma once
#include <lvgl.h>
#include "ui_events.h"

void home_screen_create(lv_obj_t *parent);
void home_screen_set_quantity(uint16_t qty);
void home_screen_set_weight(float weight);
void home_screen_set_invoice(uint32_t invoice_id);
void home_screen_set_sync_status(const char *txt);
void home_screen_register_callback(void (*cb)(int evt));
void home_screen_update_history(void);
