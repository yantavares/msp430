    #include <msp430.h>

    // WHEN BUTTON P2.1 IS PRESSED, TOGGLE LEDS

    #define TRUE 1
    #define DBC 1000

    void count_leds(int count);
    void debounce(int b);
    void io_config();


    /**
     * main.c
     */
    int main(void)
    {
        int count = 0;
        WDTCTL = WDTPW | WDTHOLD;	                         // stop watchdog timer
        io_config();
        while(TRUE){
            while((P2IN & BIT1) == BIT1);                    // botão não foi pressionado (estado alto)
            debounce(DBC);                                   // botão pressionado (saiu do loop)
            count++;
            count_leds(count);
            while((P2IN & BIT1) == 0);                      // botão continua pressionado (estado baixo)
            debounce(DBC);
        }

        return 0;
    }

    void count_leds(int count){
        switch(count & 3){                                  // count & ...011 -> Garante que o número está entre 0 e 3
            case 0: P1OUT &= ~BIT0; P4OUT &= ~BIT7; break;  // Apaga os leds
            case 1: P1OUT &= ~BIT0; P4OUT |= BIT7; break;   // Liga led verde
            case 2: P1OUT |= BIT0; P4OUT &= ~BIT7; break;   // Liga led vermelho
            case 3: P1OUT |= BIT0; P4OUT |= BIT7; break;    // Apaga os leds
        }
    }

    void debounce(int b){
        volatile int x;                                     // Faz o compilador não otimizar variável
        for(x = 0; x < b; x++);
    }

    void io_config(){
        P1DIR |= BIT0;                                      // led P1.0 -> Saída
        P1OUT &= ~BIT0;                                     // led 1 apagado

        P4DIR |= BIT7;                                      // led P4.7 -> Saída
        P4OUT &= ~BIT7;                                     // led 2 apagado

        P2DIR &= ~BIT1;                                     // botão P2.1 -> Entrada
        P2REN |= BIT1;                                      // Habilita resistor
        P2OUT |= BIT1;                                      // Habilita pull up
    }



