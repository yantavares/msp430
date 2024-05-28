#include <msp430.h>

void enter_low_power_mode() {
    __bis_SR_register(LPM4_bits); // Enter low-power mode 4
}

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	enter_low_power_mode();

	return 0;
}
