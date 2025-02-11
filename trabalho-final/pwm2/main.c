#include <msp430.h>
#include <stdint.h>

#define SLAVE_ADDR_VL53L0X 0x29
#define SYSRANGE_START 0x00
#define RESULT_RANGE_STATUS 0x14
#define LED_PIN BIT0
#define XSHUT_PIN BIT6

#define I2C_TIMEOUT 10000

void initI2C(void);
void initVL53L0X(void);
unsigned int readDistance(void);
void delay_ms(unsigned int ms);
void writeRegister(uint8_t reg, uint8_t value);
unsigned char readRegister(uint8_t reg);

volatile unsigned int distance = 0;

int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED_PIN;
    P1OUT &= ~LED_PIN;

    P1DIR |= XSHUT_PIN;
    P1OUT &= ~XSHUT_PIN;
    delay_ms(10);
    P1OUT |= XSHUT_PIN;
    delay_ms(10);

    initI2C();
    initVL53L0X();

    while (1)
    {
        distance = readDistance();

        if (distance == 0 || distance == 0xFFFF || distance > 2000)
        {
            P1OUT &= ~LED_PIN;
        }
        else if (distance < 500)
        {
            P1OUT |= LED_PIN;
        }
        else
        {
            P1OUT &= ~LED_PIN;
        }

        delay_ms(50);
    }
}

void initI2C(void)
{

    P3SEL |= BIT0 | BIT1;

    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0BRW = 10;
    UCB0I2CSA = SLAVE_ADDR_VL53L0X;

    UCB0CTL1 = UCSSEL_2 | UCSWRST;
    UCB0CTL1 &= ~UCSWRST;
}

void initVL53L0X(void)
{
    delay_ms(10);
    writeRegister(SYSRANGE_START, 0x01);
}

unsigned int readDistance(void)
{
    unsigned char msb, lsb;
    unsigned int dist;
    volatile unsigned int timeout = I2C_TIMEOUT;

    while (((readRegister(RESULT_RANGE_STATUS) & 0x01) == 0) && --timeout)
    {
        delay_ms(1);
    }
    if (timeout == 0)
    {
        return 0xFFFF;
    }

    msb = readRegister(0x1E);
    lsb = readRegister(0x1F);
    dist = (msb << 8) | lsb;

    writeRegister(SYSRANGE_START, 0x01);

    return dist;
}

void delay_ms(unsigned int ms)
{
    while (ms--)
    {
        __delay_cycles(1000);
    }
}

void writeRegister(uint8_t reg, uint8_t value)
{
    while (UCB0CTL1 & UCTXSTP)
        ;

    UCB0CTL1 |= UCTR | UCTXSTT;
    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0TXBUF = reg;

    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0TXBUF = value;

    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0CTL1 |= UCTXSTP;
}

unsigned char readRegister(uint8_t reg)
{
    volatile unsigned int timeout = I2C_TIMEOUT;

    while (UCB0CTL1 & UCTXSTP)
        ;

    UCB0CTL1 |= UCTR | UCTXSTT;
    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0TXBUF = reg;
    while (!(UCB0IFG & UCTXIFG))
        ;

    UCB0CTL1 &= ~UCTR;
    UCB0CTL1 |= UCTXSTT;

    while (UCB0CTL1 & UCTXSTT)
        ;
    UCB0CTL1 |= UCTXSTP;

    while (!(UCB0IFG & UCRXIFG) && --timeout)
        ;
    if (timeout == 0)
    {

        return 0xFF;
    }
    return UCB0RXBUF;
}
