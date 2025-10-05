/*
 * spi.h
 *
 *  Created on: Sep 20, 2022
 *      Author: hussamaldean
 */

#ifndef SPI_H_
#define SPI_H_

#include "stdint.h"
#include "stm32f1xx.h"


// #define LCD_RST1 GPIOA->BSRR=GPIO_BSRR_BS0
// #define LCD_RST0 GPIOA->BSRR=GPIO_BSRR_BR0

// #define	LCD_DC1	GPIOA->BSRR=GPIO_BSRR_BS1
// #define	LCD_DC0 GPIOA->BSRR=GPIO_BSRR_BR1

#define ST7789_CS_LOW()   (GPIOC->BSRR = GPIO_BSRR_BR13)
#define ST7789_CS_HIGH()  (GPIOC->BSRR = GPIO_BSRR_BS13)

#define ST7789_DC_LOW()   (GPIOA->BSRR = GPIO_BSRR_BR6)
#define ST7789_DC_HIGH()  (GPIOA->BSRR = GPIO_BSRR_BS6)

#define ST7789_RES_LOW()  (GPIOA->BSRR = GPIO_BSRR_BR4)
#define ST7789_RES_HIGH() (GPIOA->BSRR = GPIO_BSRR_BS4)

#define ST7789_BLK_ON()   (GPIOB->BSRR = GPIO_BSRR_BS0)
#define ST7789_BLK_OFF()  (GPIOB->BSRR = GPIO_BSRR_BR0)

void st7789_spi_init();
void st7789_spi_transmit(uint8_t *data,uint32_t size);


#endif /* SPI_H_ */
