/**********************
* KEY PA2
**********************/
#ifndef __KEY_H
#define __KEY_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"

// ms tick
#define MILLIS_TICK	(TIM3->CNT)
// 判断超时(只能判断最多一次计时溢出的情况)
#define TICK_TIMEOUT(start, timeout)	((((uint32_t)MILLIS_TICK + 0x10000 - (start)) % 0x10000) > (timeout))

void KEY_IRQ_Enable(void);
void KEY_IRQ_Disable(void);
void KEY_Init(void);
uint8_t Key_Press_Hold(void);


#ifdef __cplusplus
}
#endif
#endif /*__KEY_H */



