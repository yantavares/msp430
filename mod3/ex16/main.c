#include <msp430.h>

#define TRUE 1
#define VRX 0
#define VRY 1

#define PWM_PERIOD 255

void configureGPIO(void);
void configureADC(void);
void configureTimer(void);
void startADC(void);

volatile unsigned char brightnessX, brightnessY;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    configureGPIO();
    configureADC();
    configureTimer();
    __enable_interrupt();

    while (TRUE)
    {
        startADC();
        __bis_SR_register(LPM0_bits + GIE);
        TB0CCR1 = brightnessX;
        TB0CCR2 = brightnessY;
    }

    return 0;
}

void configureGPIO(void)
{
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    P4DIR |= BIT7;
    P4SEL |= BIT7;

    P6SEL |= BIT0 | BIT1;
}

void configureADC(void)
{
    ADC12CTL0 = ADC12SHT02 + ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12MCTL0 = ADC12INCH_0;
    ADC12MCTL1 = ADC12INCH_1 + ADC12EOS;
    ADC12IE = ADC12IE1;
    ADC12CTL0 |= ADC12ENC;
}

void configureTimer(void)
{
    TB0CCTL1 = OUTMOD_7;
    TB0CCTL2 = OUTMOD_7;
    TB0CTL = TBSSEL_2 + MC_1 + TBCLR;
    TB0CCR0 = PWM_PERIOD;
}

void startADC(void)
{
    ADC12CTL0 |= ADC12SC;
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    brightnessX = ADC12MEM0;
    brightnessY = ADC12MEM1;
    __bic_SR_register_on_exit(LPM0_bits);
}
