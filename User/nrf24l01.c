/**********************
* 2.4G����  nRF24L01
**********************/
#include "nrf24l01.h"
#include "fhss.h"

// ��ַ���4�ֽ�
static const uint8_t ADDR_WIDTH = 4;
// ���ձ�־
uint8_t rx_received = 0;
// ��������
uint8_t rx_payload[32];

// NRF24L01������
#define CE(n)		((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_3) : GPIO_ResetBits(GPIOA, GPIO_Pin_3))	// Ƭѡ
#define CSN(n)	((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_4) : GPIO_ResetBits(GPIOA, GPIO_Pin_4))	// SPIƬѡ
#define MOSI(n)	((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_6) : GPIO_ResetBits(GPIOA, GPIO_Pin_6))	// MOSI
#define SCK(n)	((n) ? GPIO_SetBits(GPIOA, GPIO_Pin_5) : GPIO_ResetBits(GPIOA, GPIO_Pin_5))	// SCK
#define MISO		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)																		// ��MISO

static ErrorStatus NRF24L01_Check(void);
static uint8_t Write_Reg(uint8_t reg, uint8_t value);
static uint8_t Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t bytes);
static uint8_t Read_Reg(uint8_t reg);
static uint8_t Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes);

// ��ʼ��
ErrorStatus NRF24L01_Init(void)
{
	/** GPIO ����
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
	// ��FIFO(��ֹ��λʱ���͸�λǰ���һ�ε�����)
	Write_Reg(FLUSH_TX, 0xFF);
	Write_Reg(FLUSH_RX, 0xFF);
	// check
	if (NRF24L01_Check() == ERROR)
	{
		return ERROR;	// ʧ��
	}
	// �ж�����
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

// ����Ƶ��
void NRF24L01_RF_CH(uint8_t rf_ch)
{
	CE(0);
	Write_Reg(W_REGISTER + RF_CH, rf_ch);	// �޸�Ƶ��
	CE(1);
}

// ����ģʽ��ʼ��
void NRF24L01_RX_Mode(uint32_t address, uint8_t data_width)
{
	CE(0);
	Write_Reg(W_REGISTER + CONFIG, 0x3F);      													// �������ж�,CRCʹ��2byte,����ģʽ,����ģʽ
	Write_Reg(W_REGISTER + EN_AA, 0x01);        												// ʹ��ͨ��0�Զ�Ӧ��
	Write_Reg(W_REGISTER + EN_RXADDR, 0x01);        										// ʹ�ܽ��չܵ�0
	Write_Reg(W_REGISTER + SETUP_AW, ADDR_WIDTH - 2);       						// ��ַ���
	Write_Reg(W_REGISTER + RF_CH, RF_CH_DEFAULT);												// ѡ����Ƶ�ŵ�
	Write_Reg(W_REGISTER + RF_SETUP, 0x27);            									// ���в�����250Kbps,���书�����
	Write_Reg(W_REGISTER + STATUS, 0xFF);  															// ������еı�־λ
	Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t*)&address, ADDR_WIDTH);	// ����ͨ��0��ַ
	Write_Reg(W_REGISTER + RX_PW_P0, data_width);												// ���չܵ�0���ݿ��
	CE(1);
}

// �����ж�
void EXTI0_IRQHandler(void)
{
	uint8_t status = Read_Reg(STATUS);
	if (status & RX_DR)
	{
		rx_received = 1;
		uint8_t len = Read_Reg(R_RX_PL_WID);
		Read_Buf(RD_RX_PLOAD, rx_payload, len);
		Write_Reg(FLUSH_RX, 0xFF);						// ��RX FIFO
		Write_Reg(W_REGISTER + STATUS, 0xFF);	// ���жϱ�־
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
}

// ģ��SPI��д�ֽ�
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

// д�Ĵ���(���ֽ�)
static uint8_t Write_Reg(uint8_t reg, uint8_t value)
{
	CSN(0);
	uint8_t status = SPI_RW(reg);
	SPI_RW(value);
	CSN(1);
	return status;
}

// д�Ĵ���(���ֽ�)
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

// ���Ĵ���(���ֽ�)
static uint8_t Read_Reg(uint8_t reg)
{
	CSN(0);
	SPI_RW(reg);
	uint8_t val = SPI_RW(0);
	CSN(1);
	return val;
}

// ���Ĵ���(���ֽ�)
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

// ���nRF24L01
static ErrorStatus NRF24L01_Check(void)
{
	uint8_t buf[5] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
	Write_Buf(W_REGISTER + TX_ADDR, buf, 5);
	Read_Buf(TX_ADDR, buf, 5);
	for(uint8_t i = 0; i < 5; i++)
	{
		if (buf[i] != 0xA5)
		{
			return ERROR;	// ������
		}
	}
	return SUCCESS;
}


