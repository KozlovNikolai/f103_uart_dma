#include "pins.h"

void PINB_2_INIT(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // RCC - тактирование порта B на шине APB2

	/* 	Настраиваем режим работы пина PB2 через регистр CRL (Control Register Low, отвечает за пины 0–7).
	Каждый пин кодируется 4 битами: MODEy[1:0] + CNFy[1:0].
	Здесь:
	MODE2_0 и MODE2_1 задают скорость выхода:
	00 – вход.
	01 – выход 10 MHz.
	10 – выход 2 MHz.
	11 – выход 50 MHz. */

	GPIOB->CRL &= ~GPIO_CRL_MODE2_0; // сбрасывает бит MODE2_0
	GPIOB->CRL |= GPIO_CRL_MODE2_1;	 //  ставит бит MODE2_1 → получаем 10 = выход 2 MHz.

	/* Настройка параметров выхода (CNF — configuration bits).
00 → General purpose output push-pull (обычный цифровой выход, классический для светодиодов).
Если бы было 01 или 10, это был бы Open-drain или альтернативная функция */
	GPIOB->CRL &= ~GPIO_CRL_CNF2_0; // push
	GPIOB->CRL &= ~GPIO_CRL_CNF2_1; // pull
}

void TIM2_INIT(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // включить тактирование TIM2

	TIM2->PSC = 7200 - 1; // предделитель: 72 МГц / 7200 = 10 кГц
	// TIM2->ARR = 10000 - 1; // автоперезагрузка: 10000 тактов = 1 секунда
	TIM2->ARR = 5000 - 1; // автоперезагрузка: 5000 тактов = 500 миллисекунд

	TIM2->DIER |= TIM_DIER_UIE; // разрешить прерывание по обновлению
	NVIC_EnableIRQ(TIM2_IRQn);	// разрешить прерывание в NVIC
}

void Blink_Start(void)
{
	TIM2->CR1 |= TIM_CR1_CEN; // включить таймер
}

void Blink_Stop(void)
{
	TIM2->CR1 &= ~TIM_CR1_CEN;	 // выключить таймер
	GPIOB->BSRR = GPIO_BSRR_BR2; // погасить светодиод
	TIM2->SR &= ~TIM_SR_UIF;	 // сбросить флаг на всякий случай
}

// Обработчик прерывания от TIM2
void TIM2_IRQHandler(void)
{
	if (TIM2->SR & TIM_SR_UIF)
	{							 // проверка флага обновления
		TIM2->SR &= ~TIM_SR_UIF; // сброс флага

		// Переключаем состояние пина PB2
		if (GPIOB->ODR & GPIO_ODR_ODR2)
		{
			GPIOB->BSRR = GPIO_BSRR_BR2; // выключить (reset)
		}
		else
		{
			GPIOB->BSRR = GPIO_BSRR_BS2; // включить (set)
		}
	}
}

void Blink(int t)
{
	for (int i = 0; i < t; i++)
	{
		GPIOB->BSRR = GPIO_BSRR_BS2;
		for (volatile int j = 0; j < 2000000; j++)
		{
		}
		GPIOB->BSRR = GPIO_BSRR_BR2;
		for (volatile int j = 0; j < 2000000; j++)
		{
		}
	}
}