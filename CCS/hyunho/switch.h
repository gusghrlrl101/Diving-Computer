#ifndef SWITCH_H_
#define SWITCH_H_

#include "msp430fr5994.h"
#include "mem.h"

void switch_init(void);

void switch_init(void)
{
    // 5.3 -> switch1
    P5DIR &= ~BIT3;
    P5SEL1 &= ~BIT3;
    P5SEL0 &= ~BIT3;
    P5IE |= BIT3;
    P5IES &= ~BIT3;
    P5IFG &= ~BIT3;

    // 5.7 -> switch2
    P5DIR &= ~BIT7;
    P5SEL1 &= ~BIT7;
    P5SEL0 &= ~BIT7;
    P5IE |= BIT7;
    P5IES &= ~BIT7;
    P5IFG &= ~BIT7;
}

#pragma vector=PORT5_VECTOR
__interrupt void _switch(void)
{
    // switch 1 (up)
    if (P5IFG & BIT3)
    {
        P5IFG &= ~BIT3;
        P1DIR ^= BIT2;

        if (mode == MOD_LOG)
        {
            if (++log_num == *log_size_addr)
                log_num = 0;

            make_text_log(log_num);
            clear_display();
            show (text1);
            nextline();
            show (text2);
        }
    }
    // switch 2 (down)
    if (P5IFG & BIT7)
    {
        P5IFG &= ~BIT7;
        P1DIR ^= BIT2;

        if (mode == MOD_LOG)
        {
            if (log_num-- == 0)
                log_num = *log_size_addr - 1;

            make_text_log(log_num);
            clear_display();
            show (text1);
            nextline();
            show (text2);
        }
    }
}

#endif /* SWITCH_H_ */
