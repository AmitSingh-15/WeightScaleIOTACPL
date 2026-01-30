#pragma once
#include <Arduino.h>
#include "invoice_service.h"   // needed for invoice_record_t

void storage_service_init(void);

/* Invoice counter */
void storage_save_invoice(uint32_t id);
void storage_load_invoice(uint32_t *id);

/* Day tracking */
void storage_save_last_day(uint32_t day);
uint32_t storage_load_last_day(void);

/* Offline queue */
bool storage_enqueue_record(const invoice_record_t *rec);

void storage_save_offset(float val);
float storage_load_offset(void);

void storage_add_full_record(const invoice_record_t *rec);
uint32_t storage_get_record_count(void);
uint8_t storage_get_last_records(invoice_record_t *out, uint8_t max);
void storage_check_new_day_and_reset(void);
void storage_clear_all_records(void);