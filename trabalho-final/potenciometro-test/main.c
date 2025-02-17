#include <msp430.h>
#include <stdint.h>

#define POTENTIOMETER_PIN 1   // P6.1 conectado ao potenciômetro

void UART_Init(void);
void UART_SendChar(char c);
void UART_SendString(const char *str);
uint16_t ADCRead(uint8_t pin);
void delay_ms(uint16_t ms);

volatile char buffer[10];
volatile uint16_t potValue;
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Desativa o Watchdog Timer
    UART_Init();               // Inicializa UART para comunicação serial


    while(1)
    {
        potValue = ADCRead(POTENTIOMETER_PIN);  // Lê valor do potenciômetro
        if (potValue > 4000){
            potValue = 4000;
        }

        // Envia o valor lido via UART
        UART_SendString("Valor do Potenciometro: ");
        UART_SendString(buffer);
        UART_SendString("\r\n");

        delay_ms(500);  // Aguarda 500ms entre leituras
    }

    return 0;
}

// Função para ler um valor do ADC
uint16_t ADCRead(uint8_t pin)
{
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_3 | ADC12ON;
    ADC12CTL1 = ADC12SHS_0 | ADC12SHP | ADC12CONSEQ_0;
    ADC12CTL2 = ADC12RES_2;

    P6SEL |= 1 << pin;   // Configura pino P6.x como entrada analógica
    ADC12MCTL0 = pin;    // Seleciona o canal ADC
    ADC12CTL0 |= ADC12ENC;
    ADC12CTL0 |= ADC12SC;

    while(!(ADC12IFG & BIT0));  // Espera a conversão terminar

    return ADC12MEM0;  // Retorna o valor convertido
}

// Inicializa UART para enviar dados serialmente
void UART_Init(void)
{
    P3SEL |= BIT3 + BIT4;  // Configura P3.3 (TX) e P3.4 (RX) para UART

    UCA0CTL1 |= UCSWRST;   // Reseta a UART para configuração
    UCA0CTL1 |= UCSSEL_2;  // Usa SMCLK como clock da UART

    UCA0BR0 = 104;         // Configura baud rate para 9600 (assumindo 1MHz)
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_1;    // Modulação para precisão

    UCA0CTL1 &= ~UCSWRST;  // Habilita UART
}

// Envia um caractere pela UART
void UART_SendChar(char c)
{
    while (!(UCA0IFG & UCTXIFG));  // Aguarda buffer estar livre
    UCA0TXBUF = c;  // Envia caractere
}

// Envia uma string pela UART
void UART_SendString(const char *str)
{
    while (*str)
    {
        UART_SendChar(*str++);
    }
}

// Função para delay aproximado (depende da frequência do clock)
void delay_ms(uint16_t ms)
{
    while (ms--)
    {
        __delay_cycles(1000);  // Supondo clock de 1MHz
    }
}
