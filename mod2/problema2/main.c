#include <msp430.h>

#define _1_x_1 0xBA45
#define _1_x_2 0xB946
#define _1_x_3 0xB847
#define _2_x_1 0xBB44
#define _2_x_2 0xBF40
#define _2_x_3 0xBC43
#define _3_x_1 0xF807
#define _3_x_2 0xEA15
#define _3_x_3 0xF609
#define _4_x_1 0xE916
#define _4_x_2 0xE619
#define _4_x_3 0xF20D
#define _5_x_2 0xE718
#define _6_x_1 0xF708
#define _6_x_2 0xE31C
#define _6_x_3 0xA55A
#define _7_x_2 0xAD52

#define POT100 10000
#define START_MAX 14500
#define START_MIN 13500
#define BIT1_MAX 2450
#define BIT1_MIN 2250
#define BIT0_MAX 1120
#define BIT0_MIN 1340
#define T20ms 20971
#define T500us 524
#define P10DEGREES 117

volatile int i = 0;
volatile int period = 0;
volatile int vector[32];
volatile unsigned long code;
volatile unsigned long key;
volatile int position = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    config_leds();
    config_tb0_1();
    config_ta1_1();
    config_servo();

    while (1)
    {
        config_falling_ta1_1();
        wait_first_falling_edge();
        wait_second_falling_edge();

        if ((period > START_MIN) && (period < START_MAX))
        {
            config_rising_ta1_1();
            wait_rising_edge();
            write_vector();
        }

        vector_into_words();
        execute_instructions(key);
        _delay_cycles(100);
    }
}

void write_vector(void)
{
    for (i = 0; i < 32; i++)
    {
        while ((TA1CCTL1 & CCIFG) == 0)
            ;
        TA1CTL |= TACLR;
        TA1CCTL1 &= ~CCIFG;
        vector[i] = TA1CCR1;
    }
}

void vector_into_words(void)
{
    for (i = 0; i < 32; i++)
    {
        code = code >> 1;
        if ((vector[i] > BIT1_MIN) && (vector[i] < BIT1_MAX))
        {
            code |= 0x80000000L;
        }
    }
    key = code >> 16;
}

void config_ta1_1(void)
{
    TA1CTL = TASSEL_2 | MC_2;
    P2DIR &= ~BIT0;
    P2REN |= BIT0;
    P2OUT |= BIT0;
    P2SEL |= BIT0;
    TA1CCTL1 &= ~CCIFG;
}

void config_rising_ta1_1(void)
{
    TA1CCTL1 = CM_1 | CCIS_0 | SCS | CAP;
    TA1CCTL1 &= ~CCIFG;
}

void config_falling_ta1_1(void)
{
    TA1CCTL1 = CM_2 | CCIS_0 | SCS | CAP;
    TA1CCTL1 &= ~CCIFG;
}

void wait_first_falling_edge(void)
{
    while ((TA1CCTL1 & CCIFG) == 0)
        ;
    TA1CTL |= TACLR;
    TA1CCTL1 &= ~CCIFG;
}

void wait_second_falling_edge(void)
{
    while ((TA1CCTL1 & CCIFG) == 0)
        ;
    TA1CTL |= TACLR;
    TA1CCTL1 &= ~CCIFG;
    period = TA1CCR1;
}

void wait_rising_edge(void)
{
    while ((TA1CCTL1 & CCIFG) == 0)
        ;
    TA1CTL |= TACLR;
    TA1CCTL1 &= ~CCIFG;
}

void config_leds(void)
{
    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

    P4OUT &= ~BIT7;
    P4DIR |= BIT7;
    P4SEL |= BIT7;
    PMAPKEYID = 0x02D52;
    P4MAP7 = PM_TB0CCR1A;
}

void increase_position(void)
{
    position++;
    if (position > 18)
        position = 18;
    TA2CCR2 = T500us + position * P10DEGREES;
}

void decrease_position(void)
{
    position--;
    if (position < 0)
        position = 0;
    TA2CCR2 = T500us + position * P10DEGREES;
}

void execute_instructions(int key)
{
    switch (key)
    {
    case _1_x_1:
        P1OUT |= BIT0;
        break;
    case _1_x_2:
        P1OUT &= ~BIT0;
        break;
    case _1_x_3:
        P1OUT ^= BIT0;
        break;
    case _2_x_1:
        TB0CCR1 = POT100;
        break;
    case _2_x_2:
        TB0CCR1 = 0;
        break;
    case _2_x_3:
        TB0CCR1 = POT100 - TB0CCR1;
        break;
    case _3_x_1:
        decrease_position();
        break;
    case _3_x_2:
        TB0CCR1 = 0;
        increase_position();
        break;
    case _3_x_3:
        TB0CCR1 = POT100 * 0.1;
        break;
    case _4_x_1:
        TB0CCR1 = POT100 * 0.2;
        position = 0;
        TA2CCR2 = T500us;
        break;
    case _4_x_2:
        TB0CCR1 = POT100 * 0.3;
        position = 9;
        TA2CCR2 = T500us + (position * P10DEGREES);
        break;
    case _4_x_3:
        TB0CCR1 = POT100 * 0.4;
        position = 18;
        TA2CCR2 = T500us + (position * P10DEGREES);
        break;
    case _5_x_2:
        TB0CCR1 = POT100 * 0.5;
        break;
    case _6_x_1:
        TB0CCR1 = POT100 * 0.6;
        break;
    case _6_x_2:
        TB0CCR1 = POT100 * 0.7;
        break;
    case _6_x_3:
        TB0CCR1 = POT100 * 0.8;
        break;
    case _7_x_2:
        TB0CCR1 = POT100 * 0.9;
        break;
    default:
        break;
    }
}

void config_tb0_1(void)
{
    TB0CTL = TBSSEL_2 | MC_1;
    TB0CCR0 = POT100;
    TB0CCTL1 = OUTMOD_6;
    TB0CCR1 = POT100;
}

void config_servo(void)
{
    TA2CTL = TASSEL_2 | ID_0 | MC_1 | TACLR;
    TA2EX0 = TAIDEX_0;
    TA2CCR0 = T20ms;
    TA2CCTL2 = OUTMOD_6;
    TA2CCR2 = T500us;
    P2DIR |= BIT5;
    P2SEL |= BIT5;
}
