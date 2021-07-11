/**********************
* KEY PA2
**********************/
#include "key.h"

// tick 1ms
static const uint16_t TIM_PRESCALER = 48000;	// 1ms(48MHz)
// 按键长按时间2s
static const uint32_t HOLD_TICKS = 2000;	// 2s
static uint8_t  pressed      		 = 0;			// 按下标志
static uint32_t pressed_tick 		 = 0;			// 按下的时刻

// 开中断
void KEY_IRQ_Enable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                    = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// 关中断
void KEY_IRQ_Disable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                    = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// 初始化
void KEY_Init(void)
{
	// PA2 上拉输入
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// TIM3产生1ms tick
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_DeInit(TIM3);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period 				= 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler 		= TIM_PRESCALER - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM3, ENABLE);
	// 外部中断
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line2;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearFlag(EXTI_Line2);
	KEY_IRQ_Enable();
}

// 获取长按状态
uint8_t Key_Press_Hold(void)
{
	if (pressed && TICK_TIMEOUT(pressed_tick, HOLD_TICKS))
	{
		pressed = 0;
		return 1;
	}
	return 0;
}

// 外部中断
void EXTI2_IRQHandler(void)
{
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2))	// 上升沿(按键抬起)
	{
		pressed = 0;
	}
	else	// 下降沿(按键按下)
	{
		pressed = 1;
		pressed_tick = MILLIS_TICK;
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
}






