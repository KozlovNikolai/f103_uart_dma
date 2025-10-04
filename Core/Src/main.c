#include "main.h"
#include "clock.h"
#include "uart_dma.h"
#include "string.h"

/* --- Возврат количества байт, принятых DMA --- */
uint16_t UART2_DMA_ReceivedLength(void)
{
    // RX_DATA_SIZE - сколько байт ещё DMA должен принять
    // => получено = всего - оставшиеся
    return RX_DATA_SIZE - DMA1_Channel6->CNDTR;
}

int value1 = 42; // Пример значения для value1
int value2 = 58; // Пример значения для value2

char uart_rx_buffer[128];            // Буфер для приёма данных по UART
volatile uint16_t uart_rx_index = 0; // Индекс для буфера
volatile uint8_t isNoDataTX = 0;     // Флаг для отправки сообщения о том, что нет данных

// Функция обработки полученной команды
void process_uart_command(void)
{
    if (strncmp(uart_rx_buffer, "get value1", uart_rx_index) == 0)
    {
        // Отправляем значение value1
        char msg[50];
        snprintf(msg, sizeof(msg), "value1 = %d\r\n", value1);
        printf(msg);
    }
    else if (strncmp(uart_rx_buffer, "get value2", uart_rx_index) == 0)
    {
        // Отправляем значение value2
        char msg[50];
        snprintf(msg, sizeof(msg), "value2 = %d\r\n", value2);
        printf(msg);
    }
    else if (strncmp(uart_rx_buffer, "get sum", uart_rx_index) == 0)
    {
        // Отправляем сумму value1 и value2
        int sum = value1 + value2;
        char msg[50];
        snprintf(msg, sizeof(msg), "sum = %d\r\n", sum);
        printf(msg);
    }
    else
    {
        // Если команда не распознана
        printf("Unknown command\r\n");
    }

    // После обработки очищаем буфер
    uart_rx_index = 0;
}

// Функция для получения данных по UART
void UART2_DMA_Process(void)
{

    uint16_t len = UART2_DMA_ReceivedLength();

    if (len > 0)
    {

        // printf(uart_rx_buffer);
        rc_data[len] = '\0'; // завершаем строку для удобства
        printf("RX[%d]: %s\r\n", len, rc_data);
        isNoDataTX = 0;
    }
    else
    {
        if (!isNoDataTX)
        {
            printf("NO DATA\r\n");
            isNoDataTX = 1;
        }
    }

    if (done)
    {
        done = 0; // Сбрасываем флаг завершения приема

        // Преобразуем полученные данные в строку (пока не выполняем разбор на слова)
        uart_rx_buffer[uart_rx_index] = '\0'; // Заканчиваем строку

        // process_uart_command();               // Обрабатываем полученную команду
    }
}
int main(void)
{
    if (SYSCLK == 72000000UL)
        SystemClock_72_Config(); // включаем 72 МГц

    // PINB_2_INIT();
    // TIM2_INIT();
    UART2_DMA_Init();

    // Blink(3);

    printf("START.\r\n");
    printf("SYSCLK = %d\r\n", (int)SYSCLK);

    while (1)
    {
        UART2_DMA_Process();
    }
}