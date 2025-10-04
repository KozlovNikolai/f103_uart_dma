#include "uart_dma.h"
#include "main.h"

#define UART_TX_BUF_SIZE 128

/* --- Глобальные переменные --- */
uint8_t rc_data[RX_DATA_SIZE];
volatile uint8_t done = 0;

static uint8_t uart_tx_buf[UART_TX_BUF_SIZE];
static volatile uint16_t tx_len = 0;

/* --- Вспомогательные функции --- */
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate)
{
    return ((PeriphClk + (BaudRate / 2U)) / BaudRate);
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate)
{
    USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

/* --- Настройка UART2 + DMA --- */
void UART2_DMA_Init(void)
{
    /* Тактирование GPIO, AFIO и USART2 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* Настройка GPIO */
    GPIOA->CRL &= ~(GPIO_CRL_MODE2_Msk | GPIO_CRL_CNF2_Msk |
                    GPIO_CRL_MODE3_Msk | GPIO_CRL_CNF3_Msk);

    GPIOA->CRL |= (GPIO_CRL_MODE2_1 | GPIO_CRL_MODE2_0); // 50 MHz
    GPIOA->CRL |= GPIO_CRL_CNF2_1;                       // AF push-pull
    GPIOA->CRL |= GPIO_CRL_CNF3_0;                       // RX floating

    AFIO->MAPR &= ~AFIO_MAPR_USART2_REMAP;

    /* Настройка USART */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;

  /* Включаем DMA контроллер (общий) */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    uart_set_baudrate(USART2, APB1CLK, Baudrate);

/* --- Настройка DMA RX (Channel6) --- */
    /* Сбросим флаги на всякий случай */
    DMA1->IFCR = DMA_IFCR_CTCIF6 | DMA_IFCR_CTCIF7;

    DMA1_Channel6->CCR = 0;
    DMA1_Channel6->CPAR = (uint32_t)&USART2->DR;
    DMA1_Channel6->CMAR = (uint32_t)rc_data;
    DMA1_Channel6->CNDTR = RX_DATA_SIZE;
    DMA1_Channel6->CCR = DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TCIE; // перезаписываем CCR целиком
    DMA1_Channel6->CCR |= DMA_CCR_EN; // включаем RX канал

    NVIC_EnableIRQ(DMA1_Channel6_IRQn);

    /* --- Настройка DMA TX (Channel7) --- */
    DMA1_Channel7->CCR = 0; // сброс
    DMA1_Channel7->CPAR = (uint32_t)&USART2->DR;
    DMA1_Channel7->CMAR = (uint32_t)uart_tx_buf;
    /* Memory->Peripheral, memory increment, TC interrupt. Не включаем EN пока не дадим CNDTR. */
    DMA1_Channel7->CCR = DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_TCIE;

    NVIC_EnableIRQ(DMA1_Channel7_IRQn);

    /* Включаем DMA запросы в USART: и на приём, и на передачу */
    USART2->CR3 |= (USART_CR3_DMAR | USART_CR3_DMAT);

    /* Включаем сам USART */
    USART2->CR1 |= USART_CR1_UE;
}

/* --- Отправка через printf (_write) --- */
int _write(int file, char *ptr, int len)
{
    if (len == 0) return 0;
    if (len > UART_TX_BUF_SIZE) len = UART_TX_BUF_SIZE;

    /* Простая защита: если предыдущая передача ещё идёт - ждём с таймаутом */
    uint32_t timeout = 100000;
    while (DMA1_Channel7->CCR & DMA_CCR_EN)
    {
        if (--timeout == 0)
            return -1; // BUSY / timeout
    }

    /* Копируем данные в буфер, из которого DMA будет читать */
    for (uint16_t i = 0; i < len; i++)
        uart_tx_buf[i] = (uint8_t)ptr[i];

    tx_len = len;

    /* Устанавливаем счётчик байт и очищаем старые флаги */
    DMA1_Channel7->CNDTR = tx_len;
    DMA1->IFCR = DMA_IFCR_CTCIF7; // очистим флаг Transfer Complete на всякий случай

    /* Запускаем канал */
    DMA1_Channel7->CCR |= DMA_CCR_EN;

    return len;
}
/* --- Прерывание DMA RX --- */
void DMA1_Channel6_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TCIF6)
    {
        /* Сброс флага */
        DMA1->IFCR = DMA_IFCR_CTCIF6;
        done = 1;
        /* Для circular mode можно ничего больше не делать, CNDTR уже перезарядится аппаратно */
    }
}

/* --- Прерывание DMA TX --- */
void DMA1_Channel7_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TCIF7)
    {
        /* Сброс флага */
        DMA1->IFCR = DMA_IFCR_CTCIF7;

        /* Остановим канал — безопасно для повторного запуска */
        DMA1_Channel7->CCR &= ~DMA_CCR_EN;

        /* При необходимости можно здесь очистить tx_len или сигнализировать о завершении */
        tx_len = 0;
    }
}