/*****************************************************************************
* @brief   2.4G接收转USB Gamepad 支持跳频与配对
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

#define IAP		// 支持IAP升级, 此时需要设置IROM1起始地址

// 工作模式
typedef enum{
	MODE_NORMAL,	// 正常接收模式
	MODE_MATCH,		// 配对接收模式
	MODE_FHSS			// 接收跳频图案模式
}WORK_MODE;
static WORK_MODE work_mode = MODE_NORMAL;

// 已配对的通信地址
static uint32_t match_addr = 0;

// 默认地址
static const uint32_t ADDR_DEFAULT = 0x59414E47;

static void nrf24l01_error_exit(void);
static void wait_for_usb_configured_or_exit(void);
static ErrorStatus match_addr_read(uint32_t* addr);
static void mode_normal_proc(void);
static void mode_match_proc(void);
static void mode_fhss_proc(void);

int main(void)
{
	// IAP升级设置
	#ifdef IAP
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4400); // 设置中断向量表
	__enable_irq();	// 开总中断
	#endif
	// 串口初始化
	Print_Init();
	// LED初始化并闪一下
	LED_Init();
	LED_BLUE(1);	
	delay_ms(100);
	LED_BLUE(0);
	// 无线模块初始化
	if (NRF24L01_Init() == ERROR)
	{
		nrf24l01_error_exit();
	}
	// USB初始化
	USB_HW_Init();
	USB_Init();
	USB_Connect(TRUE);
	wait_for_usb_configured_or_exit();
	// 按键初始化
	KEY_Init();
	// 从FLASH读取通信地址
	if (match_addr_read(&match_addr) == ERROR)
	{
		// 配对接收模式
		work_mode = MODE_MATCH;
	}
	else
	{
		printf("已配对(地址%08x)\r\n", match_addr);
		// 接收跳频图案模式
		work_mode = MODE_FHSS;
	}
	while(1)
	{
		switch(work_mode)
		{
			case MODE_MATCH:	// 配对接收模式
				mode_match_proc();
				break;
			case MODE_FHSS:		// 接收跳频图案模式
				mode_fhss_proc();
				break;
			case MODE_NORMAL:	// 正常接收模式
				mode_normal_proc();
				break;
		}
	}
}

// 无线模块初始化失败退出
static void nrf24l01_error_exit(void)
{
	printf("NRF24L01初始化失败\r\n");
	// 红色LED闪烁(2Hz)5s
	for (int i = 0; i < 10; i++)
	{
		LED_RED(1);
		delay_ms(250);
		LED_RED(0);
		delay_ms(250);
	}
	// 待机(等待重新上电)
	PWR_EnterSTANDBYMode();
	while(1);
}

// 等待USB配置完成或者超时退出
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
			// 红色LED闪烁(2Hz)
			LED_RED(1);
			delay_ms(250);
			LED_RED(0);
			delay_ms(250);
		}
		else
		{
			printf("USB连接失败\r\n");
			// 待机(等待重新上电)
			PWR_EnterSTANDBYMode();
			while(1);
		}
		cnt++;
	}
}

// 从FLASH读取已配对通信地址
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

// 将已配对通信地址写入FLASH
static void match_addr_write(uint32_t addr)
{
	// 取反校验 冗余存储
	uint32_t data[] = {addr, ~addr, addr, ~addr, addr, ~addr};
	Flash_Write(FLASH_APP_BASE - 24, (uint8_t*)data, 24);
}

// USB数据格式转换
static uint8_t* usb_data_format(uint8_t* data)
{
	// 接收数据格式(10bytes)：1byte回中+1byte按键+2bytes摇杆*4
	// USB数据格式(5bytes)：1byte按键+1byte摇杆*4
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

// 配对接收模式处理
static void mode_match_proc(void)
{
	printf("进入配对接收模式\r\n");
	// 配对模式红蓝LED常亮
	LED_ALL(1);
	// 关闭中断
	USB_IRQ_Disable();
	KEY_IRQ_Disable();
	// 配置无线模块
	NRF24L01_RX_Mode(ADDR_DEFAULT, 4);
	while(1)
	{
		__WFI();	// 睡眠(只暂停CPU)
		if (rx_received)	// 收到配对
		{
			rx_received = 0;
			break;
		}
	}
	// 记录配对地址
	match_addr = *(uint32_t*)rx_payload;
	match_addr_write(match_addr);
	// 开启KEY中断
	KEY_IRQ_Enable();
	// 转到接收跳频图案模式
	work_mode = MODE_FHSS;
	LED_ALL(0);
}

// 接收跳频图案模式处理
static void mode_fhss_proc(void)
{
	printf("进入接收跳频图案模式\r\n");
	NRF24L01_RX_Mode(match_addr, FHSS_NUM);
	uint32_t cnt = 0;
	while(rx_received == 0)	// 等待接收
	{
		// 按键长按
		if (Key_Press_Hold())
		{
			// 转到配对接收模式
			work_mode = MODE_MATCH;
			return;
		}
		else
		{
			// 红蓝LED同闪
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
	// 转到正常接收模式
	work_mode = MODE_NORMAL;
}

// 正常接收模式处理
static void mode_normal_proc(void)
{
	printf("进入正常接收模式\r\n");
	LED_RED(0);
	// 开启USB中断
	USB_IRQ_Enable();
	// 设置无线模块
	NRF24L01_RX_Mode(match_addr, 10);
	// 启动跳频
	fhss_start();
	// 接收
	uint32_t tick = MILLIS_TICK;
	while(!TICK_TIMEOUT(tick, 1000))	// 1s超时
	{
		if (rx_received)
		{
			rx_received = 0;
			tick = MILLIS_TICK;
			// 跳频
			fhss_set();
			// USB 传输
			USB_Send(usb_data_format(rx_payload));
			// 有动作时蓝色LED亮
			LED_BLUE(rx_payload[0] || *(uint32_t*)(rx_payload + 1));
			// 按键长按
			if (Key_Press_Hold())
			{
				// 转到配对接收模式
				work_mode = MODE_MATCH;
				break;
			}
		}
	}
	LED_BLUE(0);
	// 停止跳频
	fhss_stop();
	if (work_mode == MODE_NORMAL)
	{
		// 转到接收跳频图案模式
		work_mode = MODE_FHSS;
	}
}


