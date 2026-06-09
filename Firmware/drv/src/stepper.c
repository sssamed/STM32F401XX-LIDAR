#include "stepper.h"
#include "gpio.h"

void TB6612FNG_Stepper_Init(TB6612FNG_Stepper_t* stepper) 
{
	pinMode(stepper->AIN1, OUTPUT);
	pinMode(stepper->AIN2, OUTPUT);
	pinMode(stepper->BIN1, OUTPUT);
	pinMode(stepper->BIN2, OUTPUT);
}

void TB6612FNG_Stepper_Step(TB6612FNG_Stepper_t* stepper, uint8_t step) 
{
	switch (step) {
		case 1:
			digitalWrite(stepper->AIN1, HIGH);
			digitalWrite(stepper->AIN2, LOW);
		
			digitalWrite(stepper->BIN1, HIGH);
			digitalWrite(stepper->BIN2, LOW);
			break;
		case 2:
			digitalWrite(stepper->AIN1, LOW);
			digitalWrite(stepper->AIN2, HIGH);
		
			digitalWrite(stepper->BIN1, HIGH);
			digitalWrite(stepper->BIN2, LOW);
			break;
		case 3:
			digitalWrite(stepper->AIN1, LOW);
			digitalWrite(stepper->AIN2, HIGH);
		
			digitalWrite(stepper->BIN1, LOW);
			digitalWrite(stepper->BIN2, HIGH);
			break;
		case 4:
			digitalWrite(stepper->AIN1, HIGH);
			digitalWrite(stepper->AIN2, LOW);
		
			digitalWrite(stepper->BIN1, LOW);
			digitalWrite(stepper->BIN2, HIGH);
			break;
	}
}

void ULN2003_Stepper_Init(ULN2003_Stepper_t* stepper) 
{
    pinMode(stepper->IN1, OUTPUT);
    pinMode(stepper->IN2, OUTPUT);
    pinMode(stepper->IN3, OUTPUT);
    pinMode(stepper->IN4, OUTPUT);
}

void ULN2003_Stepper_Step(ULN2003_Stepper_t* stepper, uint8_t step) 
{
    switch (step) {
        case 1:
            digitalWrite(stepper->IN1, HIGH);
            digitalWrite(stepper->IN2, LOW);
            digitalWrite(stepper->IN3, LOW);
            digitalWrite(stepper->IN4, LOW);
            break;
        case 2:
            digitalWrite(stepper->IN1, LOW);
            digitalWrite(stepper->IN2, HIGH);
            digitalWrite(stepper->IN3, LOW);
            digitalWrite(stepper->IN4, LOW);
            break;
        case 3:
            digitalWrite(stepper->IN1, LOW);
            digitalWrite(stepper->IN2, LOW);
            digitalWrite(stepper->IN3, HIGH);
            digitalWrite(stepper->IN4, LOW);
            break;
        case 4:
            digitalWrite(stepper->IN1, LOW);
            digitalWrite(stepper->IN2, LOW);
            digitalWrite(stepper->IN3, LOW);
            digitalWrite(stepper->IN4, HIGH);
            break;
    }
}

