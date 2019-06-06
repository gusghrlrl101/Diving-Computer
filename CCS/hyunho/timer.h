#ifndef TIMER_H_
#define TIMER_H_

#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"

inline void timer0_enable(void);
inline void timer0_disable(void);

volatile unsigned int tick_1ms = 0;
volatile unsigned int tick2_1ms = 0;
volatile unsigned int released = 0;

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

#pragma vector=RTC_C_VECTOR
__interrupt void rtc_interrupt(void)
{
    // 1 minute interrupt
    if (RTCCTL0_L & RTCTEVIFG)
    {
        RTCCTL0_L &= ~RTCTEVIFG;
        if (mode == MOD_WATER)
        {
            clear_display();
            make_text_water1();
            show(text_water1);
            nextline();
            show(text_water2);
        }
    }
}

#endif /* TIMER_H_ */
