#include <msp430.h>

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Para o Watchdog Timer

    // Configura P3.6 como saída
    P3DIR |= BIT6;   // P3.6 como saída digital
    P3OUT |= BIT6;   // Ativa P3.6 (motor ligado)

    while (1); // Loop infinito, mantendo o motor ligado
}
