#include "stm32f1xx.h"
#include "spi.h"

void st7789_spi_init(void)
{
    /* --- Тактирование портов и SPI --- */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // GPIOC
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // GPIOB
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // Альтернативные функции
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // SPI1

    /* --- Настройка SPI пинов --- */
    // PA5 = SCK, PA7 = MOSI
    // AF Push-Pull, 50 MHz
    GPIOA->CRL &= ~(
        GPIO_CRL_MODE5 | GPIO_CRL_CNF5 |
        GPIO_CRL_MODE7 | GPIO_CRL_CNF7);

    GPIOA->CRL |=
        (GPIO_CRL_MODE5_1 | GPIO_CRL_MODE5_0 | GPIO_CRL_CNF5_1) | // PA5: SCK AF PP 50MHz
        (GPIO_CRL_MODE7_1 | GPIO_CRL_MODE7_0 | GPIO_CRL_CNF7_1);  // PA7: MOSI AF PP 50MHz

    /* --- DC (PA6) и RST (PA4) как выходы --- */
    GPIOA->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4 |
                    GPIO_CRL_MODE6 | GPIO_CRL_CNF6);
    GPIOA->CRL |=
        (GPIO_CRL_MODE4_1 | GPIO_CRL_MODE4_0) | // PA4 = Output 50 MHz
        (GPIO_CRL_MODE6_1 | GPIO_CRL_MODE6_0);  // PA6 = Output 50 MHz

    /* --- CS (PC13) как выход --- */
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0); // Output 50 MHz Push-Pull

    /* --- BLK (PB0) как выход --- */
    GPIOB->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
    GPIOB->CRL |= (GPIO_CRL_MODE0_1 | GPIO_CRL_MODE0_0); // Output 50 MHz Push-Pull

    /* --- Настройка SPI1 --- */
    SPI1->CR1 = 0;                            // сброс конфигурации
    SPI1->CR1 |= SPI_CR1_MSTR;                // мастер
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;   // программное управление NSS
    SPI1->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA; // режим 3
    SPI1->CR1 |= SPI_CR1_BR_1;                // делитель Fpclk/8 (чтобы не слишком быстро)
    SPI1->CR1 |= SPI_CR1_SPE;                 // включаем SPI
}

/* --- Простая передача по SPI --- */
void st7789_spi_transmit(uint8_t *data, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        while (!(SPI1->SR & SPI_SR_TXE))
            ;               // ждем освобождения буфера
        SPI1->DR = data[i]; // отправляем байт
    }

    while (SPI1->SR & SPI_SR_BSY)
        ; // ждем завершения передачи

    (void)SPI1->DR; // сброс OVR
    (void)SPI1->SR;
}
