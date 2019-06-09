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
    if (++tick_1ms >= 1000)
    {
        if (mode == MOD_GOING)
        {
            TA0CCTL0 &= ~CCIE;
            __enable_interrupt();

//            sensor_enable();
            // P5.0 -> SDA (UCB1CTL1)
            P5SEL1 &= ~BIT0;
            P5SEL0 |= BIT0;

            // P5.1 -> SCL
            P5SEL1 &= ~BIT1;
            P5SEL0 |= BIT1;

            UCB1CTLW0 = (UCSWRST | UCSSEL_2 | UCMST | UCMODE_3 | UCSYNC);
            UCB1BRW = 11;   // bit clock prescaler
            UCB1I2CSA = 0x77;   // slave address
            UCB1CTLW0 &= ~UCSWRST;
            UCB1CTLW1 |= UCASTP_2;
            UCB1IE |= (UCTXIE0 | UCRXIE0);

            send_cmd(RRESET);
            __delay_cycles(4000);
            read_prom();
            __delay_cycles(3000);

            send_cmd(CONV_T);
            __delay_cycles(3000);
            send_cmd(READ);
            __delay_cycles(100);
            recv_data3();
            __delay_cycles(3000);
            result_temp = ((unsigned long) RxBuffer[0] << 16)
                    + ((unsigned long) RxBuffer[1] << 8) + RxBuffer[2];

            send_cmd(CONV_P);
            __delay_cycles(3000);
            send_cmd(READ);
            __delay_cycles(100);
            recv_data3();
            __delay_cycles(3000);
            result_pres = ((unsigned long) RxBuffer[0] << 16)
                    + ((unsigned long) RxBuffer[1] << 8) + RxBuffer[2];

            calc_data();

            if (tmp_sensor != 0 && tmp_min > tmp_sensor)
                tmp_min = tmp_sensor;
            if (depth_max < depth_sensor)
                depth_max = depth_sensor;

            if (diving_sec == 0)
            {
                tmp_avg = tmp_sensor;
                depth_avg = depth_sensor;
            }
            else
            {
                long long temp_tmp_avg = tmp_avg * diving_sec + tmp_sensor;
                long long temp_depth_avg = depth_avg * diving_sec
                        + depth_sensor;
                tmp_avg = temp_tmp_avg / (diving_sec + 1);
                depth_avg = temp_depth_avg / (diving_sec + 1);
            }

            diving_sec++;
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

            timer0_enable();
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
