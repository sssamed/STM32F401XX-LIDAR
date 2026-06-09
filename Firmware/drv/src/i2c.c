#include "i2c.h"
#include "timing.h"

//I2C1 (PB6 SCL / PB7 SDA)

#define I2C_TIMEOUT_MS  2U

// -----------------------------------------------------------
// Helper utilities for timeouts and bus recovery
// -----------------------------------------------------------

/* Light-weight busy-wait used ONLY inside I2C1_RecoverBus.
 Why not delay_ms() here? delay_ms() blocks the CPU for milliseconds per call. 
 RecoverBus calls it ~23 times, freezing the MCU for ~23 ms — long enough to stall the stepper.
 The I2C spec (Sm mode) only requires tLOW/tHIGH = 4.7 us, so 10 us per clock phase 
 is perfectly legal and ~100x faster.
 At 16 MHz: 1 us approx 16 cycles. A 4-cycle loop body x 40 = 160 cycles approx 10 us.
*/
/*********************************************************************
* *
* Function: DELAY_10US - MICROSECOND BUSY WAIT DELAY          *
* *
*********************************************************************/
static void delay_10us(void)
{
    volatile uint32_t i = 40U;   // approx 10 us @ 16 MHz
    while (i--);
}

/*********************************************************************
* *
* Function: WAIT_SET - WAITS UNTIL A REGISTER BIT IS SET      *
* @param reg - pointer to register, mask - bit flag to check        *
* @return I2C_Status - I2C_OK on success, I2C_TIMEOUT on timeout    *
* *
*********************************************************************/
static I2C_Status wait_set(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t t0 = get_millis();
    while (!(*reg & mask)) {
        if ((get_millis() - t0) >= I2C_TIMEOUT_MS)
            return I2C_TIMEOUT;
    }
    return I2C_OK;
}

/*********************************************************************
* *
* Function: WAIT_CLEAR - WAITS UNTIL A REGISTER BIT CLEARS    *
* @param reg - pointer to register, mask - bit flag to check        *
* @return I2C_Status - I2C_OK on success, I2C_TIMEOUT on timeout    *
* *
*********************************************************************/
static I2C_Status wait_clear(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t t0 = get_millis();
    while (*reg & mask) {
        if ((get_millis() - t0) >= I2C_TIMEOUT_MS)
            return I2C_TIMEOUT;
    }
    return I2C_OK;
}

// Macro to check operation status; if failure occurs, trigger bus recovery automatically
#define TRY(expr)                         \
    do {                                 \
        I2C_Status _s = (expr);          \
        if (_s != I2C_OK) {              \
            I2C1_RecoverBus();           \
            return _s;                   \
        }                                \
    } while (0)

// ====================================================================
// I2C1_Init  (PB6 = SCL, PB7 = SDA)
// ====================================================================
/*********************************************************************
* *
* Function: I2C1_INIT - CONFIGURES AND ENABLES I2C1 SM MODE   *
* *
*********************************************************************/
void I2C1_Init(void)
{
    // Enable clock for GPIO 
    RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOBEN;

    // Reset + release I2C
    RCC->APB1RSTR |=  RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    // Enable I2C clock
    RCC->APB1ENR  |=  RCC_APB1ENR_I2C1EN;

    // -----------------------------------------------------------
    // GPIO configuration (PB6 & PB7)
    // -----------------------------------------------------------

    // Open-drain + pull-up + AF mode
    GPIOB->OTYPER |=  (1U << 6) | (1U << 7);            // open drain

    GPIOB->PUPDR  &= ~((3U << 12) | (3U << 14));        // clear bits
    GPIOB->PUPDR  |=  (1U << 12) | (1U << 14);         // pull-up

    GPIOB->MODER  &= ~((3U << 12) | (3U << 14));        // clear bits
    GPIOB->MODER  |=  (2U << 12) | (2U << 14);         // Alternate Function (AF) mode = 0b10

    GPIOB->AFR[0] &= ~((0xFU << 24) | (0xFU << 28));    // clear AFR[6] & AFR[7]
    GPIOB->AFR[0] |=  (4U   << 24) | (4U   << 28);     // AF4 = I2C1

    // -----------------------------------------------------------
    // I2C peripheral configuration
    // -----------------------------------------------------------

    // Software reset (sometimes needed)
    I2C1->CR1  = I2C_CR1_SWRST;
    I2C1->CR1  = 0;

    // Frequency (PCLK1 = 16 MHz)
    I2C1->CR2  = 16U;                                   // APB1 = 16 MHz

    // Standard mode 100 kHz
    I2C1->CCR  = 80U;                                   // 16MHz / (2 * 100kHz) = 80 - explained in (1)

    // Max rise time (Sm mode = 1000 ns)
    I2C1->TRISE = 17U;                                  // (1000ns / 62.5ns) + 1 = 17 - explained in (2)

    // Enable peripheral
    I2C1->CR1  |= I2C_CR1_PE;
}

// ====================================================================
// I2C1_RecoverBus
// ====================================================================
/*********************************************************************
* *
* Function: I2C1_RECOVERBUS - UNSTICKS CLOGGED I2C BUS LINES  *
* *
*********************************************************************/
void I2C1_RecoverBus(void)
{
    // Disable peripheral to control pins manually
    I2C1->CR1 &= ~I2C_CR1_PE;

    // Switch PB6/PB7 to GPIO output open-drain
    GPIOB->MODER  &= ~((3U << 12) | (3U << 14));        // clear bits
    GPIOB->MODER  |=  (1U << 12) | (1U << 14);         // general purpose output mode = 0b01
    GPIOB->OTYPER |=  (1U << 6)  | (1U << 7);           // open drain
    GPIOB->PUPDR  &= ~((3U << 12) | (3U << 14));        // clear bits
    GPIOB->PUPDR  |=  (1U << 12) | (1U << 14);         // pull-up

    // Both lines high = idle state
    GPIOB->BSRR = (1U << 6) | (1U << 7);
    delay_10us();

    // Up to 9 SCL pulses to flush the slave's shift register if stuck
    for (int i = 0; i < 9; i++) {
        GPIOB->BSRR = (1U << (6 + 16));                 // SCL low (bit 6 reset)
        delay_10us();
        GPIOB->BSRR = (1U << 6);                        // SCL high (bit 6 set)
        delay_10us();
        if (GPIOB->IDR & (1U << 7)) break;              // SDA released by slave — done recovering
    }

    // Manual STOP condition generation: SDA low -> SCL high -> SDA high
    GPIOB->BSRR = (1U << (7 + 16));                     // SDA low (bit 7 reset)
    delay_10us();
    GPIOB->BSRR = (1U << 6);                            // SCL high (bit 6 set)
    delay_10us();
    GPIOB->BSRR = (1U << 7);                            // SDA high (bit 7 set)
    delay_10us();

    // Restore Alternate Function mode on PB6/PB7
    GPIOB->MODER &= ~((3U << 12) | (3U << 14));        // clear bits
    GPIOB->MODER |=  (2U << 12) | (2U << 14);         // AF mode = 0b10

    // Re-initialise peripheral registers to safe state
    I2C1->CR1 = I2C_CR1_SWRST;
    delay_10us();
    I2C1->CR1  = 0;
    I2C1->CR2  = 16U;                                   // APB1 frequency = 16 MHz
    I2C1->CCR  = 80U;                                   // standard mode speed config
    I2C1->TRISE = 17U;                                  // rise time configuration
    I2C1->CR1  |= I2C_CR1_PE;                           // re-enable peripheral
    I2C1->CR1  |= I2C_CR1_ACK;                          // enable acknowledgment
}

// ====================================================================
// I2C1_ReadByte
// ====================================================================
/*********************************************************************
* *
* Function: I2C1_READBYTE - READS ONE BYTE FROM REGISTER      *
* @param slaveAddr - device address, regAddr - internal register    *
* @param data - pointer where the read byte will be saved           *
* @return I2C_Status - I2C_OK on success, error code on failure     *
* *
*********************************************************************/
I2C_Status I2C1_ReadByte(uint8_t slaveAddr, uint16_t regAddr, uint8_t *data)
{
    // Wait until bus not busy (BUSY flag in SR2 register)
    TRY(wait_clear((volatile uint32_t *)&I2C1->SR2, I2C_SR2_BUSY));

    // Generate start signal
    I2C1->CR1 |= I2C_CR1_START;
    // Wait for start bit in SR1 register (SB flag)
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_SB));

    // Transmit slave address with write bit (left-shifted)
    I2C1->DR = (uint8_t)(slaveAddr << 1);
    // Checks ADDR flag - confirmation that the slave responded
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_ADDR));
    (void)I2C1->SR2;                                    // reading SR2 clears the ADDR flag

    // Wait for transmitter register to empty (TXE flag)
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_TXE));
    if (regAddr > 0xFF) {
        I2C1->DR = (uint8_t)(regAddr >> 8);             // send first 8 bits of 16-bit address
        TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_TXE));
    }
    I2C1->DR = (uint8_t)(regAddr & 0xFF);               // put register/command address in data register
    // Wait until byte transfer finished (BTF flag)
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_BTF));

    // Generate repeated start signal without stopping to maintain control over the bus
    I2C1->CR1 |= I2C_CR1_START;
    // Wait for start bit in SR1 register (SB flag)
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_SB));

    // Transmit slave address again + Read bit enabled
    I2C1->DR = (uint8_t)((slaveAddr << 1) | 1U);
    // Checks ADDR flag - confirmation that the slave responded
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_ADDR));

    I2C1->CR1 &= ~I2C_CR1_ACK;                          // disable ACK because only one byte is being read
    (void)I2C1->SR2;                                    // clear ADDR flag
    I2C1->CR1 |= I2C_CR1_STOP;                          // set the stop bit in CR1 to 1

    // Wait until RxNE flag is set (data register not empty)
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_RXNE));
    *data = (uint8_t)I2C1->DR;                          // read data from data register

    I2C1->CR1 |= I2C_CR1_ACK;                           // re-enable ACK for future operations
    return I2C_OK;
}

// ====================================================================
// I2C1_ReadBurst
// ====================================================================
/*********************************************************************
* *
* Function: I2C1_READBURST - READS SEQUENTIAL BYTES FROM SLAVE *
* @param slaveAddr - device address, regAddr - starting register     *
* @param data - buffer array for received data, n - number of bytes  *
* @return I2C_Status - I2C_OK on success, error code on failure     *
* *
*********************************************************************/
I2C_Status I2C1_ReadBurst(uint8_t slaveAddr, uint16_t regAddr,
                           uint8_t *data, uint16_t n)
{
    // Wait until bus not busy
    TRY(wait_clear((volatile uint32_t *)&I2C1->SR2, I2C_SR2_BUSY));

    // Generate start
    I2C1->CR1 |= I2C_CR1_START;
    // Wait until start flag is set
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_SB));

    // Transmit slave address + Write
    I2C1->DR = (uint8_t)(slaveAddr << 1);
    // Wait until addr flag is set
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_ADDR));
    (void)I2C1->SR2;                                    // Clear addr flag

    // Wait until transmitter empty
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_TXE));
    if (regAddr > 0xFF) {
        I2C1->DR = (uint8_t)(regAddr >> 8);             // send first 8 bits
        TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_TXE));
        I2C1->DR = (uint8_t)(regAddr & 0xFF);           // send second 8 bits
    } else {
        I2C1->DR = (uint8_t)regAddr;                    // put register/command address in data register
    }
    // Wait until transfer finished
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_BTF));

    // Generate restart
    I2C1->CR1 |= I2C_CR1_START;
    // Wait until start flag is set
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_SB));

    // Transmit slave address + Read
    I2C1->DR = (uint8_t)((slaveAddr << 1) | 1U);
    // Wait until addr flag is set
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_ADDR));

    I2C1->CR1 |= I2C_CR1_ACK;                           // Enable Acknowledge
    (void)I2C1->SR2;                                    // Clear addr flag

    while (n > 0U) {
        // if last byte to read
        if (n == 1U) {
            I2C1->CR1 &= ~I2C_CR1_ACK;                  // Disable Acknowledge
            I2C1->CR1 |= I2C_CR1_STOP;                  // Generate Stop
        }
        // Wait for RXNE flag set
        TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_RXNE));
        *data++ = (uint8_t)I2C1->DR;                    // Read data from DR
        n--;
    }

    I2C1->CR1 |= I2C_CR1_ACK;                           // Ensure ACK is left enabled
    return I2C_OK;
}

// ====================================================================
// I2C1_WriteBurst
// ====================================================================
/*********************************************************************
* *
* Function: I2C1_WRITEBURST - SENDS SEQUENTIAL BYTES TO SLAVE *
* @param slaveAddr - device address, regAddr - target register      *
* @param data - data array to transmit, n - total number of bytes   *
* @return I2C_Status - I2C_OK on success, error code on failure     *
* *
*********************************************************************/
I2C_Status I2C1_WriteBurst(uint8_t slaveAddr, uint16_t regAddr,
                             uint8_t *data, uint16_t n)
{
    // Wait until bus not busy
    TRY(wait_clear((volatile uint32_t *)&I2C1->SR2, I2C_SR2_BUSY));

    // Generate start
    I2C1->CR1 |= I2C_CR1_START;
    // Wait until start flag is set
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_SB));

    // Transmit slave address
    I2C1->DR = (uint8_t)(slaveAddr << 1);
    // Wait until addr flag is set
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_ADDR));
    (void)I2C1->SR2;                                    // Clear addr flag

    // Wait until data register empty
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_TXE));
    // Send memory address
    if (regAddr > 0xFF) {
        I2C1->DR = (uint8_t)(regAddr >> 8);             // send first 8 bits
        TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_TXE));
        I2C1->DR = (uint8_t)(regAddr & 0xFF);           // send second 8 bits
    } else {
        I2C1->DR = (uint8_t)regAddr;                    // put register/command address in data register
    }

    for (uint16_t i = 0; i < n; i++) {
        // Wait until data register empty
        TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_TXE));
        I2C1->DR = *data++;                             // Transmit data byte
    }

    // Wait until transfer finished
    TRY(wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;                          // Generate stop
    return I2C_OK;
}

// ====================================================================
// I2C1_ScanBus
// ====================================================================
/*********************************************************************
* *
* Function: I2C1_SCANBUS - SEARCHES FOR ACTIVE ALIVE DEVICES  *
* @param slaveAddr - pointer where discovered address will be stored*
* *
*********************************************************************/
void I2C1_ScanBus(uint8_t *slaveAddr)
{
    // 0x08 and 0x77 are not reserved and they are usable addresses
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        // Generate START condition
        I2C1->CR1 |= I2C_CR1_START;
        if (wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_SB) != I2C_OK) {
            I2C1_RecoverBus(); continue;                // Trigger recovery loop if start fails
        }

        // Send address (left-shifted, write mode)
        I2C1->DR = (uint8_t)(addr << 1);
        uint32_t t0 = get_millis();
        while (!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF))) {
            if ((get_millis() - t0) >= I2C_TIMEOUT_MS) break;
        }

        if (I2C1->SR1 & I2C_SR1_ADDR) {
            // Device ACKed: match found
            *slaveAddr = addr;
            (void)I2C1->SR1; (void)I2C1->SR2;           // clear ADDR flag
            I2C1->CR1 |= I2C_CR1_STOP;                  // Generate STOP
        } else {
            // No ACK, clear AF flag
            I2C1->SR1 &= ~I2C_SR1_AF;
            I2C1->CR1 |= I2C_CR1_STOP;                  // Generate STOP
        }
        delay_10us(); delay_10us();                     // small gap between probes to let the bus breathe
    }
}

/*
 For EEPROM, ACK polling is used to determine if the EEPROM is ready to use
*/
/*********************************************************************
* *
* Function: I2C1_ACKPOLLING - LOOPS UNTIL SLAVE RESPONDS READY*
* @param slaveAddr - address of the busy device to poll             *
* *
*********************************************************************/
void I2C1_AckPolling(uint8_t slaveAddr)
{
    while (1) {
        // 1. Generate START
        I2C1->CR1 |= I2C_CR1_START;
        if (wait_set((volatile uint32_t *)&I2C1->SR1, I2C_SR1_SB) != I2C_OK) {
            I2C1_RecoverBus(); continue;
        }

        // 2. Send slave address (shifted)
        I2C1->DR = (uint8_t)(slaveAddr << 1);

        // 3. Wait for ADDR (Success) or AF (Failure)
        uint32_t t0 = get_millis();
        while (!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF))) {
            if ((get_millis() - t0) >= I2C_TIMEOUT_MS) break;
        }

        if (I2C1->SR1 & I2C_SR1_ADDR) {
            // SUCCESS: Slave responded
            (void)I2C1->SR2;                            // Clear ADDR flag
            I2C1->CR1 |= I2C_CR1_STOP;                  // Generate STOP
            return;
        } else {
            // FAILURE: Slave NACKed
            I2C1->SR1 &= ~I2C_SR1_AF;                   // Clear the NACK flag
            I2C1->CR1 |= I2C_CR1_STOP;                  // Generate STOP
            delay_10us(); delay_10us();                 // clean safe pause gap instead of a blocking delay_ms
        }
    }
}

// I2C2 (PB10/PB3) TODO implement I2C2
/*********************************************************************
* *
* Function: I2C2_INIT - ENABLES CLOCK FOR I2C2 PERIPHERAL     *
* *
*********************************************************************/
void I2C2_Init(void) { 
    RCC->APB1ENR |= (1U << 22); // I2C2EN is the 22nd bit of the RCC_APB1ENR register
}

// I2C3 (PA8/PB4) TODO implement I2C3
/*********************************************************************
* *
* Function: I2C3_INIT - ENABLES CLOCK FOR I2C3 PERIPHERAL     *
* *
*********************************************************************/
void I2C3_Init(void) { 
    RCC->APB1ENR |= (1U << 23); // I2C3EN is the 23rd bit of the RCC_APB1ENR register
}

/**************************************************************************************************************
* *
* (1) - From the STM32F401xB/C reference manual RM0368, page 503, we can see that 12 bits are reserved        *
* for CCR (clock control), and they represent the CCR value which determines the SCL frequency.              *
* In standard mode, we can see that Thigh = Tlow = T = CCR * TPCLK, which means that the duty cycle           *
* of the I2C serial clock (SCL) signal is 50%.                                                                *
* This means that the full SCL period TSCL = Tlow + Thigh = 2T.                                               *
* 
* TPCLK -> full GPIO port clock time period, TPCLK = 1 / fPCLK = 1 / 16Mhz                                    *
* 
* Tlow -> SCL clock low time, Tlow = CCR * TPCLK      // Both are equal                                       *
* Thigh -> SCL clock high time, Thigh = CCR * TPCLK       // to each other                                    *
* 
* Tlow = Thigh = T -> since Tlow and Thigh are equal, they will be T, the SCL clock half period               *
* *
* TSCL -> full SCL clock time period, TSCL = Tlow + Thigh = 2 * T                                             *
* In Sm - standard mode (not Fm, the fast mode), we want the SCL clock frequency to be 100 kHz.               *
* So we know that TSCL = 1 / fSCL, which means that TSCL = 1 / 100kHz.                                        *
* 
* T = TSCL / 2                                                                                                *
* 
* Knowing this, we can write the following expression:                                                        *
* 
* TSCL / 2 = CCR * TPCLK                                                                                      *
* 
* 1 / (2 * fSCL) = CCR * (1 / fPCLK)                                                                          *
* 
* Transforming this expression, we get the formula for the value for CCR:                                    *
* 
* CCR = fPCLK / (2 * fSCL)                                                                                    *
* 
* After plugging in the GPIO port frequency of 16 MHz and I2C SCL frequency of 100 kHz we have:               *
* 
* CCR = 16 MHz / (2 * 100 kHz) = 16 000 kHz / 200 kHz = 80                                                    *
* 
* The calculated CCR value for GPIO at 16 MHz and SCL at 100 kHz is 80.                                       *
* 
* (2) - The reference manual, page 504, states that the maximum SCL rise time in Sm mode is 1000ns.           *
* This needs to be divided by TPCLK (1) and the value of the TRISE register is the ratio incremented         *
* by 1.                                                                                                       *
* 
* For example, TPCLK = 1 / 16 MHz = 62.5 ns.                                                                  *
* Max. SCL rise time for Sm mode = 1000 ns.                                                                   *
* 
* TRISE = (1000 ns / 62.5 ns) + 1 = 16 + 1 = 17                                                               *
* 
* The calculated TRISE value for GPIO at 16 MHz in Sm mode is 17                                              *
* 
**************************************************************************************************************/