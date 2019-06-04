#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"
#include "timer.h"
#include "switch.h"


void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

    __enable_interrupt();

    // LED OUTPUT
    P1DIR |= BIT2;
    P1DIR |= BIT3;
    // BUZZER OUTPUT
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;

    switch_init();
    timer0_init();
    lcd_init();

    show(text_water1);
    nextline();
    show(text_water2);

    delay(1000);

    /*
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

    delete_log((unsigned char) 0);
    */

    while (1);
}
