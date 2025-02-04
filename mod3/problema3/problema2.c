#include <msp430.h>
#include <stdio.h>
#include <stdint.h>

#define TERM_CLR "\033[2J"
#define TERM_HOME "\033[0;0H"
#define NEWLINE "\n\r"

#define BUFSIZE 16
#define MAX_CH 8

#define BASE_SPS 200.0f

#define ACLK_FREQ 32768UL

volatile char rxBuffer[BUFSIZE];
volatile int rxCount = 0;
volatile int rxReady = 0;
volatile char rxChar = 0;

volatile uint16_t adcValues[MAX_CH];
volatile uint8_t numChannels = 1;

char stringOut[64];

void config_WDT(void);
void config_Clock(void);
void config_UART(void);
void config_ADC(void);
void config_TimerA(void);
void update_TimerA_for_Nchannels(uint8_t N);

void uartPrint(char *str);
void showAllChannelsUART(void);

void config_LCD(void);
void lcdClear(void);
void lcdWriteString(char *s);
void lcdShowChannel0(uint16_t value);

int main(void)
{
    config_WDT();
    config_Clock();
    config_UART();
    config_ADC();
    config_TimerA();
    config_LCD();

    uartPrint(TERM_CLR);
    uartPrint("Osciloscopio Digital - MSP430\n\r");
    uartPrint("Digite um numero (1..8) para configurar canais:\n\r");

    __enable_interrupt();

    while (1)
    {

        if (rxReady)
        {
            rxReady = 0;

            if (rxChar >= '1' && rxChar <= '8')
            {
                numChannels = rxChar - '0';
                update_TimerA_for_Nchannels(numChannels);

                uartPrint("\n\r*** Numero de canais atualizado para: ");
                sprintf(stringOut, "%d ***\n\r", numChannels);
                uartPrint(stringOut);
            }
            else
            {
            }
        }

        showAllChannelsUART();

        lcdShowChannel0(adcValues[0]);

        __delay_cycles(100000);
    }
}

void config_WDT(void)
{
    WDTCTL = WDTPW | WDTHOLD;
}

void config_Clock(void)
{
}

void config_UART(void)
{

    UCA1CTL1 |= UCSWRST;

    UCA1CTL1 |= UCSSEL__SMCLK;

    UCA1BR0 = 54;
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS_5;

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

void showAllChannelsUART(void)
{
    uint8_t i;
    for (i = 0; i < numChannels; i++)
    {

        sprintf(stringOut, "%d: %03x  ", i, (adcValues[i] & 0x0FFF));
        uartPrint(stringOut);
    }
    uartPrint(NEWLINE);
}

void config_ADC(void)
{

    ADC12CTL0 &= ~ADC12ENC;

    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;

    ADC12CTL1 = ADC12CONSEQ_1 | ADC12SHS_1 | ADC12SSEL_0 | ADC12CSTARTADD_0;

    ADC12CTL2 = ADC12RES_2;

    ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_0;
    ADC12MCTL1 = ADC12SREF_0 | ADC12INCH_1;
    ADC12MCTL2 = ADC12SREF_0 | ADC12INCH_2;
    ADC12MCTL3 = ADC12SREF_0 | ADC12INCH_3;
    ADC12MCTL4 = ADC12SREF_0 | ADC12INCH_4;
    ADC12MCTL5 = ADC12SREF_0 | ADC12INCH_5;
    ADC12MCTL6 = ADC12SREF_0 | ADC12INCH_6;

    ADC12MCTL7 = ADC12SREF_0 | ADC12INCH_7 | ADC12EOS;

    ADC12IE = ADC12IE7;

    P6SEL |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

    ADC12CTL0 |= ADC12ENC;
}

void config_TimerA(void)
{

    TA0CTL = TASSEL_1 | MC_1 | TACLR;
    TA0CCR0 = 163;

    TA0CCTL1 = OUTMOD_3;
    TA0CCR1 = 1;
}

void update_TimerA_for_Nchannels(uint8_t N)
{
    uint32_t ticks;

    if (N == 0)
        N = 1;

    ticks = ((uint32_t)ACLK_FREQ * (uint32_t)N) / 200UL;

    if (ticks < 1)
        ticks = 1;
    if (ticks > 65535)
        ticks = 65535;

    TA0CTL = TASSEL_1 | MC_1 | TACLR;
    TA0CCR0 = (uint16_t)(ticks - 1);

    TA0CCTL1 = OUTMOD_3;

    TA0CCR1 = 1;
}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    if (UCA1IFG & UCRXIFG)
    {
        rxChar = UCA1RXBUF;
        rxReady = 1;
    }
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    if (ADC12IV == ADC12IV_ADC12IFG7)
    {

        adcValues[0] = ADC12MEM0;
        adcValues[1] = ADC12MEM1;
        adcValues[2] = ADC12MEM2;
        adcValues[3] = ADC12MEM3;
        adcValues[4] = ADC12MEM4;
        adcValues[5] = ADC12MEM5;
        adcValues[6] = ADC12MEM6;
        adcValues[7] = ADC12MEM7;
    }
}

void config_LCD(void)
{
}
void lcdClear(void) { /* Stub */ }
void lcdWriteString(char *s) { /* Stub */ }
void lcdShowChannel0(uint16_t value)
{

    char buf[16];

    float volts = (float)value * 3.3f / 4095.0f;
    sprintf(buf, "A0=%4.2fV", volts);

    lcdClear();
    lcdWriteString(buf);
}
