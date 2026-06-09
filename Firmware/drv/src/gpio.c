#include "gpio.h"

/*********************************************************************
*                                                                    *
* Function: PINMODE - CONFIGURES PINS TO BE INPUT OR OUTPUT          *
* @param pinNumber - which pin is configured, mode - INPUT or OUTPUT *
*                                                                    *
*********************************************************************/
void pinMode(uint8_t pinNumber, uint8_t mode) {
    if (mode == OUTPUT) {
        if (pinNumber < B0) {
            RCC->AHB1ENR |= (1 << 0);                                        // Enable clock for GPIO A (0 = GPIOAEN)
            GPIOA->MODER &= ~(3 << (2 * pinNumber));          // Clear MODER for pin 
            GPIOA->MODER |= (1 << (2 * pinNumber));           // Set MODER to 01 (output)
            GPIOA->OTYPER &= ~(1 << pinNumber);                                // Reset OTYPER to 0 (push-pull output)
        } else if (pinNumber < C0) {
            RCC->AHB1ENR |= (1 << 1);                        // Enable clock for GPIO B  (1 = GPIOBEN)
            GPIOB->MODER &= ~(3 << (2 * (pinNumber - B0)));  // Clear MODER for pin      (1)
            GPIOB->MODER |= (1 << (2 * (pinNumber - B0)));   // Set MODER to 01 (output) (1)
            GPIOB->OTYPER &= ~(1 << (pinNumber - B0));              // Reset OTYPER to 0 (push-pull output) (1)
        } else {
            RCC->AHB1ENR |= (1 << 2);                        // Enable clock for GPIO C  (2 = GPIOCEN)
            GPIOC->MODER &= ~(3 << (2 * (pinNumber - C0)));  // Clear MODER for pin      (1)
            GPIOC->MODER |= (1 << (2 * (pinNumber - C0)));   // Set MODER to 01 (output) (1)
            GPIOC->OTYPER &= ~(1 << (pinNumber - C0));              // Reset OTYPER to 0 (push-pull output) (1)
        }
    } else {
        if (pinNumber < B0) {
            RCC->AHB1ENR |= (1 << 0);                                        // Enable clock for GPIO A
            GPIOA->MODER &= ~(3 << (2 * pinNumber));          // Clear MODER for pin (set to input)
        } else if (pinNumber < C0) {
            RCC->AHB1ENR |= (1 << 1);                        // Enable clock for GPIO B
            GPIOB->MODER &= ~(3 << (2 * (pinNumber - B0)));  // Clear MODER for pin (set to input)  (1)
        } else {
            RCC->AHB1ENR |= (1 << 2);                        // Enable clock for GPIO C
            GPIOC->MODER &= ~(3 << (2 * (pinNumber - C0)));  // Clear MODER for pin (set to input)  (1)
        }
    }
}

/*****************************************************************************************************
*                                                                                                    *
* (1) - subtract from pinNumber to adjust the position of the registers, example B0 = 16, which      *
* should be on the 0 and 1 bits of the GPIOB MODER register, so you need to subtract 16 (B0)         *
* for B pins, and according to that, for GPIO C pins you need to subtract 32 (C0)                    *
*                                                                                                    *
*****************************************************************************************************/

/*********************************************************************
*                                                                    *
* Function: DIGITALWRITE - WRITES HIGH OR LOW VALUE TO A PIN         *
* @param pinNumber - target pin, value - HIGH or LOW state           *
*                                                                    *
*********************************************************************/
void digitalWrite(uint8_t pinNumber, _Bool value) {
    if (value == HIGH) {
        if (pinNumber < B0) {
            GPIOA->BSRR |= (1 << pinNumber);
        } else if (pinNumber < C0) {
            GPIOB->BSRR |= (1 << (pinNumber - B0));
        } else {
            GPIOC->BSRR |= (1 << (pinNumber - C0));
        }
    } else {
        if (pinNumber < B0) {
            GPIOA->BSRR |= (1 << (pinNumber + 16));
        } else if (pinNumber < C0) {
            GPIOB->BSRR |= (1 << (pinNumber + 16 - B0));
        } else {
            GPIOC->BSRR |= (1 << (pinNumber + 16 - C0));
        }
    }
}

/*********************************************************************
*                                                                    *
* Function: DIGITALREAD - READS THE CURRENT DIGITAL STATE OF A PIN   *
* @param pinNumber - target pin to read                              *
* @return _Bool - returns 1 (HIGH) or 0 (LOW)                        *
*                                                                    *
*********************************************************************/
_Bool digitalRead(uint8_t pinNumber) {
    _Bool retVal = 0;
    
    if (pinNumber < B0) {
        retVal = (GPIOA->IDR & (1 << pinNumber));
    } else if (pinNumber < C0) {
        retVal = (GPIOB->IDR & (1 << (pinNumber - B0)));
    } else {
        retVal = (GPIOC->IDR & (1 << (pinNumber - C0)));
    }
    
    return retVal;
}