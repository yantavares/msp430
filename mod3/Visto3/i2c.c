#include <msp430.h>
#include "i2c.h"

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

uint8_t i2cReceive(uint8_t addr)
{
    uint8_t data;

    UCB0I2CSA = addr;
    UCB0CTL1 &= ~UCTR;   // Receptor
    UCB0CTL1 |= UCTXSTT; // START

    while (UCB0CTL1 & UCTXSTT)
        ;
    data = UCB0RXBUF;

    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP)
        ;

    return data;
}