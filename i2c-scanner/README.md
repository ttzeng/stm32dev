# STM32F103C8 Black Pill I2C Scanner

A comprehensive I2C device scanner for the STM32F103C8 Black Pill development board. In addition to the heart beat LED, this program scans all valid I2C addresses (0x08-0x77) and displays found devices in a formatted table via UART.

## Features

- 🔍 Scans all valid 7-bit I2C addresses (0x08 to 0x77)
- 📊 Displays results in a clear hexadecimal table format
- 🔄 Continuous scanning every 5 seconds
- 📡 Real-time output via UART (115200 baud)
- ⚡ Configurable I2C speed (default: 100kHz)

## Hardware Requirements

### STM32F103C8 Black Pill Board
- ARM Cortex-M3 @ 72MHz
- 64KB Flash, 20KB RAM
- Built-in LED on PB12

### Additional Components
- USB-to-Serial adapter (for viewing results)
- I2C devices to scan
- Pull-up resistors (4.7kΩ recommended for I2C lines)
- Breadboard and jumper wires

## Pin Configuration

### I2C1 Interface
| Function | Pin | Description |
|----------|-----|-------------|
| SCL | PB6 | I2C Clock Line |
| SDA | PB7 | I2C Data Line |

### UART1 Interface (Serial Output)
| Function | Pin | Description |
|----------|-----|-------------|
| TX | PA9 | UART Transmit |
| RX | PA10 | UART Receive |

### Power & Ground
| Function | Pin | Description |
|----------|-----|-------------|
| VCC | 3.3V | Power Supply |
| GND | GND | Ground |

## Wiring Diagram

```
STM32F103C8 Black Pill    USB-to-Serial Adapter
┌─────────────────────┐   ┌─────────────────────┐
│                     │   │                     │
│ PA9 (TX)    ────────┼───┤ RX                  │
│ PA10 (RX)   ────────┼───┤ TX                  │
│ GND         ────────┼───┤ GND                 │
│                     │   └─────────────────────┘
│                     │   
│ PB6 (SCL)   ────────┼─── SCL (I2C Devices)
│ PB7 (SDA)   ────────┼─── SDA (I2C Devices)
│ 3.3V        ────────┼─── VCC (I2C Devices)
│ GND         ────────┼─── GND (I2C Devices)
└─────────────────────┘
```

**Note:** Add 4.7kΩ pull-up resistors from SCL and SDA lines to 3.3V.

## Software Setup

### Prerequisites
- STM32CubeIDE or similar development environment
- STM32 HAL Library
- ST-Link programmer or compatible
- Serial terminal software (PuTTY, Tera Term, Arduino IDE Serial Monitor, etc.)

### Required HAL Modules
Ensure these modules are enabled in `stm32f1xx_hal_conf.h`:
```c
#define HAL_I2C_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
```

## Installation

1. **Clone or download** this repository
2. **Import the project** into STM32CubeIDE
3. **Build the project** (Ctrl+B)
4. **Connect your ST-Link** programmer to the Black Pill
5. **Flash the program** to the microcontroller

## Usage

### Basic Operation
1. **Connect I2C devices** to PB6 (SCL) and PB7 (SDA)
2. **Connect USB-to-Serial adapter** to PA9 (TX) and PA10 (RX)
3. **Power on** the Black Pill board
4. **Open serial terminal** with settings:
   - Baud rate: 115200
   - Data bits: 8
   - Parity: None
   - Stop bits: 1
   - Flow control: None

### Reading Results
The scanner will automatically start and display results like this:

```
=== STM32F103C8 I2C Scanner ===

Scanning I2C bus...
     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- 27 -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- 57 -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- 77 

Found 4 I2C devices.
--------------------------------
```

### Interpreting Results
- **Hexadecimal numbers** (e.g., `27`, `48`) = Device found at this address
- **Double dashes** (`--`) = No device found at this address
- **Blank spaces** = Reserved addresses (not scanned)

## Common I2C Device Addresses

| Device Type | Typical Address | Example Chips |
|-------------|----------------|---------------|
| EEPROM | 0x50-0x57 | 24C32, 24C64 |
| RTC | 0x68 | DS1307, DS3231 |
| Temperature | 0x48, 0x49 | DS18B20, LM75 |
| Display | 0x27, 0x3C | LCD with I2C backpack, OLED |
| Accelerometer | 0x1D, 0x53 | ADXL345, MMA8452 |
| Gyroscope | 0x68, 0x69 | MPU6050, ITG3200 |

## Customization

### Change I2C Speed
Modify the clock speed in `MX_I2C1_Init()`:
```c
hi2c1.Init.ClockSpeed = 400000;  // 400kHz Fast Mode
// or
hi2c1.Init.ClockSpeed = 100000;  // 100kHz Standard Mode
```

### Change Scan Interval
Modify the delay in the main loop:
```c
HAL_Delay(2000);  // Scan every 2 seconds instead of 5
```

### Use Different I2C Port
To use I2C2 instead of I2C1:
1. Change `I2C1` to `I2C2` in initialization
2. Update pins to PB10 (SCL) and PB11 (SDA)
3. Update the MSP initialization accordingly

## Troubleshooting

### No Devices Found
- ✅ Check wiring connections
- ✅ Verify 3.3V power supply to I2C devices
- ✅ Ensure pull-up resistors are connected (4.7kΩ to 3.3V)
- ✅ Test with known working I2C device

### Serial Output Issues
- ✅ Verify UART connections (TX/RX not swapped)
- ✅ Check baud rate settings (115200)
- ✅ Ensure common ground between devices

### Compilation Errors
- ✅ Verify all required HAL modules are enabled
- ✅ Check include paths and library linking
- ✅ Ensure correct STM32F1xx device is selected

## Technical Specifications

### I2C Specifications
- **Standard Mode**: 100kHz (default)
- **Fast Mode**: 400kHz (configurable)
- **Address Range**: 0x08 to 0x77 (7-bit addressing)
- **Pull-up Resistors**: 4.7kΩ recommended

### UART Specifications
- **Baud Rate**: 115200 bps
- **Data Format**: 8N1 (8 data bits, no parity, 1 stop bit)
- **Flow Control**: None

### System Clock
- **Core Clock**: 72MHz (PLL from 8MHz HSE)
- **APB1 Clock**: 36MHz
- **APB2 Clock**: 72MHz

## Contributing

Contributions are welcome! Please feel free to submit:
- 🐛 Bug reports
- 💡 Feature requests
- 🔧 Pull requests
- 📝 Documentation improvements

---

**Happy I2C scanning! 🚀**