/**********************
* LED PA0 PA1
**********************/
#include "led.h"


// 初始化
void LED_Init(void)
{
	// PA0 PA1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	LED_RED(0);
	LED_BLUE(0);
}

// 红LED
void LED_RED(uint8_t state)
{
	if (state)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_0);
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_0);
	}
}

// 蓝LED
void LED_BLUE(uint8_t state)
{
	if (state)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
	}
}

// 所有LED
void LED_ALL(uint8_t state)
{
	LED_RED(state);
	LED_BLUE(state);
}




