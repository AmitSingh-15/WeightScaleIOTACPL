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
