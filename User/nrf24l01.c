/**********************
* 2.4G无线  nRF24L01
**********************/
#include "nrf24l01.h"
#include "fhss.h"

// 地址宽度4字节
static const uint8_t ADDR_WIDTH = 4;
// 接收标志
uint8_t rx_received = 0;
// 负载数据
uint8_t rx_payload[32];

// NRF24L01操作线
#define CE(n)		((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_3) : GPIO_ResetBits(GPIOA, GPIO_Pin_3))	// 片选
#define CSN(n)	((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_4) : GPIO_ResetBits(GPIOA, GPIO_Pin_4))	// SPI片选
#define MOSI(n)	((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_6) : GPIO_ResetBits(GPIOA, GPIO_Pin_6))	// MOSI
#define SCK(n)	((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_5) : GPIO_ResetBits(GPIOA, GPIO_Pin_5))	// SCK
#define MISO		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)																		// 读MISO

static ErrorStatus NRF24L01_Check(void);
static uint8_t Write_Reg(uint8_t reg, uint8_t value);
static uint8_t Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t bytes);
static uint8_t Read_Reg(uint8_t reg);
static uint8_t Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes);

// 初始化
ErrorStatus NRF24L01_Init(void)
{
	/** GPIO 配置
	PA3 ------> CE
  PA4 ------> CSN
	PA5 ------> SCK
	PA6 ------> MOSI
  PA7 ------> MISO
	PB0 ------> IRQ */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// SPI
	SCK(0);
	CSN(1);
	CE(0);
	Write_Reg(W_REGISTER + CONFIG, 0x00);
	// 清FIFO(防止复位时发送复位前最后一次的数据)
	Write_Reg(FLUSH_TX, 0xFF);
	Write_Reg(FLUSH_RX, 0xFF);
	// check
	if (NRF24L01_Check() == ERROR)
	{
		return ERROR;	// 失败
	}
	// 中断配置
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line0;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearFlag(EXTI_Line0);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                    = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	return SUCCESS;
}

// 设置频道
void NRF24L01_RF_CH(uint8_t rf_ch)
{
	CE(0);
	Write_Reg(W_REGISTER + RF_CH, rf_ch);	// 修改频道
	CE(1);
}

// 接收模式初始化
void NRF24L01_RX_Mode(uint32_t address, uint8_t data_width)
{
	CE(0);
	Write_Reg(W_REGISTER + CONFIG, 0x3F);      													// 开接收中断,CRC使能2byte,开机模式,接收模式
	Write_Reg(W_REGISTER + EN_AA, 0x01);        												// 使能通道0自动应答
	Write_Reg(W_REGISTER + EN_RXADDR, 0x01);        										// 使能接收管道0
	Write_Reg(W_REGISTER + SETUP_AW, ADDR_WIDTH - 2);       						// 地址宽度
	Write_Reg(W_REGISTER + RF_CH, RF_CH_DEFAULT);												// 选择射频信道
	Write_Reg(W_REGISTER + RF_SETUP, 0x27);            									// 空中波特率250Kbps,发射功率最大
	Write_Reg(W_REGISTER + STATUS, 0xFF);  															// 清除所有的标志位
	Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t*)&address, ADDR_WIDTH);	// 接收通道0地址
	Write_Reg(W_REGISTER + RX_PW_P0, data_width);												// 接收管道0数据宽度
	CE(1);
}

// 接收中断
void EXTI0_IRQHandler(void)
{
	uint8_t status = Read_Reg(STATUS);
	if (status & RX_DR)
	{
		rx_received = 1;
		uint8_t len = Read_Reg(R_RX_PL_WID);
		Read_Buf(RD_RX_PLOAD, rx_payload, len);
		Write_Reg(FLUSH_RX, 0xFF);						// 清RX FIFO
		Write_Reg(W_REGISTER + STATUS, 0xFF);	// 清中断标志
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
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
static ErrorStatus NRF24L01_Check(void)
{
	uint8_t buf[5] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
	Write_Buf(W_REGISTER + TX_ADDR, buf, 5);
	Read_Buf(TX_ADDR, buf, 5);
	for(uint8_t i = 0; i < 5; i++)
	{
		if (buf[i] != 0xA5)
		{
			return ERROR;	// 检测错误
		}
	}
	return SUCCESS;
}


