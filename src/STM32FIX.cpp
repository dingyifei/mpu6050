//
// Created by 13676 on 2020/9/26.
//
#include "STM32FIX.h"
#include "stm32f4xx_hal.h"

unsigned long millis()
{
    return HAL_GetTick();
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}