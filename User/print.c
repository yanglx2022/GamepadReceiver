/**********************
* 串口打印  USART0  PA9
**********************/
#include "print.h"

// 重定义fputc函数
int fputc(int ch, FILE *f)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
	USART_SendData(USART1, (uint8_t)ch);
	return ch;
}

// 串口初始化
void Print_Init(void)
{
	// 开启外设时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	// 设置IO PA9
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// 设置USART1
	USART_DeInit(USART1);
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate			= 115200;
	USART_InitStructure.USART_Mode					= USART_Mode_Tx/*|USART_Mode_Rx*/;
	USART_InitStructure.USART_Parity				= USART_Parity_No;
	USART_InitStructure.USART_StopBits			= USART_StopBits_1;
	USART_InitStructure.USART_WordLength		= USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
	// 串口使能
	USART_Cmd(USART1, ENABLE);
}


