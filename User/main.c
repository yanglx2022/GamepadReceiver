// 2.4G转USB Gamepad
#include "usb_hw.h"
#include "delay.h"
#include "led.h"
#include "nrf24l01.h"

#define IAP		// 支持IAP升级, 此时需要设置IROM1起始地址

// 遥控标志
extern uint8_t gamepad_flag;

int main(void)
{
	// IAP升级设置
	#ifdef IAP
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4400); // 设置中断向量表
	__enable_irq();	// 开总中断
	#endif
	// 设置中断优先级组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init();
	LED_Init();
	// USB初始化
  USB_HW_Init();
  USB_Init();
	// 2.4G无线模块初始化(失败尝试3次)
	uint8_t init_ok = 0;
	for(int i = 0; i < 3; i++)
	{
		if (NRF24L01_Init())
		{
			init_ok = 1;
			break;
		}
		delay_ms(10);
	}
	// 指示灯
	while(1)
	{
		if (init_ok == 0)		// 无线模块初始化失败 红蓝LED同闪烁
		{
			LED_RED(1);
			LED_BLUE(1);
			delay_ms(500);
			LED_RED(0);
			LED_BLUE(0);
			delay_ms(500);
		}
		else
		{
			if (gamepad_flag == 0)			// 无信号红LED亮
			{
				LED_RED(1);
				LED_BLUE(0);
			}
			else if (gamepad_flag == 1)	// 无动作无LED亮
			{
				LED_RED(0);
				LED_BLUE(0);
			}
			else if (gamepad_flag == 2)	// 有动作蓝LED亮
			{
				LED_RED(0);
				LED_BLUE(1);
			}
			gamepad_flag = 0;
			delay_ms(100);
		}
	}
}




