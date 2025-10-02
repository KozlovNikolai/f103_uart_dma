#include "main.h"
#include "stdio.h"

#define Perpher_CLK 8000000
#define Baudrate 115200

static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate)
{
	return ((PeriphClk + (BaudRate / 2U)) / BaudRate);
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate)
{
	USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

void uart2_write(int ch)
{
	/*Make sure the transmit data register is empty*/
	while (!(USART2->SR & USART_SR_TXE))
	{
	}

	/*Write to transmit data register*/
	USART2->DR = (ch & 0xFF);
}

/*Retargeting printf*/
int __io_putchar(int ch)
{
	uart2_write(ch);
	return ch;
}

#define rc_data_size 5
uint8_t rc_data[rc_data_size];

volatile uint8_t done = 0;

void SystemClock_Config(void)
{
	// 1. Включаем внешний кварц HSE
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY))
		; // ждем готовности

	// 2. Настройка Flash latency (2 wait states для 72 МГц)
	FLASH->ACR |= FLASH_ACR_PRFTBE;
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |= FLASH_ACR_LATENCY_2;

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

void PINB_2_INIT(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // RCC

	GPIOB->CRL &= ~GPIO_CRL_MODE2_0; // 0: output, clear mode bits
	GPIOB->CRL |= GPIO_CRL_MODE2_1;	 // 1: output 2 MHz

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
	TIM2->CR1 |= TIM_CR1_CEN;	// включить таймер

	NVIC_EnableIRQ(TIM2_IRQn); // разрешить прерывание в NVIC
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

void TIM3_CH2_Output_1MHz(void)
{
	// Включаем порт A и TIM3
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN; // тактирование GPIOA и AFIO
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;						 // тактирование TIM3

	// Настройка PA7 как AF Push-Pull (TIM3_CH2)
	GPIOA->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);	 // сброс битов
	GPIOA->CRL |= (GPIO_CRL_MODE7_1 | GPIO_CRL_MODE7_0); // выход 50 MHz
	GPIOA->CRL |= GPIO_CRL_CNF7_1;						 // AF Push-Pull

	// Настраиваем TIM3
	TIM3->PSC = 0;	 // предделитель = 1
	TIM3->ARR = 71;	 // период = 72 MHz / 72 = 1 MHz
	TIM3->CCR2 = 36; // 50% скважность
	TIM3->CCMR1 &= ~TIM_CCMR1_OC2M;
	TIM3->CCMR1 |= (6 << TIM_CCMR1_OC2M_Pos); // PWM mode 1
	TIM3->CCER |= TIM_CCER_CC2E;			  // включаем канал
	TIM3->CR1 |= TIM_CR1_CEN;				  // старт таймера
}

int main(void)
{
	// SystemClock_Config(); // включаем 72 МГц

	// PINB_2_INIT();
	// TIM2_INIT();
	// TIM3_CH2_Output_1MHz();

	// while (1)
	// {
	// }
	/*UART2 Pin configures*/

	// enable clock access to GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	// Enable clock access to alternate function
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	/*Confgiure PA2 as output maximum speed to 50MHz
	 * and alternate output push-pull mode*/
	GPIOA->CRL |= GPIO_CRL_MODE2;

	GPIOA->CRL |= GPIO_CRL_CNF2_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF2_0;

	/*Configure PA3 as Input floating*/

	/*Set mode to be input*/
	GPIOA->CRL &= ~(GPIO_CRL_MODE3);
	GPIOA->CRL |= GPIO_CRL_CNF3_0;
	GPIOA->CRL &= ~GPIO_CRL_CNF3_1;

	/*Don't remap the pins*/
	AFIO->MAPR &= ~AFIO_MAPR_USART2_REMAP;

	/*USART2 configuration*/

	// enable clock access to USART2

	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	// Transmit Enable
	USART2->CR1 |= USART_CR1_TE;

	// Enable receiver
	USART2->CR1 |= USART_CR1_RE;

	/*Enable DMA for receiver*/
	USART2->CR3 |= USART_CR3_DMAR;

	/*Set baudrate */
	uart_set_baudrate(USART2, Perpher_CLK, Baudrate);

	/*DMA configuration*/

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

	DMA1_Channel6->CCR |= DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TCIE;

	NVIC_EnableIRQ(DMA1_Channel6_IRQn);

	/*Set the peripheral address to be USART2->DR*/
	DMA1_Channel6->CPAR = (uint32_t)&USART2->DR;

	DMA1_Channel6->CMAR = (uint32_t)rc_data;

	DMA1_Channel6->CNDTR = rc_data_size;

	DMA1_Channel6->CCR |= DMA_CCR_EN;

	// Enable UART
	USART2->CR1 |= USART_CR1_UE;

	printf("Hello from DMA");

	while (1)
	{
		while (done == 0)
			;
		printf("Received data:\"%s\"\r\n", rc_data);
		done = 0;
	}
}

void DMA1_Channel6_IRQHandler(void)
{
	if (DMA1->ISR & DMA_ISR_TCIF6)
	{
		done = 1;
		DMA1->IFCR = DMA_IFCR_CTCIF6;
	}
}