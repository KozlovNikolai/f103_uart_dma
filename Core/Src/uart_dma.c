#include "uart_dma.h"

uint8_t rc_data[rc_data_size];
volatile uint8_t done = 0;

/* --- Вспомогательные функции --- */
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate)
{
    return ((PeriphClk + (BaudRate / 2U)) / BaudRate);
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate)
{
    USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

/* --- Отправка байта --- */
void uart2_write(int ch)
{
    while (!(USART2->SR & USART_SR_TXE))
        ;
    USART2->DR = (ch & 0xFF);
}

/* --- Retarget printf --- */
int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++)
        uart2_write(ptr[i]);
    return len;
}

/* --- Настройка UART2 + DMA --- */
void UART2_DMA_Init(void)
{
    /* Тактирование */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* Настройка GPIO */
    GPIOA->CRL &= ~(GPIO_CRL_MODE2_Msk | GPIO_CRL_CNF2_Msk |
                    GPIO_CRL_MODE3_Msk | GPIO_CRL_CNF3_Msk);

    GPIOA->CRL |= (GPIO_CRL_MODE2_1 | GPIO_CRL_MODE2_0); // 50 MHz
    GPIOA->CRL |= GPIO_CRL_CNF2_1;                       // AF push-pull
    GPIOA->CRL |= GPIO_CRL_CNF3_0;                       // RX floating

    AFIO->MAPR &= ~AFIO_MAPR_USART2_REMAP;

    /* USART */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART2->CR3 |= USART_CR3_DMAR;
    uart_set_baudrate(USART2, APB1CLK, Baudrate);

    /* DMA */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel6->CCR = DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TCIE;
    DMA1_Channel6->CPAR = (uint32_t)&USART2->DR;
    DMA1_Channel6->CMAR = (uint32_t)rc_data;
    DMA1_Channel6->CNDTR = rc_data_size;
    DMA1_Channel6->CCR |= DMA_CCR_EN;

    NVIC_EnableIRQ(DMA1_Channel6_IRQn);

    USART2->CR1 |= USART_CR1_UE;
}

/* --- Прерывание DMA --- */
void DMA1_Channel6_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TCIF6)
    {
        done = 1;
        DMA1->IFCR = DMA_IFCR_CTCIF6;
    }
}