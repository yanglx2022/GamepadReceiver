/**********************
* ��Ƶ
**********************/
#include "fhss.h"
#include "delay.h"
#include "stdlib.h"
#include "key.h"
#include "nrf24l01.h"

// ��Ƶ��ʱ����22ms
#define TIM_PERIOD_T2					 22000	// 22ms
#define TIM_PRESCALER_T2			 48			// 1us��Ƶ(48MHz��Ƶ)

static uint8_t fhss_list[FHSS_NUM];	// ��Ƶͼ��
static uint8_t fhss_idx = 0;				// Ƶ������
static uint8_t fhss_on = 0;					// ��Ƶ��־
static uint32_t last_rx_time = 0;		// �ϴν���ʱ��

static void tim2_init(void);

// ������Ƶͼ��
void fhss_update(uint8_t* fhss)
{
	if (fhss)
	{
		for(int i = 0; i < FHSS_NUM; i++)
		{
			fhss_list[i] = fhss[i];
		}
	}
}

// ������Ƶ
void fhss_start(void)
{
	// ��ʱ����ʼ��
	static uint8_t initialized = 0;
	if (initialized == 0)
	{
		initialized = 1;
		tim2_init();
	}
	// ������ʱ��
	TIM_Cmd(TIM2, ENABLE);
	fhss_on = 0;
	last_rx_time = MILLIS_TICK;
}

// ֹͣ��Ƶ
void fhss_stop(void)
{
	// �رն�ʱ��
	TIM_Cmd(TIM2, DISABLE);
}

// ������Ƶ
void fhss_set(void)
{
	TIM_SetCounter(TIM2, TIM_PERIOD_T2 / 2);	// ���趨ʱ������ֵ
	fhss_on = 1;
	last_rx_time = MILLIS_TICK;
}

// ��ʱ����ʼ��
static void tim2_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_DeInit(TIM2);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period 				= TIM_PERIOD_T2 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler 		= TIM_PRESCALER_T2 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM2, DISABLE);	// �ر�Ԥװ�ػ���,��֤�޸�Ԥװ��ֵʱ������Ч
	// �ж�
	TIM_ClearFlag(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                    = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// ��Ƶ
static void hopping(void)
{
	fhss_idx++;
	if (fhss_idx >= FHSS_NUM)
	{
		fhss_idx = 0;
	}
	NRF24L01_RF_CH(fhss_list[fhss_idx]);
}

// TIM2�ж�
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		if (fhss_on)
		{
			// ��Ƶ
			hopping();
		}
		if (TICK_TIMEOUT(last_rx_time, TIM_PERIOD_T2 / 1000 * (FHSS_NUM + 1)))	// ��ʱ
		{
			fhss_on = 0;
			last_rx_time = MILLIS_TICK;
			// ��Ƶ(������һ���޷�ͨ�ŵ�Ƶ�������޵ȴ�)
			hopping();
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}




