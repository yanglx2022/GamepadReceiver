/**********************
*系统tick延时
**********************/
#include "delay.h"


// us延时(24位计数,最大延时2^24个时钟周期,6MHz时约2796ms)
void delay_us(uint16_t num)
{
	SysTick->CTRL  = 0UL;					// 使用外部时钟(HCLK/8=6MHz) 不开中断
	SysTick->LOAD  = 6 * num - 1;	// 计数值
	SysTick->VAL 	 = 0UL;					// 清空计数器
	SysTick->CTRL |= 1UL;					// 开始计数
	uint32_t ctrl  = 0;
	do
	{
		ctrl = SysTick->CTRL;
	}while((ctrl & 1UL) && !(ctrl & (1UL << 16)));	// 等待时间到
	SysTick->CTRL &= ~1UL;			// 关闭SYSTICK
	SysTick->VAL   = 0UL;				// 清空计数器
}

// ms延时(24位计数,最大延时2^24个时钟周期,6MHz时约2796ms)
static void _delay_ms(uint16_t num)
{
	SysTick->CTRL  = 0UL;						// 使用外部时钟(HCLK/8=6MHz) 不开中断
	SysTick->LOAD  = 6000 * num - 1;// 计数值
	SysTick->VAL 	 = 0UL;						// 清空计数器
	SysTick->CTRL |= 1UL;						// 开始计数
	uint32_t ctrl  = 0;
	do
	{
		ctrl = SysTick->CTRL;
	}while((ctrl & 1UL) && !(ctrl & (1UL << 16)));	// 等待时间到
	SysTick->CTRL &= ~1UL;			// 关闭SYSTICK
	SysTick->VAL   = 0UL;				// 清空计数器
}

// ms延时
void delay_ms(uint32_t num)
{
	uint32_t repeat = num / 2000;
	uint32_t remain = num % 2000;
	while(repeat--)
	{
		_delay_ms(2000);
	}
	if (remain)
	{
		_delay_ms(remain);
	}
}

// 闹钟(用于中断唤醒睡眠)最大2796ms
void delay_alarm_ms(uint16_t num)
{
	if (num > 2796)
	{
		num = 2796;
	}
	// 使用外部时钟(HCLK/8=6MHz)
	SysTick->CTRL  = SysTick_CTRL_TICKINT_Msk;	// 开启中断
	SysTick->LOAD  = 6000 * num - 1;// 计数值
	SysTick->VAL 	 = 0;							// 清空计数器
	SysTick->CTRL |= 1;							// 开始计数
}

void SysTick_Handler(void)
{
	SysTick->CTRL &= ~1UL;	// 关闭SYSTICK
	SysTick->VAL   = 0UL;		// 清空计数器
}



