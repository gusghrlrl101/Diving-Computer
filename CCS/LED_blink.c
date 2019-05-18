#include <msp430.h>

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;       // stop watchdog timer
    P1DIR |= BIT0;                  // configure P1.0(LED on board) as output

    volatile unsigned int i;        // volatile to prevent optimization

    while(1)
    {
        P1OUT ^= BIT0;              // toggle P1.0(LED on board)
        for(i=10000; i>0; i--);     // delay
    }
}