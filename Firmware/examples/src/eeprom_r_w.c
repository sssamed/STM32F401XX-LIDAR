#include "stm32f4xx.h"                  // Device header
#include "gpio.h"
#include "uart.h"
#include "i2c.h"
#include <stdlib.h>

uint8_t writeVal = 0xAA;
uint8_t* data;
uint8_t value;

int main(void) 
{
    I2C1_Init();
    
    // allocate 32 8-bit memory locations in the heap for data
    data = (uint8_t*) malloc(32);
    
    // write 0xAA in location 0x0010 to EEPROM
    I2C1_WriteBurst(0x50, 0x0010, &writeVal, 1);
    
    // poll so EEPROM has time to write
    I2C1_AckPolling(0x50);
    
    while (1) 
    {
        // read the data and store it in allocated heap memory (data)
        I2C1_ReadBurst(0x50, 0x0010, data, 1);
        
        // place data value in "value", so it can be seen in Watch
        value = *data;
    }
}
