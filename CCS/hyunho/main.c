#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"
#include "timer.h"
#include "switch.h"

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

    RTCCTL0 = (RTCKEY | RTCTEVIE);
    RTCIV ^= RT0PSIFG;
    RTCCTL13 &= ~RTCHOLD;

    __enable_interrupt();

    // BUZZER OUTPUT
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;

    switch_init();
    timer0_init();
    lcd_init();

    show(text_water1);
    nextline();
    show(text_water2);
    delay(100);

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

    while (1)
    {

    }
}
