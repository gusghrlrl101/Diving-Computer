#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"
#include "timer.h"
#include "switch.h"

inline void make_sample_data();
inline void set_time();

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

    RTCCTL0 = (RTCKEY | RTCTEVIE | ~RTCTEVIFG);
    RTCCTL13 &= ~RTCHOLD;

//    set_time();

    __enable_interrupt();

    // BUZZER OUTPUT
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;

    // BACKLIGHT OUTPUT
    P4DIR |= BIT4;
    P4OUT &= ~BIT4;

    switch_init();
    lcd_init();

    make_text_water();
    show(text_water1);
    nextline();
    show(text_water2);
    delay(100);

    // make_sample_data();

    while (1)
    {

    }
}

inline void make_sample_data()
{
    unsigned int i;
    Divelog* temp = log_addr;
    for (i = 0; i < MAX_LOG; i++)
    {
        (temp + i)->diveTime = (unsigned char) 0;
        (temp + i)->year = 0;
        (temp + i)->date = 0;
        (temp + i)->startTime = 0;
        (temp + i)->tmp_avg = 0;
        (temp + i)->tmp_min = 0;
        (temp + i)->depth_avg = 0;
        (temp + i)->depth_max = 0;
    }
    *log_size_addr = 0;

    for (i = 0; i < MAX_LOG; i++)
        insert_log(1000 + i, i, i, i, i);
}


inline void set_time(){
    RTCCTL13 |= RTCHOLD;

    RTCYEAR = 0x2019;
    RTCMON = 0x06;
    RTCDAY = 0x06;
    RTCDOW = 0x04;

    RTCHOUR = 0x02;
    RTCMIN = 0x07;
    RTCSEC = 0x00;

    RTCCTL13 &= ~RTCHOLD;
}
