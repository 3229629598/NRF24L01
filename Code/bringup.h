#ifndef bringup_h
#define bringup_h

#include "nrf24l01.h"
#include "tim.h"
#include "usbd_cdc_if.h"

void bringup_init(void);
void bringup_tim3_loop(void);
void bringup_main_loop(void);

extern uint8_t usb_buf[TX_PLOAD_WIDTH];
extern uint8_t datacom;

#endif
