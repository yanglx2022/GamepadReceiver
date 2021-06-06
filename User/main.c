// 2.4GתUSB Gamepad
#include "usb_hw.h"
#include "delay.h"
#include "led.h"
#include "nrf24l01.h"

#define IAP		// ֧��IAP����, ��ʱ��Ҫ����IROM1��ʼ��ַ

// ң�ر�־
extern uint8_t gamepad_flag;

int main(void)
{
	// IAP��������
	#ifdef IAP
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4400); // �����ж�������
	__enable_irq();	// �����ж�
	#endif
	// �����ж����ȼ���
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init();
	LED_Init();
	// USB��ʼ��
  USB_HW_Init();
  USB_Init();
	// 2.4G����ģ���ʼ��(ʧ�ܳ���3��)
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
	// ָʾ��
	while(1)
	{
		if (init_ok == 0)		// ����ģ���ʼ��ʧ�� ����LEDͬ��˸
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
			if (gamepad_flag == 0)			// ���źź�LED��
			{
				LED_RED(1);
				LED_BLUE(0);
			}
			else if (gamepad_flag == 1)	// �޶�����LED��
			{
				LED_RED(0);
				LED_BLUE(0);
			}
			else if (gamepad_flag == 2)	// �ж�����LED��
			{
				LED_RED(0);
				LED_BLUE(1);
			}
			gamepad_flag = 0;
			delay_ms(100);
		}
	}
}




