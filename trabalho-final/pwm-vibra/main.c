#include <msp430.h>
#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"

uint16_t distMili;

static void configPWM(void);

int main(void)
{
    // -----------------------------------------------------------------------
    // Stop Watchdog
    // -----------------------------------------------------------------------
    WDTCTL = WDTPW | WDTHOLD;

    // -----------------------------------------------------------------------
    // Configurar Timer B0 -> P3.6 para PWM
    // -----------------------------------------------------------------------
    configPWM(); // Configura Timer B0 para PWM em P3.6 (TB0.6)

    // -----------------------------------------------------------------------
    // I2C Setup para VL53L0X usando UCB0
    // -----------------------------------------------------------------------
    UCB0CTL1 |= UCSWRST;  // Coloca eUSCI_B em reset
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;  // Modo I2C, master, sync
    UCB0CTL1 = UCSSEL__SMCLK | UCSWRST;    // Usa SMCLK como clock
    UCB0BRW = 10;  // Define Clock I2C (SMCLK/10 ~ 100 kHz)
    P3SEL |= BIT0 | BIT1;  // P3.0 (SDA), P3.1 (SCL) para I2C
    UCB0CTL1 &= ~UCSWRST;  // Libera eUSCI_B do reset

    // -----------------------------------------------------------------------
    // VL53L0X Configuration
    // -----------------------------------------------------------------------
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    VL53L0X_Dev_t dev;
    dev.I2cDevAddr = 0x29;

    VL53L0X_DataInit(&dev);
    VL53L0X_StaticInit(&dev);

    // Calibração & SPAD
    VL53L0X_PerformRefCalibration(&dev, &VhvSettings, &PhaseCal);
    VL53L0X_PerformRefSpadManagement(&dev, &refSpadCount, &isApertureSpads);

    // Configura sensor VL53L0X para Single Ranging
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
        // Faz leitura única do sensor
        VL53L0X_PerformSingleRangingMeasurement(&dev, &RangingMeasurementData);
        VL53L0X_GetLimitCheckCurrent(&dev,
                                     VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                                     &LimitCheckCurrent);

        distMili = RangingMeasurementData.RangeMilliMeter;

        // -------------------------------------------------------------------
        // Mapeamento distância -> duty cycle do motor vibracall
        //  - Se dist > 300mm: motor OFF (CCR6 = 0)
        //  - Se dist < 50mm:  motor 50% (CCR6 = 5000)
        //  - Entre 50..300mm: interpolação linear (de 0% até 50%)
        // -------------------------------------------------------------------
        if (distMili > 300)
        {
            TB0CCR6 = 0;  // 0% duty cycle -> motor desligado
        }
        else if (distMili < 50)
        {
            TB0CCR6 = 5000; // 50% duty cycle máximo
        }
        else
        {
            // Interpola linearmente [50mm..300mm] -> [50%..0%]
            // TB0CCR6 = 5000 - (5000/250)*(distMili-50)
            TB0CCR6 = 5000 - (20 * (distMili - 50));
        }

        // Delay curto para evitar leitura excessivamente rápida
        __delay_cycles(20000);
    }
}

// ---------------------------------------------------------------------------
// Configura Timer B0 para PWM no pino **P3.6** (TB0.6)
// ---------------------------------------------------------------------------
static void configPWM(void)
{
    // 1) Configura P3.6 como saída de PWM do Timer B0 CCR6
    P3DIR |= BIT6;   // P3.6 como saída
    P3SEL |= BIT6;   // Habilita função alternativa (Timer B0)

    // 2) Configura Timer B0 para gerar PWM
    TB0CTL = TBSSEL__SMCLK  // Usa SMCLK como clock
           | MC__UP        // Modo UP (conta até TB0CCR0)
           | TBCLR;        // Limpa o contador

    TB0CCR0 = 10000;        // Período PWM (~100 Hz se SMCLK=1 MHz)
    TB0CCTL6 = OUTMOD_7;    // Modo Reset/Set
    TB0CCR6 = 0;            // Começa com duty cycle = 0% (motor OFF)
}
