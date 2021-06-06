/**********************
*系统tick延时
**********************/
#include "delay.h"

// 初始化
void Delay_Init(void)
{
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;			// 使用内核时钟
									//SysTick_CTRL_TICKINT_Msk   |	// 开启中断
                  //SysTick_CTRL_ENABLE_Msk;			// 启动
}

// us延时(24位计数,最大延时2^24个时钟周期,72MHz时约233ms)
void delay_us(uint16_t num)
{
	SysTick->LOAD  = SystemCoreClock / 1000000 * num;	// 计数值
	SysTick->VAL 	 = 0x00;		// 清空计数器
	SysTick->CTRL |= 1 << 0;	// 开始计数
	uint32_t ctrl = 0;
	do
	{
		ctrl = SysTick->CTRL;
	}while((ctrl & 0x01) && !(ctrl & (1 << 16)));			// 等待时间到
	SysTick->CTRL &= ~(1 << 0);		// 关闭SYSTICK
	SysTick->VAL   = 0x00;				// 清空计数器
}

// ms延时(24位计数,最大延时2^24个时钟周期,72MHz时约233ms)
static void _delay_ms(uint16_t num)
{
	SysTick->LOAD  = SystemCoreClock / 1000 * num;	// 计数值
	SysTick->VAL 	 = 0x00;		// 清空计数器
	SysTick->CTRL |= 1 << 0;	// 开始计数
	uint32_t ctrl = 0;
	do
	{
		ctrl = SysTick->CTRL;
	}while((ctrl & 0x01) && !(ctrl & (1 << 16)));			// 等待时间到
	SysTick->CTRL &= ~(1 << 0);		// 关闭SYSTICK
	SysTick->VAL   = 0x00;				// 清空计数器
}

// ms延时
void delay_ms(uint32_t num)
{
	uint32_t repeat = num / 200;
	uint32_t remain = num % 200;
	while(repeat--)
	{
		_delay_ms(200);
	}
	if (remain)
	{
		_delay_ms(remain);
	}
}

