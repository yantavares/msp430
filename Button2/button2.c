    #include <msp430.h>

    // WHEN BUTTON P2.1 IS PRESSED, TOGGLE LEDS (COUNT UP)
    // WHEN BUTTON P1.1 IS PRESSED, TOGGLE LEDS (COUNT DOWN)

    #define TRUE 1
    #define FALSE 0
    #define DBC 1000
    #define OPEN 1
    #define CLOSED 0

    void count_leds(int count);
    void debounce(int b);
    void io_config();
    int watch_s1();
    int watch_s2();


    /**
     * main.c
     */
    void main()
    {
        static int count = 0;
        WDTCTL = WDTPW | WDTHOLD;                            // stop watchdog timer
        io_config();
        while(TRUE){
            if(watch_s1() == TRUE) count++;                  // se botão 1 foi pressionado, incrementa contador
            if(watch_s2() == TRUE) count--;                  // se botão 2 foi pressionado, decrementa contador
            count_leds(count);
        }
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

        P1DIR &= ~BIT1;                                     // botão P1.1 -> Entrada
        P1REN |= BIT1;                                      // Habilita resistor
        P1OUT |= BIT1;                                      // Habilita pull up
    }

    int watch_s1(){
        static int ps1 = OPEN;                              // static garante que variável não é desalocada pós função
        if((P2IN & BIT1) == 0){                             // chave pressionada
            if(ps1 == OPEN){                                // garante que botão não está sendo "segurado"
                debounce(DBC);
                ps1 = CLOSED;
                return TRUE;
            }
        }else{
            if(ps1 == CLOSED){
                debounce(DBC);
                ps1 = OPEN;
                return FALSE;
            }
        }
        return FALSE;
    }

    int watch_s2(){
        static int ps2 = OPEN;                              // static garante que variável não é desalocada pós função
        if((P1IN & BIT1) == 0){                             // chave pressionada
            if(ps2 == OPEN){                                // garante que botão não está sendo "segurado"
                debounce(DBC);
                ps2 = CLOSED;
                return TRUE;
            }
        }else{
            if(ps2 == CLOSED){
                debounce(DBC);
                ps2 = OPEN;
                return FALSE;
            }
        }
        return FALSE;
    }



