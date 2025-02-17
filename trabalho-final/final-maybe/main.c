#include <msp430.h>
#include <stdint.h>
#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"

// Definições
#define POTENTIOMETER_PIN 1    // P6.1 para o potenciômetro

// Variáveis globais
volatile uint16_t potValue;
uint16_t distMili;

// Protótipos de funções
static void configPWM(void);
uint16_t ADCRead(uint8_t pin);
void delay_ms(uint16_t ms);

int main(void)
{
    // -----------------------------------------------------------------------
    // Parar o Watchdog Timer
    // -----------------------------------------------------------------------
    WDTCTL = WDTPW | WDTHOLD;

    // -----------------------------------------------------------------------
    // Configurar Timer B0 -> P3.6 para PWM
    // -----------------------------------------------------------------------
    configPWM(); // Configura PWM no pino P3.6 (TB0.6)

    // -----------------------------------------------------------------------
    // Configuração da I2C para o VL53L0X (usando UCB0)
    // -----------------------------------------------------------------------
    UCB0CTL1 |= UCSWRST;                        // Coloca eUSCI_B em reset
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;         // Modo I2C, master, síncrono
    UCB0CTL1 = UCSSEL__SMCLK | UCSWRST;           // Usa SMCLK como clock
    UCB0BRW = 10;                               // Configura clock I2C (~100 kHz se SMCLK = 1MHz)
    P3SEL |= BIT0 | BIT1;                         // P3.0 (SDA) e P3.1 (SCL) para I2C
    UCB0CTL1 &= ~UCSWRST;                         // Libera eUSCI_B do reset

    // -----------------------------------------------------------------------
    // Configuração do sensor VL53L0X
    // -----------------------------------------------------------------------
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    VL53L0X_Dev_t dev;
    dev.I2cDevAddr = 0x29;

    VL53L0X_DataInit(&dev);
    VL53L0X_StaticInit(&dev);

    // Calibração & Gerenciamento dos SPADs
    VL53L0X_PerformRefCalibration(&dev, &VhvSettings, &PhaseCal);
    VL53L0X_PerformRefSpadManagement(&dev, &refSpadCount, &isApertureSpads);

    // Configura o sensor para modo Single Ranging
    VL53L0X_SetDeviceMode(&dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
    VL53L0X_SetLimitCheckValue(&dev,
                               VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                               (FixPoint1616_t)(1.5 * 0.023 * 65536));

    FixPoint1616_t LimitCheckCurrent;
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;

    // -----------------------------------------------------------------------
    // Loop principal
    // -----------------------------------------------------------------------
    while (1)
    {
        // Lê o valor do potenciômetro (esperado de 0 a 4000)
        potValue = ADCRead(POTENTIOMETER_PIN);
        if (potValue > 4000)
            potValue = 4000;
        // Calcula o duty cycle máximo:
        // Usando a relação: potValue + 1000
        // Ex.: 4000 -> 5000 (50%), 3000 -> 4000 (40%)
        uint16_t max_duty = potValue + 1000;

        // Realiza a medição única do sensor VL53L0X
        VL53L0X_PerformSingleRangingMeasurement(&dev, &RangingMeasurementData);
        VL53L0X_GetLimitCheckCurrent(&dev,
                                     VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                                     &LimitCheckCurrent);

        distMili = RangingMeasurementData.RangeMilliMeter;

        // -------------------------------------------------------------------
        // Mapeamento da distância para o duty cycle do motor vibracall:
        //  - Se dist > 300mm: motor OFF (duty = 0)
        //  - Se dist < 50mm:  motor em intensidade máxima (duty = max_duty)
        //  - Entre 50mm e 300mm: interpolação linear
        //     duty = max_duty * (300 - distMili) / 250
        // -------------------------------------------------------------------
        uint16_t duty;
        if (distMili > 300)
        {
            duty = 0;
        }
        else if (distMili < 50)
        {
            duty = max_duty;
        }
        else
        {
            duty = (max_duty * (300 - distMili)) / 250;
        }

        TB0CCR6 = duty;  // Atualiza o duty cycle do PWM

        // Delay curto para não sobrecarregar as leituras
        __delay_cycles(20000);
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Função para ler um valor do ADC (usando ADC12) no pino especificado
// ---------------------------------------------------------------------------
uint16_t ADCRead(uint8_t pin)
{
    ADC12CTL0 &= ~ADC12ENC;                // Desabilita conversões
    ADC12CTL0 = ADC12SHT0_3 | ADC12ON;       // Configura tempo de amostragem e liga o ADC
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_0;    // Modo de conversão única, canal único com pulso de amostragem
    ADC12CTL2 = ADC12RES_2;                // Resolução de 12 bits

    // Configura o pino P6.x como entrada analógica
    P6SEL |= (1 << pin);
    ADC12MCTL0 = pin;                      // Seleciona o canal ADC correspondente

    ADC12CTL0 |= ADC12ENC;                 // Habilita conversões
    ADC12CTL0 |= ADC12SC;                  // Inicia a conversão

    while (!(ADC12IFG & BIT0));            // Aguarda término da conversão

    return ADC12MEM0;                      // Retorna o valor convertido
}

// ---------------------------------------------------------------------------
// Função de delay (assumindo clock de 1MHz)
// ---------------------------------------------------------------------------
void delay_ms(uint16_t ms)
{
    while(ms--)
    {
        __delay_cycles(1000);
    }
}

// ---------------------------------------------------------------------------
// Configura Timer B0 para PWM no pino **P3.6** (TB0.6)
// ---------------------------------------------------------------------------
static void configPWM(void)
{
    // Configura P3.6 como saída para a função PWM do Timer B0 CCR6
    P3DIR |= BIT6;   // Define P3.6 como saída
    P3SEL |= BIT6;   // Habilita função alternativa (PWM via Timer B0)

    // Configura o Timer B0:
    // - Fonte de clock: SMCLK
    // - Modo UP (conta de 0 até TB0CCR0)
    // - TB0CCR0 define o período do PWM (aqui 10000 → ~100 Hz se SMCLK = 1MHz)
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBCLR;
    TB0CCR0 = 10000;        // Período do PWM
    TB0CCTL6 = OUTMOD_7;    // Modo Reset/Set
    TB0CCR6 = 0;            // Inicia com duty cycle = 0 (motor desligado)
}
