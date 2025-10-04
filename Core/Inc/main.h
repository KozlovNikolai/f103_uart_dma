/**
 * @file    main.h
 * @brief   заголовочный файл для main.c
 *
 * @author  Nikolay Kozlov
 * @date    28.09.2025
 * @version 0.0
 * @note    использую стандартную библиотеку stm32f1xx.h
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef main_h
#define main_h

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"
#include "pins.h"
#include <stdint.h>
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
extern volatile uint8_t done;
extern uint8_t rc_data[];

/* Exported constants --------------------------------------------------------*/
#define SYSCLK 72000000UL
#define APB1CLK (SYSCLK / 2) // APB1 = 36 MHz
#define APB2CLK (SYSCLK)	 // APB2 = 72 MHz
#define Baudrate 115200

#define RX_DATA_SIZE 5

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* main_h */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
