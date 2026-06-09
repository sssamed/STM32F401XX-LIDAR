#include "stm32f4xx.h"                  // Device header
#include "VL53L0X.h"

uint16_t distance;

VL53L0X_Dev_t sensor;
VL53L0X_RangingMeasurementData_t rangingData;

int main(void) {
	VL53L0X_Init(&sensor);
	
	while (1) {
		VL53L0X_GetDistance_mm(&sensor, rangingData, &distance);
	}
}