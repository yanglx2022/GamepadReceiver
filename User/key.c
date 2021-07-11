/**********************
* KEY PA2
**********************/
#include "key.h"

// tick 1ms
static const uint16_t TIM_PRESCALER = 48000;	// 1ms(48MHz)
// ��������ʱ��2s
static const uint32_t HOLD_TICKS = 2000;	// 2s
static uint8_t  pressed      		 = 0;			// ���±�־
static uint32_t pressed_tick 		 = 0;			// ���µ�ʱ��

// ���ж�
void KEY_IRQ_Enable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                    = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// ���ж�
void KEY_IRQ_Disable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                    = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// ��ʼ��
void KEY_Init(void)
{
	// PA2 ��������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// TIM3����1ms tick
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_DeInit(TIM3);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period 				= 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler 		= TIM_PRESCALER - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM3, ENABLE);
	// �ⲿ�ж�
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

// ��ȡ����״̬
uint8_t Key_Press_Hold(void)
{
	if (pressed && TICK_TIMEOUT(pressed_tick, HOLD_TICKS))
	{
		pressed = 0;
		return 1;
	}
	return 0;
}

// �ⲿ�ж�
void EXTI2_IRQHandler(void)
{
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2))	// ������(����̧��)
	{
		pressed = 0;
	}
	else	// �½���(��������)
	{
		pressed = 1;
		pressed_tick = MILLIS_TICK;
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
}






