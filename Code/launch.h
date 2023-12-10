#ifndef launch_h
#define launch_h

#include "nrf24l01.h"
#include "tim.h"

void launch_init(void);
void launch_tim_loop(void);
void launch_main_loop(void);

#endif
