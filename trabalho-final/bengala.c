#include <msp430.h>
#include <stdint.h>
#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"

#define POTENTIOMETER_PIN 1

volatile uint16_t potValue;
uint16_t distMili;

/**
 * @brief Configura o Timer B0 para gerar PWM no pino P3.6 (TB0.6).
 */
static void configPWM(void);

/**
 * @brief Configura a interface I2C para comunicação com o sensor VL53L0X.
 */
static void configI2C(void);

/**
 * @brief Lê um valor do ADC no pino especificado.
 *
 * @param pin Pino analógico a ser lido.
 * @return Valor convertido do ADC.
 */
uint16_t ADCRead(uint8_t pin);

/**
 * @brief Gera um atraso em milissegundos.
 *
 * @param ms Tempo de atraso em milissegundos.
 */
void delay_ms(uint16_t ms);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    configPWM();
    configI2C();

    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    VL53L0X_Dev_t dev;
    dev.I2cDevAddr = 0x29;

    VL53L0X_DataInit(&dev);
    VL53L0X_StaticInit(&dev);

    VL53L0X_PerformRefCalibration(&dev, &VhvSettings, &PhaseCal);
    VL53L0X_PerformRefSpadManagement(&dev, &refSpadCount, &isApertureSpads);

    VL53L0X_SetDeviceMode(&dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
    VL53L0X_SetLimitCheckValue(&dev,
                               VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                               (FixPoint1616_t)(1.5 * 0.023 * 65536));

    FixPoint1616_t LimitCheckCurrent;
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;

    while (1)
    {
        potValue = ADCRead(POTENTIOMETER_PIN);
        if (potValue > 4000)
            potValue = 4000;
        uint16_t max_duty = potValue + 1000;

        VL53L0X_PerformSingleRangingMeasurement(&dev, &RangingMeasurementData);
        VL53L0X_GetLimitCheckCurrent(&dev,
                                     VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                                     &LimitCheckCurrent);

        distMili = RangingMeasurementData.RangeMilliMeter;

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

        TB0CCR6 = duty;
        __delay_cycles(20000);
    }

    return 0;
}

uint16_t ADCRead(uint8_t pin)
{
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_3 | ADC12ON;
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_0;
    ADC12CTL2 = ADC12RES_2;

    P6SEL |= (1 << pin);
    ADC12MCTL0 = pin;

    ADC12CTL0 |= ADC12ENC;
    ADC12CTL0 |= ADC12SC;

    while (!(ADC12IFG & BIT0))
        ;

    return ADC12MEM0;
}

void delay_ms(uint16_t ms)
{
    while (ms--)
    {
        __delay_cycles(1000);
    }
}

static void configPWM(void)
{
    P3DIR |= BIT6;
    P3SEL |= BIT6;

    TB0CTL = TBSSEL__SMCLK | MC__UP | TBCLR;
    TB0CCR0 = 10000;
    TB0CCTL6 = OUTMOD_7;
    TB0CCR6 = 0;
}

static void configI2C(void)
{
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0CTL1 = UCSSEL__SMCLK | UCSWRST;
    UCB0BRW = 10;
    P3SEL |= BIT0 | BIT1;
    UCB0CTL1 &= ~UCSWRST;
}