#include <msp430.h>
#include "lcd.h"
#include "i2c.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    USCI_B0_config();
    lcdInit();

    lcdWrite("Ola :)");

    while (lcdBusy())
    {
    }

    lcdWrite("Ready!");

    while (1)
    {

        __delay_cycles(1000000);

        lcdWrite("Lendo...");
        while (lcdBusy())
        {
        }

        uint8_t read = lcdReadByte(1);
    }

    return 0;
}
