/*********************************************************
* STM32F401CCU6 - BlackPill GPIO driver header file v1.0 *
* Author: Samed Sudzuka & Ilham Kozarevic								 *
* Date: 16.11.2025.																			 *
*********************************************************/

#ifndef stm32f401xc_gpio_h
#define stm32f401xc_gpio_h

/***************************************************
*
*GENERIC MACROS
*
****************************************************/
//-------------------MODE----------------------------

#define INPUT  (uint8_t) 0x00
#define OUTPUT (uint8_t) 0x01

//----------------------------------------------------

//----------------------VALUE-------------------------
#define HIGH (uint8_t) 0x01
#define LOW  (uint8_t) 0x00
//----------------------------------------------------

//--------------------PORT A PINS---------------------

#define A0  (uint8_t) 0x00
#define A1  (uint8_t) 0x01
#define A2  (uint8_t) 0x02
#define A3  (uint8_t) 0x03
#define A4  (uint8_t) 0x04
#define A5  (uint8_t) 0x05
#define A6  (uint8_t) 0x06
#define A7  (uint8_t) 0x07
#define A8  (uint8_t) 0x08
#define A9  (uint8_t) 0x09
#define A10 (uint8_t) 0x0a
#define A11 (uint8_t) 0x0b
#define A12 (uint8_t) 0x0c
#define A13 (uint8_t) 0x0d
#define A14 (uint8_t) 0x0e
#define A15 (uint8_t) 0x0f

//--------------------PORT B PINS---------------------

#define B0  (uint8_t) 0x10
#define B1  (uint8_t) 0x11
#define B2  (uint8_t) 0x12
#define B3  (uint8_t) 0x13
#define B4  (uint8_t) 0x14
#define B5  (uint8_t) 0x15
#define B6  (uint8_t) 0x16
#define B7  (uint8_t) 0x17
#define B8  (uint8_t) 0x18
#define B9  (uint8_t) 0x19
#define B10 (uint8_t) 0x1a
#define B11 (uint8_t) 0x1b
#define B12 (uint8_t) 0x1c
#define B13 (uint8_t) 0x1d
#define B14 (uint8_t) 0x1e
#define B15 (uint8_t) 0x1f

//--------------------PORT C PINS---------------------

#define C0  (uint8_t) 0x20
#define C1  (uint8_t) 0x21
#define C2  (uint8_t) 0x22
#define C3  (uint8_t) 0x23
#define C4  (uint8_t) 0x24
#define C5  (uint8_t) 0x25
#define C6  (uint8_t) 0x26
#define C7  (uint8_t) 0x27
#define C8  (uint8_t) 0x28
#define C9  (uint8_t) 0x29
#define C10 (uint8_t) 0x2a
#define C11 (uint8_t) 0x2b
#define C12 (uint8_t) 0x2c
#define C13 (uint8_t) 0x2d
#define C14 (uint8_t) 0x2e
#define C15 (uint8_t) 0x2f

#include "stm32f4xx.h"                  // Device header

//-------------------------------------------
//Input Output Declaration
//-------------------------------------------


	void pinMode(uint8_t pinNumber, uint8_t mode); 			
	
	
//-------------------------------------------
//Port - Pin Read Write
//-------------------------------------------


	void digitalWrite(uint8_t pinNumber, _Bool value);
	_Bool digitalRead(uint8_t pinNumber);	


#endif