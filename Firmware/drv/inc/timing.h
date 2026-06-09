/************************************************************
* STM32F401CCU6 - BlackPill delay function header file v1.0 *
* Author: Samed Sudzuka & Ilham Kozarevic								    *
* Date: 14.01.2026.																			    *
*************************************************************/

#ifndef stm32f4xc_delay_h

#define stm32f4xc_delay_h

#include "stm32f4xx.h"                  // Device header

void delay_ms(uint32_t ms); 
void SysTick_Handler(void);
void SysTick_Init(void);
uint32_t get_millis(void);

#endif