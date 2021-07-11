/**********************
* 跳频
**********************/
#include "fhss.h"
#include "delay.h"
#include "stdlib.h"
#include "key.h"
#include "nrf24l01.h"

// 跳频定时周期22ms
#define TIM_PERIOD_T2					 22000	// 22ms
#define TIM_PRESCALER_T2			 48			// 1us分频(48MHz主频)

static uint8_t fhss_list[FHSS_NUM];	// 跳频图案
static uint8_t fhss_idx = 0;				// 频道索引
static uint8_t fhss_on = 0;					// 调频标志
static uint32_t last_rx_time = 0;		// 上次接收时间

static void tim2_init(void);

// 更新跳频图案
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

// 启动跳频
void fhss_start(void)
{
	// 定时器初始化
	static uint8_t initialized = 0;
	if (initialized == 0)
	{
		initialized = 1;
		tim2_init();
	}
	// 开启定时器
	TIM_Cmd(TIM2, ENABLE);
	fhss_on = 0;
	last_rx_time = MILLIS_TICK;
}

// 停止跳频
void fhss_stop(void)
{
	// 关闭定时器
	TIM_Cmd(TIM2, DISABLE);
}

// 设置跳频
void fhss_set(void)
{
	TIM_SetCounter(TIM2, TIM_PERIOD_T2 / 2);	// 重设定时器计数值
	fhss_on = 1;
	last_rx_time = MILLIS_TICK;
}

// 定时器初始化
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
	TIM_ARRPreloadConfig(TIM2, DISABLE);	// 关闭预装载缓冲,保证修改预装载值时立即生效
	// 中断
	TIM_ClearFlag(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                    = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// 跳频
static void hopping(void)
{
	fhss_idx++;
	if (fhss_idx >= FHSS_NUM)
	{
		fhss_idx = 0;
	}
	NRF24L01_RF_CH(fhss_list[fhss_idx]);
}

// TIM2中断
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		if (fhss_on)
		{
			// 跳频
			hopping();
		}
		if (TICK_TIMEOUT(last_rx_time, TIM_PERIOD_T2 / 1000 * (FHSS_NUM + 1)))	// 超时
		{
			fhss_on = 0;
			last_rx_time = MILLIS_TICK;
			// 跳频(避免在一个无法通信的频道上无限等待)
			hopping();
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}




