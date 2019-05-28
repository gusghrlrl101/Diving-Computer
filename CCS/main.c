#include "msp430fr5994.h"

void timer0_init(void);
void switch_init(void);

volatile unsigned int tick_1ms = 0;
volatile unsigned int delay_1ms = 0;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

    // LED OUTPUT
    P1DIR |= BIT2;
    P1DIR |= BIT3;

    switch_init();
    timer0_init();
    __bis_SR_register(GIE);

    while (1);
}

void timer0_init(void)
{
    TA0CCTL0 = CCIE;
    TA0CCR0 = 999;
    TA0CTL = TASSEL_2 + MC_2;
}

void switch_init(void)
{
    P5DIR &= ~BIT7;
    P5SEL1 &= ~BIT7;
    P5SEL0 &= ~BIT7;

    P5IE |= BIT7;
    P5IES &= ~BIT7;
    P5IFG &= ~BIT7;
}

void delay(int time)
{
    TA1CTL = TASSEL_2 + MC_2;
    TA1CCTL0 = CCIE;
    TA1CCR0 = 999;

    delay_1ms = 0;
    while (delay_1ms < time);
    TA1CTL = MC_0;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void _tick_1sec(void)
{
    if (++tick_1ms >= 1000)
    {
        // tick
        P1OUT ^= BIT3;

        tick_1ms = 0;
    }
    TA0CCR0 += 999;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void _delay(void)
{
    delay_1ms++;
    TA1CCR0 += 999;
}

#pragma vector=PORT5_VECTOR
__interrupt void _switch1(void)
{
    P5IFG &= ~BIT7;
    P1OUT ^= BIT2;
}
