#include <msp430.h>
#include <stdint.h>

#define SLAVE_ADDR_1 0x27
#define SLAVE_ADDR_2 0x3F

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    USCI_B0_config();

    lcdInit();
    lcdWrite("Teste de quebra\nde linha");
    turnOnBacklight();

    return 0;
}

void USCI_B0_config(void)
{
    UCB0CTL1 = UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0BRW = 105;
    UCB0CTL1 = UCSSEL_3;
    P3SEL |= BIT0 | BIT1;
    P3REN |= BIT0 | BIT1;
    P3OUT |= BIT0 | BIT1;
    UCB0CTL1 &= ~UCSWRST;
}

uint8_t i2cSend(uint8_t addr, uint8_t data)
{
    UCB0I2CSA = addr;
    UCB0CTL1 |= UCTR | UCTXSTT;

    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0TXBUF = data;

    while (UCB0CTL1 & UCTXSTT)
        ;

    if (UCB0IFG & UCNACKIFG)
    {
        UCB0CTL1 |= UCTXSTP;
        while (UCB0CTL1 & UCTXSTP)
            ;
        return 1;
    }

    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP)
        ;

    return 0;
}

void turnOnBacklight(void)
{
    uint8_t addr = lcdAddr();
    i2cSend(addr, BIT3);
    __delay_cycles(200000);
}

void clearScreen(void)
{
    uint8_t addr = lcdAddr();
    i2cSend(addr, 0x01);
    __delay_cycles(200000);
}

void blink(void)
{
    uint8_t addr = lcdAddr();
    int i;
    for (i = 0; i < 11; i++)
    {
        i2cSend(addr, BIT3);
        __delay_cycles(500000);
        i2cSend(addr, 0x00);
        __delay_cycles(500000);
    }
}

uint8_t lcdAddr(void)
{
    if (i2cSend(SLAVE_ADDR_1, 0x00) == 0)
        return SLAVE_ADDR_1;
    if (i2cSend(SLAVE_ADDR_2, 0x00) == 0)
        return SLAVE_ADDR_2;
    return 0;
}

void lcdWriteNibble(uint8_t nibble, uint8_t isChar)
{
    uint8_t addr;
    uint8_t data;

    addr = lcdAddr();
    data = nibble << 4;

    if (isChar == 1)
        data = data | BIT0;

    i2cSend(addr, data);
    i2cSend(addr, data | BIT2); // BIT2 = EN
    i2cSend(addr, data);
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

void lcdInit(void)
{
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x2, 0);

    lcdWriteByte(0x28, 0);
    lcdWriteByte(0x0C, 0);
    lcdWriteByte(0x06, 0);
    lcdWriteByte(0x01, 0);
    __delay_cycles(400000);
}

void lcdWrite(char *str)
{
    uint8_t pos = 0;

    while (*str)
    {
        if (*str == '\n')
        {
            if (pos <= 16)
            {
                lcdWriteByte(0xC0, 0);
                str++;
                pos = 16;
            }
            else
            {
                lcdWriteByte(0x80, 0);
                str++;
                pos = 0;
            }
        }
        else
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
}
