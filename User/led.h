/**********************
* LED PA0 PA1
**********************/
#ifndef __LED_H
#define __LED_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"

void LED_Init(void);
void LED_RED(uint8_t state);
void LED_BLUE(uint8_t state);
void LED_ALL(uint8_t state);


#ifdef __cplusplus
}
#endif
#endif /*__LED_H */


