#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "uart.h"

void UART_Write_Line (char* data)
{
	for (int i = 0; i < strlen(data); i++){
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, (uint16_t)data[i]);
	}
}

void USART1_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef initStruct;
    GPIO_StructInit(&initStruct);
    initStruct.GPIO_Pin = GPIO_Pin_9;  // Tx
    initStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    initStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &initStruct);
    GPIO_StructInit(&initStruct);
    initStruct.GPIO_Pin = GPIO_Pin_10; // Rx
    initStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    initStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &initStruct);
        
    USART_InitTypeDef initStructUSART;
	USART_StructInit(&initStructUSART);
	initStructUSART.USART_BaudRate = 115200;
	initStructUSART.USART_WordLength = USART_WordLength_8b;
	initStructUSART.USART_StopBits = USART_StopBits_1;
	initStructUSART.USART_Parity = USART_Parity_No;
	initStructUSART.USART_Mode = USART_Mode_Tx;
	initStructUSART.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &initStructUSART);
    //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    //NVIC_SetPriority(USART1_IRQn, 15);
    //NVIC_EnableIRQ(USART1_IRQn);
    USART_Cmd(USART1, ENABLE);
}