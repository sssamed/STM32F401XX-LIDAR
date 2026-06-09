# STM32F401XX-LIDAR
Bare-metal STM32F401xx implementation of a LIDAR scanner. Features a real-time Python visualizer app, custom bare-metal I2C VL53L0X ToF sensor driver, bare metal GPIO and UART drivers, as well as a 28BYJ-48 stepper motor controlled by a ULN2003.

## Project Architecture
The system captures distance data across a 180/360-degree plane by rotating the ToF sensor using a stepper motor, streaming coordinates over UART to a desktop application for plotting.

* **Microcontroller:** STM32F401CCU6 (ARM Cortex-M4) programmed completely at the register level (no HAL/LL libraries).
* **Peripherals Used:**
    * `I2C1` (PB6/PB7) - Sensor data acquisition with custom hardware bus recovery.
    * `USART2` (PA2/PA3) - Telemetry data transmission to PC (115200 baud).
    * `GPIO` - 4-wire step sequence output for motor control.
* **Necessary Hardware**:
  * USB-to-TTL Bridge - To receive UART data on PC.
  * VL53L0X ToF Sensor - To measure distance.
  * 28BYJ-48 Stepper - Rotates the sensor to measure radially.
  * ULN2003 Darlington Array - Stepper motor driver.
  * Slip Ring Connector - Enables the rotation of the sensor without twisting wires.

## Hardware Wiring Reference

| Component | Pin | Peripheral Function |
| :--- | :--- | :--- |
| **VL53L0X SDA** | PB7 | I2C1 Data (Open-Drain, Internal Pull-Up) |
| **VL53L0X SCL** | PB6 | I2C1 Clock (Open-Drain, Internal Pull-Up) |
| **ULN2003 IN1-IN4**| PA4 - PA7 | GPIO Outputs (Motor Phasing) |
| **USB-to-TTL RX** | PA2 | USART2 TX |
| **USB-to-TTL TX** | PA3 | USART2 RX |

## Repository Structure
* `/Firmware` - Complete Keil uVision project containing register-level C code.
    * `/drv` - GPIO, I2C and UART drivers and VL53L0X wrapper.
      * `src/i2c.c` / `inc/i2c.h` - I2C driver with timeout handling, ACK polling, and 16-bit address support.
      * `src/uart.c` / `inc/uart.h` - Non-blocking character/string transmission and custom string formatting modules.
      * `src/vl53l0x.c` - Core Time-of-Flight initialization and continuous ranging loop.
* `/Visualizer` - Python desktop application (`matplotlib` / `pyserial`) for real-time polar plotting.

## How to Run

**NOTE:** Python, Keil uVision and STM32F401xx DFP 2.17.1 (strictly not newer), and CH340 or CP210x drivers for a USB-to-TTL module are mandatory.

**INSTALL LINKS:**
* [Install Python 3.14.5](https://www.python.org/ftp/python/3.14.5/python-3.14.5-amd64.exe)
* [Install Keil uVision MDK543a](https://armkeil.blob.core.windows.net/eval/MDK543a.exe)
* [Install STM32F4xx DFP 2.17.1](https://keilpack.azureedge.net/pack/Keil.STM32F4xx_DFP.2.17.1.pack)
* [Install CH340 Driver](https://sparks.gogo.co.nz/assets/_site_/downloads/CH34x_Install_Windows_v3_4.zip) or [Install CP210x Driver](https://www.silabs.com/documents/public/software/CP210x_Universal_Windows_Driver.zip)

### 1. Firmware Flash
1. Open the project inside **Keil uVision**.
2. Build the project (`F7`) and flash (`F8`) it to the STM32F401 target using an ST-Link debugger.

### 2. Launch Visualizer

1. Connect the USB-to-TTL module to the PC.
2. Install the necessary Python packages:
   ```bash
   pip install pyserial matplotlib
   ```
3. Start the Python application
   ```bash
   python lidar-visualizer.py
   ```
