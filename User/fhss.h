/**********************
* 跳频
**********************/
#ifndef __FHSS_H
#define __FHSS_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"


#define RF_CH_DEFAULT 	120	// 默认信道2520MHZ
#define FHSS_NUM				20	// 跳频频点数


void fhss_update(uint8_t* fhss);
void fhss_start(void);
void fhss_stop(void);
void fhss_set(void);


#ifdef __cplusplus
}
#endif
#endif /*__FHSS_H */


