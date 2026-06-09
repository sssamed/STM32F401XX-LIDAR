/***************************************************************
* STM32F401CCU6 - BlackPill VL53L0X functions header file v1.0 *
* Author: Samed Sudzuka & Ilham Kozarevic								       *
* Date: 14.01.2026.																			       *
***************************************************************/

#include "i2c.h"
#include "vl53l0x_api.h"
#include "timing.h"

void VL53L0X_Init(VL53L0X_Dev_t* pDevice);
uint8_t VL53L0X_GetDistance_mm(VL53L0X_Dev_t* pDevice, VL53L0X_RangingMeasurementData_t* rangingData, uint16_t* distance);