#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    TA0CTL = TASSEL__ACLK | MC__UP | TACLR; // Config ACLK, UP mode control, começa no 0

    // Saída -> LED1
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;

    TA0CCR0 = 16384 - 1; // Para desligar/ligar a cada 500ms

    while(1){
        while(!(TA0CTL & TAIFG));
        P4OUT ^= BIT7;
        TA0CTL &= ~TAIFG;
    }


    return 0;
}
