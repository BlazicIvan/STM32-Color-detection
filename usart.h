#ifndef USART_H_
#define USART_H_

#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_conf.h"


void USART1_init();
void UART_Write_Line (char* data);
#endif