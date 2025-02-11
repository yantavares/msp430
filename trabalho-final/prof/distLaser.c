#include <msp430.h>
#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"

uint16_t distMili;

int main(void)
{
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    VL53L0X_Dev_t dev;

    WDTCTL = WDTPW | WDTHOLD;
    P1DIR |= BIT0;

    dev.I2cDevAddr = 0x29;
    UCB0CTL0 = UCMODE_3 | UCMST;
    UCB0CTL1 |= UCSSEL__SMCLK;
    UCB0BRW = 10;
    P3SEL |= BIT0 | BIT1;
    UCB0CTL1 &= ~UCSWRST;

    P4SEL |= BIT4 | BIT5;
    UCA1BRW = 54;
    UCA1MCTL = UCBRS_5;
    UCA1CTL1 = UCSSEL__SMCLK;

    VL53L0X_DataInit(&dev);
    VL53L0X_StaticInit(&dev);
    //
    VL53L0X_PerformRefCalibration(&dev, &VhvSettings, &PhaseCal);
    VL53L0X_PerformRefSpadManagement(&dev, &refSpadCount, &isApertureSpads);
    //
    VL53L0X_SetDeviceMode(&dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);
    //
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
    VL53L0X_SetLimitCheckValue(&dev, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));

    FixPoint1616_t LimitCheckCurrent;
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;


    while (1)
    {
        VL53L0X_PerformSingleRangingMeasurement(&dev, &RangingMeasurementData);
        VL53L0X_GetLimitCheckCurrent(&dev, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, &LimitCheckCurrent);

        distMili = RangingMeasurementData.RangeMilliMeter;

        if (distMili > 50)
            P1OUT &= ~BIT0;
        else
            P1OUT |= BIT0;
    }
}
