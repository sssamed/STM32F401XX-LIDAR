#include "VL53L0X.h"
#include "i2c.h"
#include "timing.h"

/* Maximum time to wait for a new ranging measurement */
#define VL53L0X_READY_TIMEOUT_MS  100U

void VL53L0X_Init(VL53L0X_Dev_t* pDevice) {
	I2C1_Init();
	
	pDevice->I2cDevAddr = 0x52; // default write address
	
	VL53L0X_DataInit(pDevice);
	VL53L0X_StaticInit(pDevice);
	
	// Lower signal rate limit (allows weaker reflections)
	VL53L0X_SetLimitCheckEnable(pDevice,
    VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);

	VL53L0X_SetLimitCheckValue(pDevice,
    VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
    (FixPoint1616_t)(0.1 * 65536));

	// Increase laser pulse periods
	VL53L0X_SetVcselPulsePeriod(pDevice,
    VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);

	VL53L0X_SetVcselPulsePeriod(pDevice,
    VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);

	// Increase timing budget (in microseconds)
	VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pDevice, 10000);
	
	uint32_t refSpadCount;
	uint8_t isApertureSpads;
	VL53L0X_PerformRefSpadManagement(pDevice, &refSpadCount, &isApertureSpads);
	
	uint8_t vhvSettings;
	uint8_t phaseCal;
	VL53L0X_PerformRefCalibration(pDevice, &vhvSettings, &phaseCal);
	
	VL53L0X_SetDeviceMode(pDevice, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
	
	VL53L0X_StartMeasurement(pDevice);
}

uint8_t VL53L0X_GetDistance_mm(VL53L0X_Dev_t* pDevice, VL53L0X_RangingMeasurementData_t* pRangingData, uint16_t* distance)
{
    uint8_t  newDataReady = 0;
    uint32_t t0           = get_millis();
 
    while (newDataReady == 0) {
        VL53L0X_GetMeasurementDataReady(pDevice, &newDataReady);
 
        if ((get_millis() - t0) >= VL53L0X_READY_TIMEOUT_MS) {
            /* Sensor stopped responding, try to recover the bus and
             * report failure so main() can decide what to do next */
            I2C1_RecoverBus();
            return 0;   /* failure */
        }
 
        delay_ms(2);
    }
 
    VL53L0X_GetRangingMeasurementData(pDevice, pRangingData);
    *distance = pRangingData->RangeMilliMeter;
    VL53L0X_ClearInterruptMask(pDevice, 0);
 
    return 1;   /* success */
}