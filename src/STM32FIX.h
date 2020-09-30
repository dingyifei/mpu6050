//
// Created by 13676 on 2020/9/26.
//

#ifndef STM32FIX_H

#define STM32FIX_H
#include "stm32f4xx_hal.h"
#include <i2c.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

#define delay HAL_Delay
#define PI 3.1415926
long map(long x, long in_min, long in_max, long out_min, long out_max);

#endif //STM32FIX_H
