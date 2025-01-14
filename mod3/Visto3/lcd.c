#include <msp430.h>
#include "lcd.h"
#include "i2c.h"

uint8_t lcdAddr(void)
{
    if (i2cSend(SLAVE_ADDR_1, 0x00) == 0)
        return SLAVE_ADDR_1;
    if (i2cSend(SLAVE_ADDR_2, 0x00) == 0)
        return SLAVE_ADDR_2;
    return 0;
}

void lcdInit(void)
{
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x2, 0);
    __delay_cycles(15000);

    lcdWriteByte(0x28, 0);
    lcdWriteByte(0x0C, 0);
    lcdWriteByte(0x06, 0);
    lcdWriteByte(0x01, 0);
}

void lcdWrite(char *str)
{
    uint8_t pos = 0;

    while (*str)
    {
        if (pos == 16)
        {
            lcdWriteByte(0xC0, 0);
        }
        if (pos == 32)
        {
            lcdWriteByte(0x80, 0);
            pos = 0;
        }
        lcdWriteByte(*str++, 1);
        pos++;
    }
}

uint8_t lcdReadNibble(uint8_t isChar)
{
    uint8_t addr = lcdAddr();
    uint8_t data;

    // Leitura
    i2cSend(addr, (isChar ? BIT_RS : 0) | BIT_RW);
    i2cSend(addr, ((isChar ? BIT_RS : 0) | BIT_RW | BIT_EN));
    __delay_cycles(300);

    data = i2cReceive(addr);
    i2cSend(addr, (isChar ? BIT_RS : 0) | BIT_RW); // EN = 0

    return (data >> 4);
}

void lcdWriteByte(uint8_t byte, uint8_t isChar)
{
    uint8_t msb;
    uint8_t lsb;

    msb = byte / 16;
    lsb = byte % 16;

    lcdWriteNibble(msb, isChar);
    lcdWriteNibble(lsb, isChar);
}

uint8_t lcdReadByte(uint8_t isChar)
{
    uint8_t msb = lcdReadNibble(isChar);

    uint8_t lsb = lcdReadNibble(isChar);

    uint8_t byte = (msb << 4) | lsb;

    return byte;
}

uint8_t lcdBusy(void)
{
    uint8_t busyFlag;

    busyFlag = lcdReadByte(0);

    if (busyFlag & BIT7)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}