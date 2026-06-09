/************************************************************
* STM32F401CCU6 - BlackPill stepper driver header file v1.0 *
* Author: Samed Sudzuka & Ilham Kozarevic								    *
* Date: 16.04.2026.																			    *
************************************************************/

#ifndef TB6612FNG_Stepper_h

#define TB6612FNG_Stepper_h

#include "stm32f4xx.h"                  // Device header

typedef struct {
	uint8_t AIN1;
	uint8_t AIN2;
	uint8_t BIN1;
	uint8_t BIN2;
} TB6612FNG_Stepper_t;

typedef struct {
	uint8_t IN1;
	uint8_t IN2;
	uint8_t IN3;
	uint8_t IN4;
} ULN2003_Stepper_t;

void TB6612FNG_Stepper_Init(TB6612FNG_Stepper_t* stepper);
void TB6612FNG_Stepper_Step(TB6612FNG_Stepper_t* stepper, uint8_t step);

void ULN2003_Stepper_Init(ULN2003_Stepper_t* stepper);
void ULN2003_Stepper_Step(ULN2003_Stepper_t* stepper, uint8_t step);

#endif