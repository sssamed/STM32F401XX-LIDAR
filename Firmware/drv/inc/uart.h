/*********************************************************
* STM32F401CCU6 - BlackPill UART driver header file v2.0 *
* Author: Samed Sudzuka & Ilham Kozarevic                *
* Date: 07.01.2026                                       *
*********************************************************/

#ifndef STM32F401XC_UART_H
#define STM32F401XC_UART_H

#include "stm32f4xx.h"  // Device header

/***************************************************
*                  MACROS / DEFINES                *
****************************************************/

/* System clock & baudrate */
#define SYSCLK         16000000U
#define BAUDRATE       115200U

/* RCC peripheral enable bits */
#define GPIOA_EN       (1U << 0)
#define USART2_EN      (1U << 17)
#define USART1_EN      (1U << 4)

/* USART CR1 bits */
#define USART_UE       (1U << 13)
#define USART_TE       (1U << 3)
#define USART_RE       (1U << 2)

/* USART SR bits */
#define USART_TXE      (1U << 7)
#define USART_RXNE     (1U << 5)

/***************************************************
*                 PUBLIC FUNCTIONS                 *
****************************************************/

/* USART2 */
void UART2_Init(void);
void UART2_WriteChar(char c);
void UART2_WriteString(const char *str);
char UART2_ReadChar(void);
void UART2_ReadString(char *buffer, uint32_t maxLen);

/* USART1 */
void UART1_Init(void);
void UART1_WriteChar(char c);
void UART1_WriteString(const char *str);
char UART1_ReadChar(void);
void UART1_ReadString(char *buffer, uint32_t maxLen);

/* FORMAT - SPRINTF ALTERNATIVE */
static char* uint_to_str(uint32_t value, char* buf);
void uart_format(char* buf, uint32_t a, uint32_t b);

#endif    /* STM32F401XC_UART_H */
