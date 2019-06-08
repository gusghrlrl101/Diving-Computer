#ifndef TIMER_H_
#define TIMER_H_

#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"
#include "sensor.h"

inline void timer0_enable(void);
inline void timer0_disable(void);

volatile unsigned int tick_1ms = 0;
volatile unsigned int tick2_1ms = 0;
volatile unsigned int released = 0;

inline void timer0_enable(void)
{
    TA0CCR0 = 1000;
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
    __enable_interrupt();

    if (++tick_1ms >= 1000)
    {
        if (mode == MOD_GOING)
        {
            //// sensor_enable();
            __no_operation();
            __enable_interrupt();

            //// read_prom();
            unsigned char j;
            for (j = 0; j <= 7; j++)
            {
                //// send_cmd(0xA0 + (j * 2));
                TXData = 0xA0 + (j * 2);
                TXByteCtr = 1;                          // Load TX byte counter
                while (UCB1CTLW0 & UCTXSTP)
                {

                }
                UCB1CTLW0 |= UCTR + UCTXSTT;             // I2C TX, start condition
                __no_operation();
                __enable_interrupt();
                ////

                __delay_cycles(1000);

                //// recv_data2();
                UCB1TBCNT = 2;  // number of bytes to be received
                PRxData = (unsigned char *) RxBuffer;    // Start of RX buffer
                RXByteCtr = 1;                          // Clear RX byte count
                while (UCB1CTLW0 & UCTXSTP)
                {

                }
                UCB1CTLW0 &= ~UCTR;
                UCB1CTLW0 |= UCTXSTT;
                ////

                __delay_cycles(3000);
                coefficient[j] = (RxBuffer[0] << 8) | RxBuffer[1];
            }
            ////

            __delay_cycles(3000);

            send_cmd(CONV_T);
            __delay_cycles(3000);
            send_cmd(READ);
            __delay_cycles(1000);
            recv_data3();
            __delay_cycles(3000);
            result_temp = ((unsigned long) RxBuffer[0] << 16)
                    + ((unsigned long) RxBuffer[1] << 8) + RxBuffer[2];

            send_cmd(CONV_P);
            __delay_cycles(3000);
            send_cmd(READ);
            __delay_cycles(1000);
            recv_data3();
            __delay_cycles(3000);
            result_pres = ((unsigned long) RxBuffer[0] << 16)
                    + ((unsigned long) RxBuffer[1] << 8) + RxBuffer[2];

            calc_data();
            ////

            if (++second_water == 60)
            {
                second_water = 0;
                minute_water++;
            }

            // response

            clear_display();
            make_text_water();
            show(text_water1);
            nextline();
            show(text_water2);
        }
    }
    TA0CCR0 += 1000;
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
            show(text_water3);
        }
    }
}

#endif /* TIMER_H_ */
