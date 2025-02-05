#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TERM_CLR "\033[2J"
#define TERM_HOME "\033[0;0H"

volatile uint8_t nChannels = 1;
volatile uint8_t newConfigFlag = 0;
volatile uint8_t new_nChannels = 1;
volatile uint8_t sampleReady = 0;
volatile uint16_t adcResults[8] = {0};

char buffer[20];

#define SLAVE_ADDR_1 0x27
#define SLAVE_ADDR_2 0x3F

void USCI_B0_config(void);
uint8_t i2cSend(uint8_t addr, uint8_t data);
void turnOnBacklight(void);
void clearScreen(void);
void blink(void);
uint8_t lcdAddr(void);
void lcdWriteNibble(uint8_t nibble, uint8_t isChar);
void lcdWriteByte(uint8_t byte, uint8_t isChar);
void lcdInit(void);
void lcdWrite(char *str);
void UARTConfig(void);
void uartPrint(char *str);
void ADC_config(void);
void ADC_config_channels(uint8_t channels);
void TimerAConfig(void);
void updateTimerPeriod(void);

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
        return 1;
    }
    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP)
        ;
    return 0;
}

void turnOnBacklight(void)
{
    uint8_t addr = lcdAddr();
    i2cSend(addr, BIT3);
    __delay_cycles(200000);
}

void clearScreen(void)
{
    uint8_t addr = lcdAddr();
    i2cSend(addr, 0x01);
    __delay_cycles(200000);
}

void blink(void)
{
    uint8_t addr = lcdAddr();
    int i;
    for (i = 0; i < 11; i++)
    {
        i2cSend(addr, BIT3);
        __delay_cycles(500000);
        i2cSend(addr, 0x00);
        __delay_cycles(500000);
    }
}

uint8_t lcdAddr(void)
{
    if (i2cSend(SLAVE_ADDR_1, 0x00) == 0)
        return SLAVE_ADDR_1;
    if (i2cSend(SLAVE_ADDR_2, 0x00) == 0)
        return SLAVE_ADDR_2;
    return 0;
}

void lcdWriteNibble(uint8_t nibble, uint8_t isChar)
{
    uint8_t addr;
    uint8_t data;

    addr = lcdAddr();
    data = (nibble << 4);

    if (isChar == 1)
        data |= BIT0;

    i2cSend(addr, data);
    i2cSend(addr, data | BIT2);
    i2cSend(addr, data);
}

void lcdWriteByte(uint8_t byte, uint8_t isChar)
{
    uint8_t msb, lsb;
    msb = byte / 16;
    lsb = byte % 16;
    lcdWriteNibble(msb, isChar);
    lcdWriteNibble(lsb, isChar);
}

void lcdInit(void)
{
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x2, 0);

    lcdWriteByte(0x28, 0);
    lcdWriteByte(0x0C, 0);
    lcdWriteByte(0x06, 0);
    lcdWriteByte(0x01, 0);
    __delay_cycles(400000);
}

void lcdWrite(char *str)
{
    uint8_t pos = 0;
    while (*str)
    {
        if (*str == '\n')
        {
            if (pos <= 16)
            {
                lcdWriteByte(0xC0, 0);
                str++;
                pos = 16;
            }
            else
            {
                lcdWriteByte(0x80, 0);
                str++;
                pos = 0;
            }
        }
        else
        {
            if (pos == 16)
            {
                lcdWriteByte(0xC0, 0);
            }
            if (pos == 32)
            {
                lcdWriteByte(0x80, 0);
                pos = 0;
            }
            lcdWriteByte(*str++, 1);
            pos++;
        }
    }
}

void UARTConfig(void)
{
    UCA1CTL1 = UCSWRST;
    UCA1CTL1 |= UCSSEL_2;
    UCA1BR0 = 52;
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS0;
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

void ADC_config(void)
{
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_1;
    ADC12CTL2 = ADC12RES_2;
    P6SEL |= 0xFF;
}

void ADC_config_channels(uint8_t channels)
{
    uint8_t i;
    ADC12CTL0 &= ~ADC12ENC;
    for (i = 0; i < channels; i++)
    {
        ADC12MCTL[i] = ADC12SREF_0 | i;
    }
    ADC12MCTL[channels - 1] |= ADC12EOS;
    ADC12IE = 0;
    ADC12IE |= (1 << (channels - 1));
    ADC12CTL0 |= ADC12ENC;
}

// 1 000 000 / 200 = 5000
void TimerAConfig(void)
{
    TA0CCR0 = nChannels * 5000 - 1;
    TA0CTL = TASSEL_2 | MC_1 | TACLR;
    TA0CCTL0 = CCIE;
}

void updateTimerPeriod(void)
{
    TA0CCR0 = nChannels * 5000 - 1;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    UARTConfig();

    __enable_interrupt();

    char lcdBuffer[17];

    USCI_B0_config();
    lcdInit();

    ADC_config();
    ADC_config_channels(nChannels);

    TimerAConfig();

    turnOnBacklight();

    uartPrint(TERM_CLR);
    uartPrint(TERM_HOME);


    while (1)
    {

        if (newConfigFlag)
        {
            nChannels = new_nChannels;
            ADC12CTL0 &= ~ADC12ENC;
            ADC_config_channels(nChannels);
            updateTimerPeriod();
            newConfigFlag = 0;

            uartPrint(TERM_CLR);
            uartPrint(TERM_HOME);
        }

        if (sampleReady)
        {
            sampleReady = 0;

            sprintf(lcdBuffer, "A0: %03X ", adcResults[0]);
            clearScreen();
            lcdWrite(lcdBuffer);

            uint8_t i;

            uartPrint("Recebendo...\r\n");
            for (i = 0; i < nChannels; i++)
            {
                sprintf(buffer, "%d: %03X\r\n", i, adcResults[i]);
                uartPrint(buffer);
            }
        }
    }
}


#pragma vector = USCI_A1_VECTOR
__interrupt void ISR(void)
{
    char c;
    if (UCA1IFG & UCRXIFG)
    {
        c = UCA1RXBUF;

        if (c >= '1' && c <= '8')
        {
            new_nChannels = c - '0';
            newConfigFlag = 1;
        }
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void ISR2(void)
{
    ADC12CTL0 |= ADC12SC;
}

#pragma vector = ADC12_VECTOR
__interrupt void ISR3(void)
{
    uint8_t i;
    for (i = 0; i < nChannels; i++)
    {
        adcResults[i] = ADC12MEM[i];
    }
    sampleReady = 1;
}

