#include <msp430.h>

#define BUFSIZE 20

void UARTConfig();
void uartPrint(char *str);


volatile char buff[BUFSIZE];
volatile unsigned int count = 0;
volatile unsigned int READY = 0;
volatile char rec;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    UARTConfig();

    __enable_interrupt();

    uartPrint("Comunicando...");

    while (1)
        ;
}

void UARTConfig(){
    UCA1CTL1 = UCSWRST;
    UCA1CTL1 |= UCSSEL__SMCLK;
    UCA1BRW = 9;
    UCA1MCTL = UCBRS_1;
    P4SEL |= BIT4 | BIT5;
    UCA1CTL1 &= ~UCSWRST;

    UCA1IE |= UCRXIE;
}

void uartPrint(char *str)
{
    while (*str)
    {
        while (!(UCA1IFG & UCTXIFG))
            ;
        UCA1TXBUF = *str++;
    }
}

#pragma vector = USCI_A1_VECTOR
__interrupt void isr(void)
{
    count++;

    if(count >= BUFSIZE){
        count = 0;
    }

    rec = UCA1RXBUF;

    if (rec == '\n')
        READY = 1;
    else
        buff[count] = rec;

}
