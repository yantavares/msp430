#include <msp430.h>
#include <stdint.h>

#define SLAVE_ADDR 0x34
#define TEST_BYTE 0x55

void USCI_B0_config(void);
void USCI_B1_config(void);
uint8_t i2cSend(uint8_t addr, uint8_t data);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    USCI_B0_config();
    USCI_B1_config();

    uint8_t ack = i2cSend(SLAVE_ADDR, TEST_BYTE);

    if (ack == 0)
    {
        // ACK: LED verde
        P4DIR |= BIT7;
        while (1)
        {
            P4OUT ^= BIT7;
            __delay_cycles(100000);
        }
    }
    else
    {
        // NACK: LED vermelho
        P1DIR |= BIT0;
        while (1)
        {
            P1OUT ^= BIT0;
            __delay_cycles(100000);
        }
    }
}

// USCI_B0: mestre
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

// USCI_B1: escravo
void USCI_B1_config(void)
{
    UCB1CTL1 = UCSWRST;
    UCB1CTL0 = UCMODE_3 | UCSYNC;
    UCB1I2COA = SLAVE_ADDR;
    P4SEL |= BIT1 | BIT2;
    P4REN |= BIT1 | BIT2;
    P4OUT |= BIT1 | BIT2;
    UCB1CTL1 &= ~UCSWRST;
    UCB1IE = UCTXIE | UCRXIE;
    __enable_interrupt();
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
        return 1; // NACK
    }

    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP)
        ;

    return 0; // ACK
}

#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
    switch (__even_in_range(UCB1IV, 0xC))
    {
    case 0x0:
        break;
    case 0x2:
        break; // START
    case 0x4:
        break; // STOP
    case 0x6:
        break; // Condição de recepção
    case 0x8:
        break; // NACK
    case 0xA:  // Receber dado
    {
        uint8_t rx_data = UCB1RXBUF; // Ler dado recebido
        break;
    }
    case 0xC:             // Transmitir dado
        UCB1TXBUF = 0x00; // Enviar dado fixo
        break;
    }
}
