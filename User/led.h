/**********************
* LED PA0 PA1
**********************/
#ifndef __LED__
#define __LED__

#include "stm32f10x.h"

void LED_Init(void);
void LED_RED(uint8_t state);
void LED_BLUE(uint8_t state);


#endif

