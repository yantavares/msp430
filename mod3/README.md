#### **Exercise 1: I2C Interface Test**

Configure and test I2C communication between `USCI_B0` (master) and `USCI_B1` (slave):

- Set `USCI_B0` as master operating at 100 kHz.
- Set `USCI_B1` with slave address `0x34`.
- Connect P3.1 to P4.2 (SCL) and P3.0 to P4.1 (SDA).
- Write the function `uint8_t i2cSend(uint8_t addr, uint8_t data)` to send `0x55` to the slave. The function should return 0 for ACK and 1 for NACK.

---

#### **Exercise 2: I2C Address Scanner**

Use the configuration from **Exercise 1** to implement an I2C address scanner:

- Probe all valid addresses on the bus (0x03 to 0x77).
- Return a list of detected device addresses using polling on the `TXIFG` and `RXIFG` flags.

---

#### **Exercise 3: LCD Backlight Control**

Toggle the backlight of the LCD at 1 Hz:

- Use `i2cSend()` to modify the backlight control bit (BIT3) of the PCF8574 IC connected to the LCD.
- Ensure the I2C clock speed is below 100 kHz.

---

#### **Exercise 4: Detect LCD Address**

Write `uint8_t lcdAddr()` to determine the I2C address of the connected LCD:

- Test both possible addresses (`0x27` and `0x3F`) using a write operation.
- Return the address that responds with an ACK.

---

#### **Exercise 5: Write Nibble to LCD**

Implement `lcdWriteNibble(uint8_t nibble, uint8_t isChar)`:

- Follow the 4-bit LCD protocol:
  1. Prepare RS, RW, and D[7:4] with EN = 0.
  2. Set EN = 1 for the pulse.
  3. Return EN to 0.

---

#### **Exercise 6: Write Byte to LCD**

Write a function `lcdWriteByte(uint8_t byte, uint8_t isChar)`:

- Call `lcdWriteNibble()` twice:
  1. First for the most significant nibble (MSB).
  2. Then for the least significant nibble (LSB).

---

#### **Exercise 7: Initialize LCD**

Create `lcdInit()` to ensure the LCD starts in a known state:

- Send the 8-bit initialization sequence (3 × 0x03 and 1 × 0x02).
- Configure the LCD for 4-bit mode and set display settings (e.g., clear display, enable cursor).

---

#### **Exercise 8: Write String to LCD**

Implement `lcdWrite(char *str)`:

- Write characters to the LCD until the null terminator is reached.
- Handle line wrapping by moving to the second line when the first line is full.

---

#### **Exercise 9: Read Byte from LCD**

Write `uint8_t lcdReadByte(uint8_t isChar)`:

- Use the 4-bit protocol to read both the MSB and LSB of the byte.
- Return the combined byte value.

---

#### **Exercise 10: Check LCD Busy Status**

Create `uint8_t lcdBusy()`:

- Use `lcdReadByte(0)` to check the busy flag (bit 7).
- Return 1 if the LCD is busy and 0 otherwise.

---

### **Problem 3: Digital Oscilloscope with ADC, LCD, and UART**

**Objective:**
Develop a digital oscilloscope using the MSP430’s ADC, LCD, and UART.

**Requirements:**

1. **Resolution:** Fixed at 12 bits.
2. **Display format:** Show ADC values as 3-digit hexadecimal numbers (e.g., `0: 800`).
3. **Channel selection:** Accept UART input (`1–8`) to configure the number of ADC channels.
4. **UART speed:** Operate at 19200 bps.
5. **LCD:** Display only the value of channel A0.
6. **UART:** Print values of all active channels.
7. **Sampling rate:** Adjust dynamically to `200/N` samples per second, where `N` is the number of channels.
8. **Safety:** Ensure the external resistance is within 10kΩ.
