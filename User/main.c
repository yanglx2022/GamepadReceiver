/*****************************************************************************
* @brief   2.4G����תUSB Gamepad ֧����Ƶ�����
* @author  yanglx2022
* @version V0.2
* @date    2021/7/11
******************************************************************************/
#include "delay.h"
#include "print.h"
#include "key.h"
#include "led.h"
#include "nrf24l01.h"
#include "fhss.h"
#include "flash.h"
#include "usb_hw.h"

#define IAP		// ֧��IAP����, ��ʱ��Ҫ����IROM1��ʼ��ַ

// ����ģʽ
typedef enum{
	MODE_NORMAL,	// ��������ģʽ
	MODE_MATCH,		// ��Խ���ģʽ
	MODE_FHSS			// ������Ƶͼ��ģʽ
}WORK_MODE;
static WORK_MODE work_mode = MODE_NORMAL;

// ����Ե�ͨ�ŵ�ַ
static uint32_t match_addr = 0;

// Ĭ�ϵ�ַ
static const uint32_t ADDR_DEFAULT = 0x59414E47;

static void nrf24l01_error_exit(void);
static void wait_for_usb_configured_or_exit(void);
static ErrorStatus match_addr_read(uint32_t* addr);
static void mode_normal_proc(void);
static void mode_match_proc(void);
static void mode_fhss_proc(void);

int main(void)
{
	// IAP��������
	#ifdef IAP
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4400); // �����ж�������
	__enable_irq();	// �����ж�
	#endif
	// ���ڳ�ʼ��
	Print_Init();
	// LED��ʼ������һ��
	LED_Init();
	LED_BLUE(1);	
	delay_ms(100);
	LED_BLUE(0);
	// ����ģ���ʼ��
	if (NRF24L01_Init() == ERROR)
	{
		nrf24l01_error_exit();
	}
	// USB��ʼ��
	USB_HW_Init();
	USB_Init();
	USB_Connect(TRUE);
	wait_for_usb_configured_or_exit();
	// ������ʼ��
	KEY_Init();
	// ��FLASH��ȡͨ�ŵ�ַ
	if (match_addr_read(&match_addr) == ERROR)
	{
		// ��Խ���ģʽ
		work_mode = MODE_MATCH;
	}
	else
	{
		printf("�����(��ַ%08x)\r\n", match_addr);
		// ������Ƶͼ��ģʽ
		work_mode = MODE_FHSS;
	}
	while(1)
	{
		switch(work_mode)
		{
			case MODE_MATCH:	// ��Խ���ģʽ
				mode_match_proc();
				break;
			case MODE_FHSS:		// ������Ƶͼ��ģʽ
				mode_fhss_proc();
				break;
			case MODE_NORMAL:	// ��������ģʽ
				mode_normal_proc();
				break;
		}
	}
}

// ����ģ���ʼ��ʧ���˳�
static void nrf24l01_error_exit(void)
{
	printf("NRF24L01��ʼ��ʧ��\r\n");
	// ��ɫLED��˸(2Hz)5s
	for (int i = 0; i < 10; i++)
	{
		LED_RED(1);
		delay_ms(250);
		LED_RED(0);
		delay_ms(250);
	}
	// ����(�ȴ������ϵ�)
	PWR_EnterSTANDBYMode();
	while(1);
}

// �ȴ�USB������ɻ��߳�ʱ�˳�
static void wait_for_usb_configured_or_exit(void)
{
	uint8_t cnt = 0;
	while(DeviceState != CONFIGURED)
	{
		if (cnt < 4)
		{
			delay_ms(500);
		}
		else if (cnt < 20)
		{
			// ��ɫLED��˸(2Hz)
			LED_RED(1);
			delay_ms(250);
			LED_RED(0);
			delay_ms(250);
		}
		else
		{
			printf("USB����ʧ��\r\n");
			// ����(�ȴ������ϵ�)
			PWR_EnterSTANDBYMode();
			while(1);
		}
		cnt++;
	}
}

// ��FLASH��ȡ�����ͨ�ŵ�ַ
static ErrorStatus match_addr_read(uint32_t* addr)
{
	uint32_t data[6];
	Flash_Read(FLASH_APP_BASE - 24, (uint8_t*)data, 24);
	if (data[0] == ~data[1])
	{
		*addr = data[0];
		return SUCCESS;
	}
	else if (data[2] == ~data[3])
	{
		*addr = data[2];
		return SUCCESS;
	}
	else if (data[4] == ~data[5])
	{
		*addr = data[4];
		return SUCCESS;
	}
	return ERROR;
}

// �������ͨ�ŵ�ַд��FLASH
static void match_addr_write(uint32_t addr)
{
	// ȡ��У�� ����洢
	uint32_t data[] = {addr, ~addr, addr, ~addr, addr, ~addr};
	Flash_Write(FLASH_APP_BASE - 24, (uint8_t*)data, 24);
}

// USB���ݸ�ʽת��
static uint8_t* usb_data_format(uint8_t* data)
{
	// �������ݸ�ʽ(10bytes)��1byte����+1byte����+2bytesҡ��*4
	// USB���ݸ�ʽ(5bytes)��1byte����+1byteҡ��*4
	uint8_t auto_back = data[0];
	data[0] = data[1];
	data[1] = *(int16_t*)(data + 2) * 127 / 200;
	if (auto_back & 0x01)
	{
		data[2] = -*(int16_t*)(data + 4) * 127 / 200;
	}
	else
	{
		data[2] = -(*(int16_t*)(data + 4) - 200) * 127 / 200;
	}
	data[3] = *(int16_t*)(data + 6) * 127 / 200;
	data[4] = -*(int16_t*)(data + 8) * 127 / 200;
	return data;
}

// ��Խ���ģʽ����
static void mode_match_proc(void)
{
	printf("������Խ���ģʽ\r\n");
	// ���ģʽ����LED����
	LED_ALL(1);
	// �ر��ж�
	USB_IRQ_Disable();
	KEY_IRQ_Disable();
	// ��������ģ��
	NRF24L01_RX_Mode(ADDR_DEFAULT, 4);
	while(1)
	{
		__WFI();	// ˯��(ֻ��ͣCPU)
		if (rx_received)	// �յ����
		{
			rx_received = 0;
			break;
		}
	}
	// ��¼��Ե�ַ
	match_addr = *(uint32_t*)rx_payload;
	match_addr_write(match_addr);
	// ����KEY�ж�
	KEY_IRQ_Enable();
	// ת��������Ƶͼ��ģʽ
	work_mode = MODE_FHSS;
	LED_ALL(0);
}

// ������Ƶͼ��ģʽ����
static void mode_fhss_proc(void)
{
	printf("���������Ƶͼ��ģʽ\r\n");
	NRF24L01_RX_Mode(match_addr, FHSS_NUM);
	uint32_t cnt = 0;
	while(rx_received == 0)	// �ȴ�����
	{
		// ��������
		if (Key_Press_Hold())
		{
			// ת����Խ���ģʽ
			work_mode = MODE_MATCH;
			return;
		}
		else
		{
			// ����LEDͬ��
			if (cnt % 50 == 0)
			{
				LED_ALL(1);
			}
			else if (cnt % 50 == 1)
			{
				LED_ALL(0);
			}
			delay_ms(20);
			cnt++;
		}
	}
	rx_received = 0;
	fhss_update(rx_payload);
	// ת����������ģʽ
	work_mode = MODE_NORMAL;
}

// ��������ģʽ����
static void mode_normal_proc(void)
{
	printf("������������ģʽ\r\n");
	LED_RED(0);
	// ����USB�ж�
	USB_IRQ_Enable();
	// ��������ģ��
	NRF24L01_RX_Mode(match_addr, 10);
	// ������Ƶ
	fhss_start();
	// ����
	uint32_t tick = MILLIS_TICK;
	while(!TICK_TIMEOUT(tick, 1000))	// 1s��ʱ
	{
		if (rx_received)
		{
			rx_received = 0;
			tick = MILLIS_TICK;
			// ��Ƶ
			fhss_set();
			// USB ����
			USB_Send(usb_data_format(rx_payload));
			// �ж���ʱ��ɫLED��
			LED_BLUE(rx_payload[0] || *(uint32_t*)(rx_payload + 1));
			// ��������
			if (Key_Press_Hold())
			{
				// ת����Խ���ģʽ
				work_mode = MODE_MATCH;
				break;
			}
		}
	}
	LED_BLUE(0);
	// ֹͣ��Ƶ
	fhss_stop();
	if (work_mode == MODE_NORMAL)
	{
		// ת��������Ƶͼ��ģʽ
		work_mode = MODE_FHSS;
	}
}


