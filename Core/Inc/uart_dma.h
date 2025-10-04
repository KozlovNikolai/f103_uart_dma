#ifndef UART_DMA_H
#define UART_DMA_H

#include "stm32f1xx.h"
#include "main.h"

#define rc_data_size 5

void UART2_DMA_Init(void);
void uart2_write(int ch);
/* Новая функция */
uint16_t UART2_DMA_ReceivedLength(void);
#endif