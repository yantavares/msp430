#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void USCI_B0_config(void);
uint8_t i2cSend(uint8_t addr, uint8_t data);
uint8_t i2cReceive(uint8_t addr);

#endif