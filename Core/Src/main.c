#include "main.h"
#include "clock.h"
#include "uart_dma.h"

int main(void)
{
    if (SYSCLK == 72000000UL)
        SystemClock_72_Config(); // включаем 72 МГц

    UART2_DMA_Init();

    printf("Hello from DMA");

    while (1)
    {
        while (!done)
            ;
        printf("Received data:\"%s\"\r\n", rc_data);
        done = 0;
    }
}