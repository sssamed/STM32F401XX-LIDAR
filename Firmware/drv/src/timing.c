#include "timing.h"

#include "stm32f4xx.h"                  // Device header

volatile uint32_t msTicks = 0; 

void delay_ms(uint32_t ms) {
	
    // Enable TIM5 clock (Bit 3 in APB1ENR)
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;

    // Set Prescaler
    // If your APB1 Timer Clock is 84MHz, setting PSC to 84000-1 
    // gives you a 1kHz timer clock (1ms per tick).
    TIM5->PSC = (SystemCoreClock / 1000) - 1; 

    // Set Auto-Reload to the number of ms requested
    TIM5->ARR = ms;
    
    // Reset counter and clear flags
    TIM5->CNT = 0;
    TIM5->SR &= ~TIM_SR_UIF; 

    // Start the timer
    TIM5->CR1 |= TIM_CR1_CEN;

    // Wait until the Update Interrupt Flag is set
    while (!(TIM5->SR & TIM_SR_UIF));

    // Stop the timer
    TIM5->CR1 &= ~TIM_CR1_CEN;
}

// ISR running once every 1ms
void SysTick_Handler(void) {
    msTicks++;
}

// SysTick timer init
void SysTick_Init(void) {
    // Set SysTick to 1ms interval
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1);
    }
}

uint32_t get_millis(void) {
    return msTicks;
}