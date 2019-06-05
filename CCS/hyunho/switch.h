#ifndef SWITCH_H_
#define SWITCH_H_

#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"

void switch_init(void);

void switch_init(void)
{
    // 6.1 -> switch1
    P6DIR &= ~BIT1;
    P6SEL1 &= ~BIT1;
    P6SEL0 &= ~BIT1;
    P6IE |= BIT1;
    P6IES ^= BIT1;
    P6IFG &= ~BIT1;

    // 6.0 -> switch2
    P6DIR &= ~BIT0;
    P6SEL1 &= ~BIT0;
    P6SEL0 &= ~BIT0;
    P6IE |= BIT0;
    P6IES ^= BIT0;
    P6IFG &= ~BIT0;

    // 3.7 -> switch3
    P3DIR &= ~BIT7;
    P3SEL1 &= ~BIT7;
    P3SEL0 &= ~BIT7;
    P3IE |= BIT7;
    P3IES ^= BIT7;
    P3IFG &= ~BIT7;

    // 3.6 -> switch4
    P3DIR &= ~BIT6;
    P3SEL1 &= ~BIT6;
    P3SEL0 &= ~BIT6;
    P3IE |= BIT6;
    P3IES ^= BIT6;
    P3IFG &= ~BIT6;
}

#pragma vector=PORT6_VECTOR
__interrupt void _switch1(void)
{
    __disable_interrupt();

    // switch 1 (LOG: next, WATER: start)
    if (P6IFG & BIT1)
    {
        P6IFG &= ~BIT1;

        if (mode == MOD_LOG)
        {
            if (*log_size_addr > 0)
            {
                if (++log_num == *log_size_addr)
                    log_num = 0;
                log_page = 0;

                clear_display();
                make_text_log();
                show(text_log1);
                nextline();
                show(text_log2);
            }
        }
        else if (mode == MOD_WATER)
        {
            RTCCTL0_L &= ~RTCTEVIE;
            mode = MOD_GOING;

            minute_water = 0;
            second_water = 0;

            timer0_enable();
        }
        else if (mode == MOD_GOING)
        {
            timer0_disable();
            RTCCTL0_L &= ~RTCTEVIFG;
            RTCCTL0_L |= RTCTEVIE;

            mode = MOD_WATER;

            minute_water = 0;
            second_water = 0;

            clear_display();
            make_text_water();
            show (text_water1);
            nextline();
            show (text_water2);
        }
    }
    // switch 2 (LOG: page change, WATER: stop)
    else if (P6IFG & BIT0)
    {
        P6IFG &= ~BIT0;

        if (mode == MOD_LOG)
        {
            if (*log_size_addr > 0)
            {
                if (log_page == 0)
                {
                    log_page = 1;

                    clear_display();
                    make_text_log();
                    show(text_log3);
                    nextline();
                    show(text_log4);
                }
                else if (log_page == 1)
                {
                    log_page = 0;

                    clear_display();
                    make_text_log();
                    show(text_log1);
                    nextline();
                    show(text_log2);
                }
            }
        }
    }

    __enable_interrupt();
}

#pragma vector=PORT3_VECTOR
__interrupt void _switch2(void)
{
    __disable_interrupt();

    // switch 3 (change mode)
    if (P3IFG & BIT7)
    {
        P3IFG &= ~BIT7;

        if (mode == MOD_LOG)
        {
            RTCCTL0_L &= ~RTCTEVIE;

            mode = MOD_WATER;
            log_num = 0;

            clear_display();
            make_text_water();
            show(text_water1);
            nextline();
            show(text_water2);
        }
        else if (mode == MOD_WATER)
        {
            RTCCTL0_L &= ~RTCTEVIFG;
            RTCCTL0_L |= RTCTEVIE;
            mode = MOD_LOG;
            log_num = 0;
            log_page = 0;

            if (*log_size_addr == 0)
            {
                clear_display();
                show(text_log5);
            }
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
    // switch 4 (LOG: delete log, WATER: backlight LED)
    else if (P3IFG & BIT6)
    {
        P3IFG &= ~BIT6;

        if (mode == MOD_LOG)
        {
            if (*log_size_addr > 0)
            {
                delete_log(log_num);

                if (*log_size_addr == 0)
                {
                    clear_display();
                    show(text_log5);
                }
                else
                {
                    if (log_num == *log_size_addr)
                        log_num--;

                    clear_display();
                    make_text_log();
                    show(text_log1);
                    nextline();
                    show(text_log2);
                }
            }
        }
    }

    __enable_interrupt();
}

#endif /* SWITCH_H_ */
