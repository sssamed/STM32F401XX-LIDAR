#include "stm32f4xx.h"                  // Device header
#include "uart.h"
#include <stdlib.h>

char rx_char;
char str_buffer[64];
uint32_t loop_count = 0;

int main(void)
{
    // USART2 (PA2 = TX, PA3 = RX), 115200 Baud
    UART2_Init();
    
    UART2_WriteString("--- UART2 USB-to-TTL Test ---\r\n");
    UART2_WriteString("Type any character to test echo functionality.\r\n\r\n");

    while (1)
    {
        // Slanje primljenih karaktera
        rx_char = UART2_ReadChar();
        UART2_WriteString("Received: ");
        UART2_WriteChar(rx_char);
        UART2_WriteString("\r\n");
        
        // Test formata
        loop_count++;
        uart_format(str_buffer, loop_count, (uint32_t)(rx_char));
        UART2_WriteString("Formatted buffer data: ");
        UART2_WriteString(str_buffer);
        UART2_WriteString("\r\n");
    }
}