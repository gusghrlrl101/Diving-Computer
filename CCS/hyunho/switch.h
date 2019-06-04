#ifndef SWITCH_H_
#define SWITCH_H_

#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"

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

    // 5.0 -> switch3
    P5DIR &= ~BIT0;
    P5SEL1 &= ~BIT0;
    P5SEL0 &= ~BIT0;
    P5IE |= BIT0;
    P5IES &= ~BIT0;
    P5IFG &= ~BIT0;
}

#pragma vector=PORT5_VECTOR
__interrupt void _switch1(void)
{
    __disable_interrupt();
    // switch 1 (up)
    if (P5IFG & BIT3)
    {
        P5IFG &= ~BIT3;
        P1DIR ^= BIT2;

        if (mode == MOD_LOG && *log_size_addr > 0)
        {
            if (++log_num == *log_size_addr)
                log_num = 0;

            make_text_log();
            clear_display();
            show(text_log1);
            nextline();
            show(text_log2);
        }
    }
    // switch 2 (down)
    else if (P5IFG & BIT7)
    {
        P5IFG &= ~BIT7;
        P1DIR ^= BIT2;

        if (mode == MOD_LOG && *log_size_addr > 0)
        {
            if (log_num-- == 0)
                log_num = *log_size_addr - 1;

            clear_display();
            make_text_log();
            show(text_log1);
            nextline();
            show(text_log2);
        }
    }
    // switch 3 (change)
    else if (P5IFG & BIT0)
    {
        P5IFG &= ~BIT0;
        P1DIR ^= BIT2;

        if (mode == MOD_LOG)
        {
            mode = MOD_WATER;
            log_num = 0;

            clear_display();
            show(text_water1);
            nextline();
            show(text_water2);
        }
        else if (mode == MOD_WATER)
        {
            mode = MOD_LOG;
            log_num = 0;

            if (*log_size_addr == 0)
                show(text_log5);
            else
            {
                clear_display();
                make_text_log();
                show(text_log1);
                nextline();
                show(text_log2);
            }
        }
    }

    __enable_interrupt();
}


#endif /* SWITCH_H_ */
