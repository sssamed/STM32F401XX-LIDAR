#include "uart.h"

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
*                STATIC FUNCTIONS                  *
****************************************************/

static void UART2_SetBaudrate(uint32_t periph_clk, uint32_t baudrate);
static void UART1_SetBaudrate(uint32_t periph_clk, uint32_t baudrate);

/***************************************************
*                USART2 FUNCTIONS                  *
*                TX: PA2  RX: PA3                  *
****************************************************/

void UART2_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= GPIOA_EN;

    /* PA2 -> TX (AF) */
    GPIOA->MODER &= ~(3U << (2 * 2));
    GPIOA->MODER |=  (2U << (2 * 2));

    /* PA3 -> RX (AF) */
    GPIOA->MODER &= ~(3U << (2 * 3));
    GPIOA->MODER |=  (2U << (2 * 3));

    /* AF7 for USART2 */
    GPIOA->AFR[0] &= ~(0xF << (4 * 2));
    GPIOA->AFR[0] |=  (7U  << (4 * 2));

    GPIOA->AFR[0] &= ~(0xF << (4 * 3));
    GPIOA->AFR[0] |=  (7U  << (4 * 3));

    /* Enable USART2 clock */
    RCC->APB1ENR |= USART2_EN;

    /* Disable USART */
    USART2->CR1 &= ~USART_UE;

    /* Set baudrate */
    UART2_SetBaudrate(SYSCLK, BAUDRATE);

    /* Enable TX & RX */
    USART2->CR1 |= USART_TE | USART_RE;

    /* Enable USART */
    USART2->CR1 |= USART_UE;
}

void UART2_WriteChar(char c)
{
    while (!(USART2->SR & USART_TXE)) {}
    USART2->DR = c;
}

void UART2_WriteString(const char *str)
{
    while (*str)
    {
        UART2_WriteChar(*str++);
    }
}

char UART2_ReadChar(void)
{
    while (!(USART2->SR & USART_RXNE)) {}
    return (char)(USART2->DR);
}

void UART2_ReadString(char *buffer, uint32_t maxLen)
{
    uint32_t i = 0;
    char c;

    while (i < (maxLen - 1))
    {
        c = UART2_ReadChar();
        if (c == '\n' || c == '\r')
            break;
        buffer[i++] = c;
    }
    buffer[i] = '\0';
}

static void UART2_SetBaudrate(uint32_t periph_clk, uint32_t baudrate)
{
    USART2->BRR = (periph_clk + (baudrate / 2U)) / baudrate;
}

/***************************************************
*                USART1 FUNCTIONS                  *
*                TX: PA9  RX: PA10                 *
****************************************************/

void UART1_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= GPIOA_EN;

    /* PA9 -> TX (AF) */
    GPIOA->MODER &= ~(3U << (2 * 9));
    GPIOA->MODER |=  (2U << (2 * 9));

    /* PA10 -> RX (AF) */
    GPIOA->MODER &= ~(3U << (2 * 10));
    GPIOA->MODER |=  (2U << (2 * 10));

    /* AF7 for USART1 */
    GPIOA->AFR[1] &= ~(0xF << (4 * 1));
    GPIOA->AFR[1] |=  (7U  << (4 * 1));

    GPIOA->AFR[1] &= ~(0xF << (4 * 2));
    GPIOA->AFR[1] |=  (7U  << (4 * 2));

    /* Enable USART1 clock */
    RCC->APB2ENR |= USART1_EN;

    /* Disable USART */
    USART1->CR1 &= ~USART_UE;

    /* Set baudrate */
    UART1_SetBaudrate(SYSCLK, BAUDRATE);

    /* Enable TX & RX */
    USART1->CR1 |= USART_TE | USART_RE;

    /* Enable USART */
    USART1->CR1 |= USART_UE;
}

void UART1_WriteChar(char c)
{
    while (!(USART1->SR & USART_TXE)) {}
    USART1->DR = c;
}

void UART1_WriteString(const char *str)
{
    while (*str)
    {
        UART1_WriteChar(*str++);
    }
}

char UART1_ReadChar(void)
{
    while (!(USART1->SR & USART_RXNE)) {}
    return (char)(USART1->DR);
}

void UART1_ReadString(char *buffer, uint32_t maxLen)
{
    uint32_t i = 0;
    char c;

    while (i < (maxLen - 1))
    {
        c = UART1_ReadChar();
        if (c == '\n' || c == '\r')
            break;
        buffer[i++] = c;
    }
    buffer[i] = '\0';
}

static void UART1_SetBaudrate(uint32_t periph_clk, uint32_t baudrate)
{
    USART1->BRR = (periph_clk + (baudrate / 2U)) / baudrate;
}

/***************************************************
*          DATA FORMATTING FUNCTIONS               *
****************************************************/

static char* uint_to_str(uint32_t value, char* buf) {
    char temp[11];
    int i = 0;
    if (value == 0) {
        buf[0] = '0'; buf[1] = '\0';
        return buf;
    }
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }
    int j = 0;
    while (i > 0) buf[j++] = temp[--i];
    buf[j] = '\0';
    return buf;
}

void uart_format(char* buf, uint32_t a, uint32_t b) {
    char* p = buf;
    uint_to_str(a, p);
    while (*p) p++;
    *p++ = ',';
    uint_to_str(b, p);
    while (*p) p++;
    *p++ = '\r';
    *p++ = '\n';
    *p = '\0';
}

/**************************************************************************************************************
*
* (1) - USART peripheral configuration is based on the STM32F401xB/C reference manual RM0368,
*       Chapter 19 "Universal synchronous asynchronous receiver transmitter (USART)" and
*       Figure 167 "USART block diagram".
*
*       GPIO pins PA2/PA3 (USART2) and PA9/PA10 (USART1) are configured in Alternate Function mode (AF7)
*       to serve as TX and RX respectively. This connects the physical pins to the internal
*       USART transmitter and receiver blocks shown in the reference diagram.
*
*
* (2) - The USART clocks are enabled via the APB1 (USART2) and APB2 (USART1) buses by setting
*       the corresponding USARTEN bits in the RCC_APB1ENR / RCC_APB2ENR registers. Without these clocks,
*       the USART peripheral logic, including baud rate generation, transmitter and receiver
*       control units, would remain inactive.
*
*
* (3) - Before configuring the baud rate, the USARTs are disabled by clearing the UE (USART Enable)
*       bit in the USART_CR1 register. This follows the recommendation from the reference manual
*       to avoid unpredictable behavior while modifying configuration registers.
*
*
* (4) - Baud rate configuration is performed using the USART_BRR registers.
*       Oversampling by 16 is used (OVER8 = 0), therefore the baud rate divider is calculated as:
*
*           USARTDIV = fCK / baudrate
*
*       where:
*           fCK      -> APB1 (USART2) or APB2 (USART1) peripheral clock frequency (16 MHz)
*           baudrate -> desired baud rate (115200 bps)
*
*       The calculated USARTDIV value is written directly into the BRR register. The rounding
*       expression used in the code reduces baud rate error caused by integer division.
*
*
* (5) - The transmitters and receivers are enabled by setting the TE (Transmitter Enable) and
*       RE (Receiver Enable) bits in the USART_CR1 registers. These bits activate the internal
*       transmit and receive control blocks shown in the USART block diagram.
*
*
* (6) - Data transmission is performed by writing a byte to the USART_DR registers.
*       The TXE (Transmit Data Register Empty) flag in the USART_SR registers is continuously
*       polled to ensure that the transmit data register is ready to accept new data.
*
*       Once written, the hardware automatically transfers the data to the transmit shift
*       register and serializes it on the TX line, including start bit, data bits and stop bit.
*
*
* (7) - Data reception is handled by polling the RXNE (Receive Data Register Not Empty) flag
*       in the USART_SR registers. When RXNE is set, a complete byte has been received and
*       transferred from the receive shift register to the data register.
*
*       Reading the USART_DR registers returns the received byte and automatically clears
*       the RXNE flags, preparing the receivers for the next incoming data.
*
*
* (8) - This UART implementation enables full-duplex communication between the STM32
*       microcontroller and external devices (e.g., PC applications or other MCUs). It is suitable
*       for transmitting sensor measurement data to the PC and receiving control commands
*       from the PC to the microcontroller. Both UART1 and UART2 can operate independently,
*       allowing simultaneous serial communications on different interfaces.
*
**************************************************************************************************************/
