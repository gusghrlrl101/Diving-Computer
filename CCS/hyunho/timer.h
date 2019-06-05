#ifndef TIMER_H_
#define TIMER_H_

#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"

inline void timer0_enable(void);
inline void timer0_disable(void);
void delay2(int time);

volatile unsigned int tick_1ms = 0;
volatile unsigned int delay_1ms = 0;

inline void timer0_enable(void)
{
    TA0CCR0 = 999;
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL_2 + MC_2;
    tick_1ms = 0;
}

inline void timer0_disable(void)
{
    TA0CCTL0 &= ~CCIE;
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
        tick_1ms = 0;

        if (mode == MOD_GOING)
        {
            if (++second_water == 60)
            {
                second_water = 0;
                minute_water++;
            }

            clear_display();
            make_text_water();
            show(text_water1);
            nextline();
            show(text_water2);
        }
    }
    TA0CCR0 += 999;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void _delay(void)
{
    delay_1ms--;
    TA1CCR0 += 999;
}

#pragma vector=RTC_C_VECTOR
__interrupt void rtc_interrupt(void)
{
    if (RTCCTL0_L & RTCTEVIFG)
    {
        RTCCTL0_L &= ~RTCTEVIFG;

        P4OUT |= BIT7;
        if (mode == MOD_WATER)
        {
            clear_display();
            make_text_water1();
            show(text_water1);
            nextline();
            show(text_water2);
        }
        P4OUT &= ~BIT7;
    }
}

#endif /* TIMER_H_ */
