#include <msp430.h>
#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"

uint16_t distMili;

static void configPWM(void);

int main(void)
{
    // -----------------------------------------------------------------------
    // Stop Watchdog and configure LED pins
    // -----------------------------------------------------------------------
    WDTCTL = WDTPW | WDTHOLD;

    // If you want to keep P1.0 as a debug LED, you can. (Not strictly needed)
    // P1DIR |= BIT0;

    // -----------------------------------------------------------------------
    // Configure Timer B0 -> P4.7 for PWM
    // -----------------------------------------------------------------------
    configPWM();  // Sets up Timer B0 for PWM at P4.7

    // -----------------------------------------------------------------------
    // I2C Setup for VL53L0X using UCB0
    // -----------------------------------------------------------------------
    // 1) Put eUSCI_B in reset
    UCB0CTL1 |= UCSWRST;

    // 2) I2C mode, master, sync
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    // 3) Use SMCLK as source
    UCB0CTL1 = UCSSEL__SMCLK | UCSWRST;
    // 4) Set I2C clock: SMCLK / 10 => ~100kHz if SMCLK ~1MHz
    UCB0BRW = 10;
    // 5) Pins for I2C: P3.0 (SDA), P3.1 (SCL)
    P3SEL |= BIT0 | BIT1;
    // 6) Release eUSCI_B from reset
    UCB0CTL1 &= ~UCSWRST;

    // -----------------------------------------------------------------------
    // (Optional) UART Setup on USCI_A1 (if you need it)
    // -----------------------------------------------------------------------
    P4SEL |= BIT4 | BIT5;        // P4.4 = TX, P4.5 = RX (for UCA1)
    UCA1BRW  = 54;               // Example ~ 19200 baud
    UCA1MCTL = UCBRS_5;          // Modulation
    UCA1CTL1 = UCSSEL__SMCLK;    // SMCLK as source

    // -----------------------------------------------------------------------
    // VL53L0X Configuration
    // -----------------------------------------------------------------------
    uint32_t refSpadCount;
    uint8_t  isApertureSpads;
    uint8_t  VhvSettings;
    uint8_t  PhaseCal;
    VL53L0X_Dev_t dev;
    dev.I2cDevAddr = 0x29;

    VL53L0X_DataInit(&dev);
    VL53L0X_StaticInit(&dev);

    // Calibration & reference SPAD management
    VL53L0X_PerformRefCalibration(&dev, &VhvSettings, &PhaseCal);
    VL53L0X_PerformRefSpadManagement(&dev, &refSpadCount, &isApertureSpads);

    // Single ranging mode
    VL53L0X_SetDeviceMode(&dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);

    // Enable some limit checks
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    VL53L0X_SetLimitCheckEnable(&dev, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
    VL53L0X_SetLimitCheckValue(&dev,
                               VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                               (FixPoint1616_t)(1.5 * 0.023 * 65536));

    FixPoint1616_t LimitCheckCurrent;
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;

    // -----------------------------------------------------------------------
    // Main Loop
    // -----------------------------------------------------------------------
    while (1)
    {
        // Perform a single measurement
        VL53L0X_PerformSingleRangingMeasurement(&dev, &RangingMeasurementData);
        VL53L0X_GetLimitCheckCurrent(&dev,
                                     VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                                     &LimitCheckCurrent);

        distMili = RangingMeasurementData.RangeMilliMeter;

        // -------------------------------------------------------------------
        // Simple distance-to-brightness mapping:
        // - If dist > 300mm:  LED off (CCR1=0)
        // - If dist < 50mm:   LED max brightness (CCR1 = TB0CCR0)
        // - Else: linear interpolation between those distances
        // -------------------------------------------------------------------
        if (distMili > 300)
        {
            TB0CCR1 = 0;  // 0% duty cycle
        }
        else if (distMili < 50)
        {
            TB0CCR1 = TB0CCR0; // 100% duty cycle
        }
        else
        {
            // Linear map [50..300] -> [100%..0%]
            // Range is 250mm wide; let's define slope
            // slope = (0 - TB0CCR0) / (300 - 50)
            //       = (-TB0CCR0)/250
            // brightness = TB0CCR0 + slope*(distMili-50)
            //            = TB0CCR0 - (TB0CCR0/250)*(distMili-50)
            //
            // For TB0CCR0=10000, slope = -10000/250 = -40
            TB0CCR1 = 10000 - 40 * (distMili - 50);
        }

        // Optional small delay so we don't loop *too* fast
        __delay_cycles(20000);
    }
}

// ---------------------------------------------------------------------------
// Configure Timer B0 for PWM on P4.7
// ---------------------------------------------------------------------------
static void configPWM(void)
{
    // 1) P4.7 as Timer B0 CCR1 output
    P4DIR  |= BIT7;            // P4.7 as output
    P4SEL  |= BIT7;            // Select peripheral
    PMAPKEYID = 0x02D52;       // Unlock PMAP
    P4MAP7 = PM_TB0CCR1A;      // Map TB0CCR1 output to P4.7
    PMAPKEYID = 0;             // Lock PMAP

    // 2) Configure Timer B0
    TB0CTL   = TBSSEL__SMCLK    // SMCLK source
             | MC__UP          // Up mode
             | TBCLR;          // Clear timer
    TB0CCR0  = 10000;          // PWM period (~100Hz if SMCLK=1MHz)
    TB0CCTL1 = OUTMOD_7;       // Reset/Set mode
    TB0CCR1  = 0;              // Start with LED off
}
