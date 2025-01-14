#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define SLAVE_ADDR_1 0x27
#define SLAVE_ADDR_2 0x3F
#define BIT_EN BIT2
#define BIT_RW BIT1
#define BIT_RS BIT0

void lcdInit(void);
void lcdWrite(char *str);
uint8_t lcdReadNibble(uint8_t isChar);
uint8_t lcdReadByte(uint8_t isChar);
uint8_t lcdBusy(void);

#endif