#ifndef SWITCH_H_
#define SWITCH_H_

#include "msp430fr5994.h"
#include "mem.h"
#include "timer.h"
#include "lcd.h"

void switch_init(void);

void switch_init(void)
{
    // 6.1 -> switch1
    P6DIR &= ~BIT1;
    P6SEL1 &= ~BIT1;
    P6SEL0 &= ~BIT1;
    P6IFG &= ~BIT1;
    P6IE |= BIT1;
    P6IES |= BIT1;

    // 6.0 -> switch2
    P6DIR &= ~BIT0;
    P6SEL1 &= ~BIT0;
    P6SEL0 &= ~BIT0;
    P6IFG &= ~BIT0;
    P6IE |= BIT0;
    P6IES |= BIT0;

    // 3.7 -> switch3
    P3DIR &= ~BIT7;
    P3SEL1 &= ~BIT7;
    P3SEL0 &= ~BIT7;
    P3IFG &= ~BIT7;
    P3IE |= BIT7;
    P3IES |= BIT7;

    // 3.6 -> switch4
    P3DIR &= ~BIT6;
    P3SEL1 &= ~BIT6;
    P3SEL0 &= ~BIT6;
    P3IFG &= ~BIT6;
    P3IE |= BIT6;
    P3IES |= BIT6;

    // 5.7 -> switch5
    P5DIR &= ~BIT7;
    P5SEL1 &= ~BIT7;
    P5SEL0 &= ~BIT7;
    P5IFG &= ~BIT7;
    P5IE |= BIT7;
    P5IES |= BIT7;
}

#pragma vector=PORT6_VECTOR
__interrupt void switch_left(void)
{
    __disable_interrupt();

    // switch 1 (LOG: next, WATER: start, GOINT: stop)
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
            P3IE &= ~BIT7;
            P3IE &= ~BIT6;
            P5IE &= ~BIT7;
            P3IFG &= ~BIT7;
            P3IFG &= ~BIT6;
            P5IFG &= ~BIT7;

            RTCCTL0_L &= ~RTCTEVIE;

            // change mode
            mode = MOD_GOING;
            mode_water = WATER_WAIT;

            clear_display();
            make_text_water();
            show(text_water1);
            nextline();
            show(text_water3);

            timer0_enable();
        }
        else if (mode == MOD_GOING)
        {
            timer0_disable();
            RTCCTL0_L |= (RTCTEVIE & ~RTCTEVIFG);

            // change mode
            mode = MOD_WATER;
            minute_water = 0;
            second_water = 0;

            // display
            clear_display();
            make_text_main();
            show(text_main1);
            nextline();
            show(text_main2);

            P3IE |= BIT7;
            P3IE |= BIT6;
            P5IE |= BIT7;
            P3IFG &= ~BIT7;
            P3IFG &= ~BIT6;
            P5IFG &= ~BIT7;
        }
    }
    // switch 2 (LOG: page change, WATER: BACKLIGHT)
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
        else if (mode == MOD_WATER || mode == MOD_GOING)
            P4OUT ^= BIT4;
    }

    __no_operation();
    __enable_interrupt();
}

#pragma vector=PORT3_VECTOR
__interrupt void switch_right(void)
{
    __disable_interrupt();

    // switch 3 (change mode)
    if (P3IFG & BIT7)
    {
        P3IFG &= ~BIT7;

        if (mode == MOD_LOG)
        {
            P6IFG &= ~BIT1;
            P6IFG &= ~BIT0;
            P3IFG &= ~BIT6;
            P5IFG &= ~BIT7;

            RTCCTL0_L &= ~RTCTEVIE;

            mode = MOD_WATER;
            log_num = 0;

            clear_display();
            make_text_main();
            show(text_main1);
            nextline();
            show(text_main2);
        }
        else if (mode == MOD_WATER)
        {
            P6IFG &= ~BIT1;
            P6IFG &= ~BIT0;
            P3IFG &= ~BIT6;
            P5IFG &= ~BIT7;

            RTCCTL0_L |= (RTCTEVIE & ~RTCTEVIFG);
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
        else if (mode == MOD_GOING)
        {
            P6IFG &= ~BIT1;
            P6IFG &= ~BIT0;
            P3IFG &= ~BIT6;
            P5IFG &= ~BIT7;
        }
    }
    // switch 4 (LOG: delete log)
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

    __no_operation();
    __enable_interrupt();
}

// power on / off
#pragma vector=PORT5_VECTOR
__interrupt void switch_power(void)
{
    __disable_interrupt();

    if (P5IFG & BIT7)
    {
        P5IFG &= ~BIT7;

        if (*power == 0)
        {
            // low power disable
            PMMCTL0 = PMMPW & ~PMMREGOFF;
            PM5CTL0 &= ~LOCKLPM5;
            __bic_SR_register_on_exit(LPM3_bits + GIE);
        }
        else if (*power > 0)
        {
            *power = 0;
            mode = MOD_OFF;

            // 4 times
            P4OUT |= BIT7;
            __delay_cycles(100000);
            P4OUT &= ~BIT7;
            __delay_cycles(100000);
            P4OUT |= BIT7;
            __delay_cycles(100000);
            P4OUT &= ~BIT7;
            __delay_cycles(100000);
            P4OUT |= BIT7;
            __delay_cycles(100000);
            P4OUT &= ~BIT7;
            __delay_cycles(100000);
            P4OUT |= BIT7;
            __delay_cycles(100000);
            P4OUT &= ~BIT7;

            // switch interrupt disable
            P6IE &= ~BIT1;
            P6IE &= ~BIT0;
            P3IE &= ~BIT7;
            P3IE &= ~BIT6;

            // minute interrupt disable
            RTCCTL0 = (RTCKEY & ~RTCTEVIE);

            // power off LCD, Backlight, Sensor
            P6OUT &= ~BIT2;
            P4OUT &= ~BIT4;
            P4OUT &= ~BIT1;

            // low power enable
            PMMCTL0 = (PMMPW | PMMREGOFF);
            __bis_SR_register(LPM3_bits + GIE);
        }
    }
    __no_operation();
    __enable_interrupt();
}

#endif /* SWITCH_H_ */
