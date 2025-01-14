#include <msp430.h> 

void debounce(){
    volatile int DBC = 20000;
    while (DBC--){}
}

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Saída -> LED1
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;


    // Entrada -> Botão S1
    P2DIR &= ~BIT1;
    P2REN |= BIT1;
    P2OUT |= BIT1;


    while(1){
        if (!(P2IN & BIT1)){
            P1OUT ^= BIT0;
            while(!(P2IN & BIT1)){}
        }
        debounce();
    }


    return 0;
}


