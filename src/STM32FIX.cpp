//
// Created by 13676 on 2020/9/26.
//
#include "STM32FIX.h"
#include "stm32f4xx_hal.h"

unsigned long millis()
{
    return HAL_GetTick();
}