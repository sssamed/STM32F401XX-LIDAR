#include "stm32f4xx.h"
#include "gpio.h"
#include "uart.h"
#include "i2c.h"
#include "timing.h"
#include "VL53L0X.h"
#include "stepper.h"

/* steps/rev = 2048 × (15/54) ˜ 567 */
 
#define STEPPER_NATIVE_STEPS_PER_REV 2048.0f
#define GEAR_RATIO (1.0f / 3.6f)
#define STEPS_PER_REV (STEPPER_NATIVE_STEPS_PER_REV * GEAR_RATIO) // = 568.888...

/* Step period in ms — set by TIM2 */
#define STEP_PERIOD_MS    2U

/* Poll sensor every 15 ms */
#define SENSOR_POLL_MS   15U

volatile uint8_t  step       = 1;
volatile uint32_t step_count = 0;

uint16_t distance  = 0;
float    angle     = 0.0f;

VL53L0X_Dev_t sensor;
VL53L0X_RangingMeasurementData_t rangingData;
ULN2003_Stepper_t stepper = {A4, A5, A6, A7};
char uartBuf[32];

/* TIM2 — Hardware stepper timer */
static void Stepper_Timer_Init(void)
{
    /* Enable TIM2 clock (APB1) */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /*
     * Timer clock = APB1 = 16 MHz 
     * Prescaler: 16 MHz / 16000 = 1000 Hz ? 1 ms per tick.
     */
    TIM2->PSC  = (SystemCoreClock / 1000U) - 1U;
    TIM2->ARR  = STEP_PERIOD_MS - 1U;
    TIM2->CNT  = 0U;
    TIM2->SR   = 0U;                  /* clear any pending flags */
    TIM2->DIER |= TIM_DIER_UIE;       /* enable update (overflow) interrupt */
    TIM2->CR1  |= TIM_CR1_CEN;        /* start timer */

    NVIC_SetPriority(TIM2_IRQn, 1U);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/* TIM2_IRQHandler — fires every STEP_PERIOD_MS */
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;   /* must clear flag or ISR fires again */

        ULN2003_Stepper_Step(&stepper, (uint8_t)step);

        if (++step > 4U) step = 1U;
        if (++step_count >= (uint32_t)(STEPS_PER_REV + 0.5f)) step_count = 0;
    }
}

int main(void)
{
    SysTick_Init();
    UART2_Init();

    VL53L0X_Init(&sensor);
    ULN2003_Stepper_Init(&stepper);

    /* soft-start ramp using blocking delay */
    uint32_t ramp_delay = 12U;
    for (int i = 0; i < 500; i++) {
        ULN2003_Stepper_Step(&stepper, (uint8_t)step);
        if (++step > 4U) step = 1U;
        delay_ms(ramp_delay);
        if (i % 5 == 0 && ramp_delay > STEP_PERIOD_MS) ramp_delay--;
    }

    Stepper_Timer_Init();

    uint32_t last_sensor_poll = get_millis();
		
		/* MAIN LOOP */
    while (1)
    {
        uint32_t now = get_millis();

        if (now - last_sensor_poll >= SENSOR_POLL_MS) {

            uint8_t result = VL53L0X_GetDistance_mm(&sensor, &rangingData, &distance);

            if (result == 1) {
                /* Snapshot step_count atomically */
                __disable_irq();
                uint32_t sc = step_count;
                __enable_irq();

                angle = 360.0f - ((float)step_count * 360.0f) / STEPS_PER_REV;

                uart_format(uartBuf, (uint32_t)angle, (uint32_t)distance);
                UART2_WriteString(uartBuf);
            }
            /* result == 0: not ready yet, normal    */
            /* result == 2: I2C error, bus recovered */

            last_sensor_poll = now;
        }
    }
}