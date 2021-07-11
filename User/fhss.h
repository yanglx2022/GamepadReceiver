/**********************
* ��Ƶ
**********************/
#ifndef __FHSS_H
#define __FHSS_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"


#define RF_CH_DEFAULT 	120	// Ĭ���ŵ�2520MHZ
#define FHSS_NUM				20	// ��ƵƵ����


void fhss_update(uint8_t* fhss);
void fhss_start(void);
void fhss_stop(void);
void fhss_set(void);


#ifdef __cplusplus
}
#endif
#endif /*__FHSS_H */


