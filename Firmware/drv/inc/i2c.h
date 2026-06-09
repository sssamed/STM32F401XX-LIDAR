/*********************************************************
* STM32F401CCU6 - BlackPill I2C driver header file v1.0  *
* Author: Samed Sudzuka & Ilham Kozarevic								 *
* Date: 20.04.2026.																			 *
*********************************************************/

#ifndef stm32f4xx_i2c_h
#define stm32f4xx_i2c_h

#include "stm32f4xx.h"
#include <stdint.h>

/* ------------------------------------------------------------------
 *  Return codes for all I2C operations.
 * ------------------------------------------------------------------ */
typedef enum {
    I2C_OK      = 0,
    I2C_TIMEOUT = 1,   // A flag never arrived — bus/slave stuck
    I2C_NACK    = 2,   // Slave did not acknowledge its address
} I2C_Status;

/* ------------------------------------------------------------------
 *  Core driver API
 * ------------------------------------------------------------------ */
void       I2C1_Init(void);

I2C_Status I2C1_ReadByte (uint8_t slaveAddr, uint16_t regAddr, uint8_t *data);
I2C_Status I2C1_ReadBurst(uint8_t slaveAddr, uint16_t regAddr, uint8_t *data, uint16_t n);
I2C_Status I2C1_WriteBurst(uint8_t slaveAddr, uint16_t regAddr, uint8_t *data, uint16_t n);

void I2C1_RecoverBus(void);

void I2C1_ScanBus   (uint8_t *slaveAddr);
void I2C1_AckPolling(uint8_t  slaveAddr);

void I2C2_Init(void);
void I2C3_Init(void);

#endif /* I2C_H */