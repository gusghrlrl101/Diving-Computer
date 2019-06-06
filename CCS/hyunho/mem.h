#ifndef MEM_H_
#define MEM_H_

#include "msp430fr5994.h"

// Divelog struct (16 Bytes)
typedef struct Divelog
{
    unsigned char diveTime;
    unsigned char num;
    unsigned int year;
    unsigned int date;
    unsigned int startTime;
    int tmp_avg;
    int tmp_min;
    unsigned int depth_avg;
    unsigned int depth_max;
} Divelog;

// global variable
#define MOD_OFF 0
#define MOD_WATER 1
#define MOD_GOING 2
#define MOD_LOG 3
#define MAX_LOG 30
volatile unsigned int mode = MOD_WATER;
volatile unsigned int log_page = 0;
volatile unsigned char log_num = 0;

// time from start in water
volatile unsigned char minute_water = 0;
volatile unsigned char second_water = 0;

// assume value from sensor in water
volatile unsigned int tmp_sensor = 0;
volatile unsigned int depth_sensor = 0;

// memory map
Divelog* log_addr = (Divelog*) 0x1800;  // 1800 ~ 19DF

unsigned int* water_startTime = (unsigned int*) 0x19FA;
unsigned int* power = (unsigned int*) 0x19FC;
unsigned int* log_size_addr = (unsigned int*) 0x19FE;

// funtion prototype
inline void delay(int time);
inline void power_init();
inline void set_time();
void insert_log(unsigned int tmp_avg, unsigned int tmp_min,
                unsigned int depth_avg, unsigned int depth_max);
void delete_log(unsigned char num);
inline unsigned int convert_time(unsigned int time);

inline void delay(int time)
{
    int i = time;
    while (i-- > 0)
        __delay_cycles(1000);
}

inline void power_init()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PMMCTL0_H = PMMPW_H;
    PM5CTL0 &= ~LOCKLPM5;

    *power = 1;

    RTCCTL0 = (RTCKEY | RTCTEVIE & ~RTCTEVIFG);
    RTCCTL13 &= ~RTCHOLD;
    //    set_time();


    __enable_interrupt();

    // BUZZER OUTPUT
    P4OUT &= ~BIT7;
    P4DIR |= BIT7;

    // BACKLIGHT OUTPUT
    P4OUT &= ~BIT4;
    P4DIR |= BIT4;

    // LCD OUTPUT
    P6OUT |= BIT2;
    P6DIR |= BIT2;
}


inline void set_time(){
    RTCCTL13 |= RTCHOLD;

    RTCYEAR = 0x2019;
    RTCMON = 0x06;
    RTCDAY = 0x06;
    RTCDOW = 0x04;

    RTCHOUR = 0x18;
    RTCMIN = 0x05;
    RTCSEC = 0x50;

    RTCCTL13 &= ~RTCHOLD;
}


// insert log
void insert_log(unsigned int tmp_avg, unsigned int tmp_min,
                unsigned int depth_avg, unsigned int depth_max)
{
    unsigned char diveTime = minute_water * 60 + second_water;

    // if full, delete first log
    if (*log_size_addr == MAX_LOG)
        delete_log((unsigned char) 0);

    Divelog* temp = log_addr + *log_size_addr;

    // insert datas
    temp->diveTime = diveTime;
    temp->year = RTCYEAR;
    temp->date = RTCDATE;
    temp->startTime = water_startTime;
    temp->tmp_avg = tmp_avg;
    temp->tmp_min = tmp_min;
    temp->depth_avg = depth_avg;
    temp->depth_max = depth_max;

    // increase size
    (*log_size_addr)++;
}

// delete log : num = log_num - 1
void delete_log(unsigned char num)
{
    // exception
    if (MAX_LOG <= num)
        return;

    unsigned int log_size = *log_size_addr;
    Divelog* temp = log_addr;
    int i;

    // move datas
    for (i = num; i < log_size - 1; i++)
    {
        (temp + i)->diveTime = (temp + i + 1)->diveTime;
        (temp + i)->year = (temp + i + 1)->year;
        (temp + i)->date = (temp + i + 1)->date;
        (temp + i)->startTime = (temp + i + 1)->startTime;
        (temp + i)->tmp_avg = (temp + i + 1)->tmp_avg;
        (temp + i)->tmp_min = (temp + i + 1)->tmp_min;
        (temp + i)->depth_avg = (temp + i + 1)->depth_avg;
        (temp + i)->depth_max = (temp + i + 1)->depth_max;
    }

    // decrease size
    *log_size_addr = log_size - 1;
}

// convert hh:mm -> mmm
inline unsigned int convert_time(unsigned int time)
{
    unsigned h = time / 100;
    unsigned m = time % 100;
    return h * 60 + m;
}

#endif /* MEM_H_ */
