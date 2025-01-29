#include <msp430.h>
#include <stdint.h>

#define PINx 1
#define PINy 6

uint16_t ADCRead(uint8_t pin);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;

	volatile uint16_t x, y;

	while(1){
 	     x = ADCRead(PINx);
	     y = ADCRead(PINy);

	}

	return 0;
}


uint16_t ADCRead(uint8_t pin){
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_3 | ADC12ON;
    ADC12CTL1 = ADC12SHS_0 | ADC12SHP | ADC12CONSEQ_0;
    ADC12CTL2 = ADC12RES_2;

    P6SEL |= 1 << pin;
    ADC12MCTL0 = pin;
    ADC12CTL0 |= ADC12ENC;
    ADC12CTL0 |= ADC12SC;
    ADC12CTL0 &= ~ADC12SC;

    while(!(ADC12IFG & BIT0));

    return ADC12MEM0;
}
