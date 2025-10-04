#include "clock.h"


void SystemClock_72_Config(void)
{
	// 1. Включаем внешний кварц HSE
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY))
		; // ждем готовности

	// 2. Настройка Flash latency (2 wait states для 72 МГц)
	FLASH->ACR |= FLASH_ACR_PRFTBE;
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	// FLASH->ACR |= FLASH_ACR_LATENCY_2;
	FLASH->ACR |= 0x2UL;

	// 3. Настройка PLL: источник HSE, умножение ×9 (8 МГц × 9 = 72 МГц)
	RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);
	RCC->CFGR |= (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);

	// 4. Настройка делителей шин (AHB = 72, APB1 = 36, APB2 = 72)
	RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 max 36 MHz

	// 5. Включаем PLL
	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY))
		; // ждем готовности PLL

	// 6. Переключаемся на PLL как источник SYSCLK
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		;
}