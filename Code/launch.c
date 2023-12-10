#include "launch.h"

void launch_init(void)
{
    nrf_rx_init();
    HAL_TIM_Base_Start_IT(&htim3);
}

void launch_tim_loop(void)
{}

void launch_main_loop(void)
{}
