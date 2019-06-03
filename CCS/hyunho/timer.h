#ifndef TIMER_H_
#define TIMER_H_

#include "msp430fr5994.h"
#include "mem.h"

void timer0_init(void);
inline void delay(int time);
void delay2(int time);

volatile unsigned int tick_1ms = 0;
volatile unsigned int delay_1ms = 0;


void timer0_init(void)
{
    TA0CCTL0 = CCIE;
    TA0CCR0 = 999;
    TA0CTL = TASSEL_2 + MC_2;
}

inline void delay(int time)
{
    int i = time;
    while (i-- > 0)
        __delay_cycles(1000);
}

void delay2(int time)
{
    TA1CCR0 = 999;
    TA1CTL = TASSEL_2 + MC_2;
    TA1CCTL0 = CCIE;

    delay_1ms = time;
    while (delay_1ms > 0)
        ;
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
    delay_1ms--;
    TA1CCR0 += 999;
}


#endif /* TIMER_H_ */
