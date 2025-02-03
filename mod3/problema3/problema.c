#include <msp430.h>
#include <stdio.h>
#include <string.h>

#define TERM_CLR "\033[2J"
#define TERM_HOME "\033[0;0H"
#define BAUD_RATE 19200

volatile unsigned int num_canais = 1; // Número de canais configurados
volatile unsigned int adc_values[8];  // Buffer para armazenar valores do ADC
char uart_buffer[16];

// Definições do LCD
#define LCD_RS BIT0
#define LCD_RW BIT1
#define LCD_EN BIT2
#define LCD_DATA P2OUT
#define LCD_DIR P2DIR

void delay_ms(unsigned int ms)
{
    while (ms--)
    {
        __delay_cycles(1000);
    }
}

void LCD_Command(unsigned char cmd)
{
    LCD_DATA = (cmd & 0xF0);
    P1OUT &= ~LCD_RS; // Modo comando
    P1OUT |= LCD_EN;
    delay_ms(2);
    P1OUT &= ~LCD_EN;
    delay_ms(2);
    LCD_DATA = (cmd << 4) & 0xF0;
    P1OUT |= LCD_EN;
    delay_ms(2);
    P1OUT &= ~LCD_EN;
    delay_ms(2);
}

void LCD_Init(void)
{
    LCD_DIR = 0xFF;
    P1DIR |= (LCD_RS | LCD_EN);
    delay_ms(20);
    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
    delay_ms(2);
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    delay_ms(2);
}

void LCD_WriteChar(char c)
{
    LCD_DATA = (c & 0xF0);
    P1OUT |= LCD_RS;
    P1OUT |= LCD_EN;
    delay_ms(2);
    P1OUT &= ~LCD_EN;
    delay_ms(2);
    LCD_DATA = (c << 4) & 0xF0;
    P1OUT |= LCD_EN;
    delay_ms(2);
    P1OUT &= ~LCD_EN;
    delay_ms(2);
}

void LCD_WriteString(char *str)
{
    while (*str)
    {
        LCD_WriteChar(*str++);
    }
}

void LCD_Display(unsigned int value)
{
    char lcd_buffer[5];
    sprintf(lcd_buffer, "%03X", value);
    LCD_Clear();
    LCD_WriteString(lcd_buffer);
}

void UART_Init(void)
{
    UCA0CTL1 |= UCSWRST;
    UCA0CTL1 |= UCSSEL_2;
    UCA0BR0 = 52;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;
    UCA0CTL1 &= ~UCSWRST;
    IE2 |= UCA0RXIE;
}

void UART_SendString(char *str)
{
    while (*str)
    {
        while (!(IFG2 & UCA0TXIFG))
            ;
        UCA0TXBUF = *str++;
    }
}

void ADC_Init(void)
{
    ADC12CTL0 = ADC12SHT02 + ADC12ON;
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1; // Modo de sequência automática
    ADC12MCTL0 = ADC12INCH_0;             // Canal A0
    ADC12MCTL1 = ADC12INCH_1;             // Canal A1
    ADC12MCTL2 = ADC12INCH_2;             // Canal A2
    ADC12MCTL3 = ADC12INCH_3;             // Canal A3
    ADC12MCTL4 = ADC12INCH_4;             // Canal A4
    ADC12MCTL5 = ADC12INCH_5;             // Canal A5
    ADC12MCTL6 = ADC12INCH_6;             // Canal A6
    ADC12MCTL7 = ADC12INCH_7 | ADC12EOS;  // Canal A7 + End of Sequence
    ADC12CTL0 |= ADC12ENC;
}

void Timer_Init(void)
{
    TA0CCTL0 = CCIE;
    TA0CCR0 = 1000000 / (200 / num_canais); // Ajuste correto da taxa de amostragem (200/N Hz)
    TA0CTL = TASSEL_2 + MC_1;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
    unsigned int i;
    ADC12CTL0 |= ADC12SC; // Inicia conversão

    for (i = 0; i < num_canais; i++)
    {
        while (!(ADC12IFG & (1 << i)))
            ;                                 // Espera a conversão
        adc_values[i] = ADC12MEM[i] & 0x0FFF; // Lê o valor do ADC
    }

    for (i = 0; i < num_canais; i++)
    {
        sprintf(uart_buffer, "%d: %03X\r\n", i, adc_values[i]);
        UART_SendString(uart_buffer);
    }

    LCD_Display(adc_values[0]); // Mostra apenas A0 no LCD
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    char received_char = UCA0RXBUF;
    if (received_char >= '1' && received_char <= '8')
    {
        num_canais = received_char - '0';
        TA0CCR0 = 1000000 / (200 / num_canais); // Atualiza taxa de amostragem
    }
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    UART_Init();
    ADC_Init();
    Timer_Init();
    LCD_Init();

    __enable_interrupt();

    while (1)
        ;
}
