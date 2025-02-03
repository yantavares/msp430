#include <msp430.h>
#include <stdio.h>
#include <stdint.h>

//==================================================
// Configurações de Terminal
//==================================================
#define TERM_CLR   "\033[2J"    // Limpa tela
#define TERM_HOME  "\033[0;0H"  // Cursor no canto superior esquerdo
#define NEWLINE    "\n\r"

//==================================================
// Constantes e Definições
//==================================================
#define BUFSIZE    16           // Tamanho do buffer para recepção
#define MAX_CH     8            // Máximo de canais que usaremos (A0..A7)

// Frequência base desejada para as amostras TOTAIS
// Queremos 200 amostras/seg, mas ao se escolher N canais, cada canal terá 200/N Hz.
#define BASE_SPS   200.0f

// Clock do ADC e Timer (exemplo assume ACLK=32768 Hz ou SMCLK=1 MHz, 
// ajustado conforme seu hardware):
// Supondo ACLK=32768 Hz, podemos usar esse timer em modo Up para gerar a taxa de amostragem.
#define ACLK_FREQ  32768UL

//==================================================
// Variáveis Globais
//==================================================
volatile char  rxBuffer[BUFSIZE];
volatile int   rxCount      = 0;
volatile int   rxReady      = 0;
volatile char  rxChar       = 0;

volatile uint16_t adcValues[MAX_CH]; // Armazena as leituras (12 bits) de cada canal
volatile uint8_t  numChannels = 1;   // Número de canais ativo (1..8), inicia em 1

// Buffer para formatação
char stringOut[64];

//==================================================
// Protótipos de Funções
//==================================================
void config_WDT(void);
void config_Clock(void);
void config_UART(void);
void config_ADC(void);
void config_TimerA(void);
void update_TimerA_for_Nchannels(uint8_t N);

void uartPrint(char *str);
void showAllChannelsUART(void);

// LCD: aqui apenas um stub; implemente conforme seu driver/MCU
void config_LCD(void);
void lcdClear(void);
void lcdWriteString(char *s);
void lcdShowChannel0(uint16_t value);

//==================================================
// Main
//==================================================
int main(void)
{
    config_WDT();
    config_Clock();
    config_UART();
    config_ADC();
    config_TimerA();
    config_LCD();

    // Mensagem inicial no terminal
    uartPrint(TERM_CLR);
    uartPrint("Osciloscopio Digital - MSP430\n\r");
    uartPrint("Digite um numero (1..8) para configurar canais:\n\r");

    __enable_interrupt(); // Habilita interrupções gerais

    while (1)
    {
        // Se chegou uma reconfiguracao via UART (recebeu um digito 1..8), atualiza
        if (rxReady)
        {
            rxReady = 0;  // Zera flag
            // Verifica se rxChar esta entre '1' e '8'
            if (rxChar >= '1' && rxChar <= '8')
            {
                numChannels = rxChar - '0'; // Converte char em inteiro 1..8
                update_TimerA_for_Nchannels(numChannels);

                uartPrint("\n\r*** Numero de canais atualizado para: ");
                sprintf(stringOut, "%d ***\n\r", numChannels);
                uartPrint(stringOut);

                // Limpa a tela do terminal se desejar
                // uartPrint(TERM_CLR);
            }
            else
            {
                // Caracter não esperado, ignore ou avise
            }
        }

        // Neste exemplo simples, a captura do ADC está sendo disparada pelo Timer
        // e tratada via interrupção do ADC. Sempre que o ADC terminar a sequência,
        // podemos imprimir ou não. Aqui, fazemos a impressão "contínua" (polling)
        // ou poderíamos fazer a impressão no final da interrupção do ADC.

        // Exemplo: mostra todos os canais no UART
        showAllChannelsUART();

        // No LCD, apenas canal A0
        lcdShowChannel0(adcValues[0]);
        
        // Simples atraso ou uso de low-power mode
        __delay_cycles(100000); // Ajuste se quiser diminuir a frequência de impressão
    }
}

//==================================================
// Configura Watchdog (desligado)
//==================================================
void config_WDT(void)
{
    WDTCTL = WDTPW | WDTHOLD;  // Para o WDT
}

//==================================================
// (Exemplo) Configura Clock principal/SMCLK/ACLK
// - Ajuste conforme a placa e a frequência desejada
//==================================================
void config_Clock(void)
{
    // Aqui deixamos o default do DCO ~1 MHz (ou 1.048 MHz) 
    // e ACLK = REFO ~32768 Hz, dependendo do MSP430.
    // Em muitos MSP430, isso já basta para 19200 bps.
    // Se necessário, ajuste para garantir exatidão da UART.
}

//==================================================
// Configuração da UART em 19200 bps
// - Exemplo usando UCA1, SMCLK ~1MHz
//==================================================
void config_UART(void)
{
    // Coloca em reset
    UCA1CTL1 |= UCSWRST;

    // Fonte de clock = SMCLK
    UCA1CTL1 |= UCSSEL__SMCLK;

    // Para ~1.048 MHz e 19200 bps, valores aproximados:
    // Baud Rate = 19200
    // N = 1,048,576 / 19200 ~ 54.6
    // BR = 54, resto => UCBRS ~ 0.5..0.6
    UCA1BR0 = 54;        // parte inteira
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS_5;  // Ajuste fino (pode variar dependendo do MCU)

    // Mapeia pinos TX/RX (ex: P4.4 = TX, P4.5 = RX) - Ajustar conforme LaunchPad
    P4SEL |= BIT4 | BIT5; 
    
    // Tira do reset
    UCA1CTL1 &= ~UCSWRST;

    // Habilita interrupção de RX
    UCA1IE |= UCRXIE;
}

//==================================================
// Função de Envio (Polling)
//==================================================
void uartPrint(char *str)
{
    while (*str)
    {
        while (!(UCA1IFG & UCTXIFG)); // Espera buffer livre
        UCA1TXBUF = *str++;
    }
}

//==================================================
// Mostra todos os canais (numChannels) no formato "c: XXX"
//==================================================
void showAllChannelsUART(void)
{
    uint8_t i;
    for (i = 0; i < numChannels; i++)
    {
        // Formato:  i: XYZ  (XYZ em hex de 3 dígitos)
        sprintf(stringOut, "%d: %03x  ", i, (adcValues[i] & 0x0FFF));
        uartPrint(stringOut);
    }
    uartPrint(NEWLINE);
}

//==================================================
// Configura ADC12 para ler até 8 canais (A0..A7) 
// em modo SEQUENCE OF CHANNELS, 12 bits
// Disparo pelo TimerA (TA0.1 ou TA0.0), p.ex. SHS_1
//==================================================
void config_ADC(void)
{
    // Desabilita para configurar
    ADC12CTL0 &= ~ADC12ENC;

    // Liga o ADC, amostra e conversão
    // SHT0_2 => Sample and Hold Time 16 ciclos, ADC12ON => Liga ADC
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;

    // Modo de conversão sequencial repetitivo (CONSEQ_1 ou CONSEQ_3)
    // Escolha: CONSEQ_1 => sequencial a partir do canal que setamos, com SHS_1 => TA0.1
    // SSEL_0 => ADC12CLK = MODOSC ou SSEL_3 => SMCLK (depende do MSP430)
    // Exemplo: usaremos a fonte interna do ADC (MODOSC) ou SMCLK, tanto faz, 
    // mas iremos disparar a amostra pelo timer (SHS_1).
    ADC12CTL1 = ADC12CONSEQ_1       // sequencial, single-sequence
              | ADC12SHS_1         // sample trigger: TA0.1
              | ADC12SSEL_0        // clk = ADC12OSC
              | ADC12CSTARTADD_0;  // start from MEM0

    // Resolução 12 bits
    ADC12CTL2 = ADC12RES_2;

    // Configura cada MCTL (MEM0..MEM7) para canais A0..A7
    // SREF_0 => Referência = AVCC
    // EOS no último canal que quisermos
    ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_0;  // A0
    ADC12MCTL1 = ADC12SREF_0 | ADC12INCH_1;  // A1
    ADC12MCTL2 = ADC12SREF_0 | ADC12INCH_2;  // A2
    ADC12MCTL3 = ADC12SREF_0 | ADC12INCH_3;  // A3
    ADC12MCTL4 = ADC12SREF_0 | ADC12INCH_4;  // A4
    ADC12MCTL5 = ADC12SREF_0 | ADC12INCH_5;  // A5
    ADC12MCTL6 = ADC12SREF_0 | ADC12INCH_6;  // A6
    // No canal final, colocar EOS:
    ADC12MCTL7 = ADC12SREF_0 | ADC12INCH_7 | ADC12EOS; // A7, end of sequence

    // Habilita interrupção no canal final da sequência (MEM7).
    // Contudo, se quisermos ler menos de 8 canais, ainda assim
    // a interrupção dispara ao chegar no MEM7. 
    // (Outra estratégia seria reconfigurar MCTLx a cada vez que muda N)
    ADC12IE = ADC12IE7;

    // Configura P6.0..P6.7 como entradas analógicas se necessário
    // Exemplo: P6SEL |= 0xFF; // Depende do MSP430 e do mapeamento exato
    P6SEL |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

    // Libera o ADC
    ADC12CTL0 |= ADC12ENC;
}

//==================================================
// Configura TimerA para ~200 Hz inicialmente, 
// mas quem dispara o ADC é o canal CCR1 (por ex. OUTMOD_3 => set/reset).
//==================================================
void config_TimerA(void)
{
    // Timer A0: ACLK = 32768 Hz (exemplo)
    // Precisamos de 200 Hz => período = 32768/200 ~ 163.84
    // CCR0 = 163 (aprox) => freq ~200 Hz
    TA0CTL = TASSEL_1 | MC_1 | TACLR; // ACLK, modo Up
    TA0CCR0 = 163;                    // ~200 Hz
    // Usar OUTMOD_3 em TA0.1 para gerar pulso -> SHS_1 no ADC
    TA0CCTL1 = OUTMOD_3;
    TA0CCR1  = 1;   // Pulso pequeno, mas suficiente para o ADC amostrar
}

//==================================================
// Atualiza TimerA para nova taxa de amostragem
//   Se o usuário escolheu N canais, a frequência do Timer deve ser 200/N
//   Período = ACLK_FREQ / (200/N) = (ACLK_FREQ * N) / 200
//==================================================
void update_TimerA_for_Nchannels(uint8_t N)
{
    uint32_t ticks;

    // Evitar N=0
    if (N == 0) N = 1;
    // Cálculo de ticks para (200/N) Hz
    // freq = 200/N => período = 1/(200/N) => N/200
    // Ticks = ACLK_FREQ * (N/200) = (ACLK_FREQ*N)/200
    ticks = ((uint32_t)ACLK_FREQ * (uint32_t)N) / 200UL;

    // Se exceder 65535, precisa outro modo ou prescaler,
    // mas para N <=8 e 32768 Hz, max = (32768*8)/200 ~1310, que cabe em 16 bits
    if (ticks < 1) ticks = 1; // para evitar zero
    if (ticks > 65535) ticks = 65535;

    // Para ajustar sem parar completamente o timer:
    TA0CTL = TASSEL_1 | MC_1 | TACLR;
    TA0CCR0 = (uint16_t)(ticks - 1);

    // Mantém o TA0.1 com OUTMOD_3 gerando pulso
    TA0CCTL1 = OUTMOD_3;
    // Podemos deixar CCR1 fixo como 1. Assim, a cada ciclo, 
    // geramos um pulso estreito e isso dispara o ADC.
    TA0CCR1  = 1;
}

//==================================================
// Interrupção da UART (Recepção de caractere)
// - Guarda em rxChar e sinaliza rxReady
//==================================================
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    if (UCA1IFG & UCRXIFG)
    {
        rxChar = UCA1RXBUF;
        rxReady = 1;
    }
}

//==================================================
// Interrupção do ADC12
// - Quando finaliza a sequência (MEM7 com EOS), cai aqui
// - Lê todos os 8 valores, mas só "usa" até numChannels
//==================================================
#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    if (ADC12IV == ADC12IV_ADC12IFG7)
    {
        // Leitura dos 8 canais do MEM0..MEM7
        // Se numChannels < 8, ainda assim lemos todos, mas só "usamos" 
        // os primeiros numChannels nos prints.
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

//==================================================
// (Exemplo) Configura LCD (Stub)
//==================================================
void config_LCD(void)
{
    // Implementar conforme seu display (p.ex. via I2C, via pinos paralelos, etc.)
    // Aqui apenas um "stub".
}
void lcdClear(void) { /* Stub */ }
void lcdWriteString(char *s) { /* Stub */ }
void lcdShowChannel0(uint16_t value)
{
    // Converte em volts ou mostra apenas a contagem
    // Exemplo de string:
    char buf[16];
    // 12 bits => 0..4095, se Vref=3.3V => valor em Volts = (value * 3.3 / 4095)
    float volts = (float)value * 3.3f / 4095.0f;
    sprintf(buf, "A0=%4.2fV", volts);
    // Stub de escrita no LCD
    lcdClear();
    lcdWriteString(buf);
}

