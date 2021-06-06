/**********************
* 2.4G无线  nRF24L01
**********************/
#include "nrf24l01.h"
#include "usb_hw.h"

// 遥控标志
uint8_t gamepad_flag = 0;

// 地址
#define ADDR_WIDTH		3		// 地址宽度(3~5bytes)
const static uint8_t ADDRESS[ADDR_WIDTH] = {0x4A, 0x22, 0x50};

// NRF24L01操作线
#define CE(n)		((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_3) : GPIO_ResetBits(GPIOA, GPIO_Pin_3))	// 片选
#define CSN(n)	((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_4) : GPIO_ResetBits(GPIOA, GPIO_Pin_4))	// SPI片选
#define MOSI(n)	((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_6) : GPIO_ResetBits(GPIOA, GPIO_Pin_6))	// MOSI
#define SCK(n)	((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_5) : GPIO_ResetBits(GPIOA, GPIO_Pin_5))	// SCK
#define MISO		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)																		// 读MISO

static uint8_t NRF24L01_Check(void);
static void NRF24L01_RX_Mode(void);

// 初始化
uint8_t NRF24L01_Init(void)
{
	/** GPIO 配置
	PA3 ------> CE
  PA4 ------> CSN
	PA5 ------> SCK
	PA6 ------> MOSI
  PA7 ------> MISO
	PB0 ------> IRQ */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// SPI
	SCK(0);
	CSN(1);
	CE(0);
	// check
	if (NRF24L01_Check() == 0)
	{
		return 0;	// 失败
	}
	// 中断配置
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_DeInit();
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line0;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearFlag(EXTI_Line0);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                    = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	// RX Mode
	NRF24L01_RX_Mode();
	return 1;
}

// 模拟SPI读写字节
static uint8_t SPI_RW(uint8_t byte)
{
	for(uint8_t i = 0; i < 8; i++)
	{
		MOSI(byte & 0x80);
		byte = byte << 1;
		SCK(1);
		byte |= MISO;
		SCK(0);
	}
	return(byte);                              
}

// 写寄存器(单字节)
static uint8_t Write_Reg(uint8_t reg, uint8_t value)
{
	CSN(0);
	uint8_t status = SPI_RW(reg);
	SPI_RW(value);
	CSN(1);
	return status;
}

// 写寄存器(多字节)
static uint8_t Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t bytes)
{
	CSN(0);
	uint8_t status = SPI_RW(reg);
	for(uint8_t i = 0; i < bytes; i++)
	{
		SPI_RW(*pBuf++);
	}
	CSN(1);
	return status;
}		

// 读寄存器(单字节)
static uint8_t Read_Reg(uint8_t reg)
{
	CSN(0);
	SPI_RW(reg);
	uint8_t val = SPI_RW(0);
	CSN(1);
	return val;
}

// 读寄存器(多字节)
static uint8_t Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	CSN(0);
	uint8_t status = SPI_RW(reg);
	for(uint8_t i = 0; i < bytes; i++)
	{
		pBuf[i] = SPI_RW(0);
	}
	CSN(1);
	return status;
}

// 检测nRF24L01
static uint8_t NRF24L01_Check(void)
{
	uint8_t buf[5] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
	Write_Buf(W_REGISTER + TX_ADDR, buf, 5);
	Read_Buf(TX_ADDR, buf, 5);
	for(uint8_t i = 0; i < 5; i++)
	{
		if (buf[i] != 0xA5)
		{
			return 0;	// 检测错误
		}
	}
	return 1;
}

// 接收模式初始化
static void NRF24L01_RX_Mode(void)
{
	CE(0);
	Write_Reg(W_REGISTER + CONFIG, 0x3F);      							// 开接收中断,CRC使能2byte,开机模式,接收模式
	Write_Reg(W_REGISTER + EN_AA, 0x01);        						// 使能通道0自动应答
	Write_Reg(W_REGISTER + EN_RXADDR, 0x01);        				// 使能接收管道0
	Write_Reg(W_REGISTER + SETUP_AW, ADDR_WIDTH - 2);       // 地址宽度
	Write_Reg(W_REGISTER + RF_CH, 20);                 			// 选择射频信道2420MHz
	Write_Reg(W_REGISTER + RF_SETUP, 0x27);            			// 空中波特率250Kbps,发射功率最大
	Write_Reg(W_REGISTER + STATUS, 0xFF);  									// 清除所有的标志位
	Write_Buf(W_REGISTER + RX_ADDR_P0, ADDRESS, ADDR_WIDTH);// 接收通道0地址
	Write_Reg(W_REGISTER + RX_PW_P0, PAYLOAD_WIDTH);				// 接收管道0数据宽度
	CE(1);
}

// 接收中断
void EXTI0_IRQHandler(void)
{
 	Read_Reg(STATUS);		// 第一次读总是返回0，要连读两次
	uint8_t status = Read_Reg(STATUS);
	if (status & RX_DR)
	{
		uint8_t payload[PAYLOAD_WIDTH];
		Read_Buf(RD_RX_PLOAD, payload, PAYLOAD_WIDTH);
		Write_Reg(W_REGISTER + STATUS, status);	// 清中断标志
		Write_Reg(FLUSH_RX, 0xFF);							// 清RX FIFO寄存器
		// USB数据为-127~127
		uint8_t auto_back = payload[0];
		payload[0] = payload[1];
		payload[1] = *(int16_t*)(payload + 2) * 127 / 200;
		if (auto_back & 0x01)
		{
			payload[2] = -*(int16_t*)(payload + 4) * 127 / 200;
		}
		else
		{
			payload[2] = -(*(int16_t*)(payload + 4) - 200) * 127 / 200;
		}
		payload[3] = *(int16_t*)(payload + 6) * 127 / 200;
		payload[4] = -*(int16_t*)(payload + 8) * 127 / 200;
		// USB传输
		USB_Send(payload);
		// 遥控标志
		if (payload[0] == 0 && *(uint32_t*)(payload + 1) == 0)
		{
			gamepad_flag = 1;
		}
		else
		{
			gamepad_flag = 2;
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
}


