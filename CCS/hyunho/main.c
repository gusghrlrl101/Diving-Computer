#include "msp430fr5994.h"
#include "lcd.h"
#include "timer.h"
#include "switch.h"

unsigned char text1[] = { "0513 14:30 25.5m" };
unsigned char text2[] = { "D:15.5m  T:17.3c" };
unsigned char text3[] = { "test test test  " };

typedef struct Divelog  // 16 Bytes
{
    unsigned char num;
    unsigned int year;
    unsigned int date;
    unsigned int startTime;
    unsigned char diveTime;
    int tmp_avg;
    int tmp_max;
    unsigned int depth_avg;
    unsigned int depth_max;
} Divelog;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

//    MPUCTL0 = MPUPW;
//    MPUSEGB1 = 0x4000;
//    MPUSEGB2 = 0x4200;
//    MPUCTL0 = MPUPW | MPUENA | MPUSEGIE;

    __enable_interrupt();

    // LED OUTPUT
    P1DIR |= BIT2;
    P1DIR |= BIT3;

    switch_init();
    timer0_init();
    lcd_init();

    show(text1);
    nextline();
    show(text2);

    delay(1000);

    Divelog* temp = (Divelog*) 0x41000;
    unsigned long temp2 = (unsigned long) 0x41000;
    __data20_write_char(temp2, (unsigned char) 0x11);
//    temp->num = (unsigned char) 0x11;
    temp->year = 0x2222;
    temp->date = 0x3333;
    temp->startTime = 0x4444;
    temp->diveTime = (unsigned char) 0x55;
    temp->tmp_avg = 0x6666;
    temp->tmp_max = 0x7777;
    temp->depth_avg = 0x8888;
    temp->depth_max = 0x9999;
    (temp + 1)->num = 0xDD;


    while (1)
    {
        P1DIR ^= BIT2;
    }
}
