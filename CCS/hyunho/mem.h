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

#define WATER_WAIT 0
#define WATER_START 1
#define WATER_GOING 2
#define WATER_FINISH 3
volatile unsigned char mode_water = WATER_WAIT;

volatile unsigned int log_page = 0;
volatile unsigned char log_num = 0;

// time from start in water
volatile unsigned char minute_water = 0;
volatile unsigned char second_water = 0;
volatile unsigned long diving_sec = 0;

// value from sensor in water
volatile unsigned int tmp_sensor = 0;
volatile unsigned int depth_sensor = 0;
volatile unsigned int depth_before = 0;
volatile unsigned char alarm = 0;
volatile unsigned int tmp_avg = 0;
volatile unsigned int depth_avg = 0;
volatile float tmp_avg_f = 0.0f;
volatile float depth_avg_f = 0.0f;
volatile unsigned int tmp_min = 999;
volatile unsigned int depth_max = 0;

// memory map
Divelog* log_addr = (Divelog*) 0x1800;  // 1800 ~ 19DF

unsigned int* firstPower = (unsigned int*) 0x19F0;
unsigned int* water_startTime = (unsigned int*) 0x19FA;
unsigned int* power = (unsigned int*) 0x19FC;
unsigned int* log_size_addr = (unsigned int*) 0x19FE;

// funtion prototype
inline void power_init();
inline void set_time();
void insert_log();
void delete_log(unsigned char num);

inline void power_init()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PMMCTL0_H = PMMPW_H;
    PM5CTL0 &= ~LOCKLPM5;

    *power = 1;

    RTCCTL0 = (RTCKEY | RTCTEVIE & ~RTCTEVIFG);
    RTCCTL13 &= ~RTCHOLD;

    *firstPower = 1;
    if (*firstPower == 1)
    {
        set_time();
        *firstPower = 0;
    }
    __enable_interrupt();

    // BUZZER OUTPUT
    P4OUT &= ~BIT7;
    P4DIR |= BIT7;

    // 3 times
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

    // BACKLIGHT OUTPUT
    P4OUT &= ~BIT4;
    P4DIR |= BIT4;

    // LCD POWER OUTPUT
    P6OUT |= BIT2;
    P6DIR |= BIT2;

    // SENSOR POWER OUTPUT
    P4OUT |= BIT1;
    P4DIR |= BIT1;
}

inline void set_time()
{
    RTCCTL13 |= RTCHOLD;

    RTCYEAR = 2019;
    RTCMON = 6;
    RTCDAY = 10;
    RTCDOW = 0;

    RTCHOUR = 14;
    RTCMIN = 31;
    RTCSEC = 0;

    RTCCTL13 &= ~RTCHOLD;
}

// insert log
void insert_log()
{
    unsigned char diveTime = diving_sec / 60;

    // if full, delete first log
    if (*log_size_addr == MAX_LOG)
        delete_log((unsigned char) 0);

    Divelog* temp = log_addr + *log_size_addr;

    if (diving_sec % 60 != 0)
        diveTime++;

    // insert datas
    temp->diveTime = diveTime;
    temp->year = RTCYEAR;
    temp->date = (RTCMON / 10) * 1000 + (RTCMON % 10) * 100 + (RTCDAY / 10) * 10
            + (RTCDAY % 10);
    temp->startTime = *water_startTime;
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

#endif /* MEM_H_ */
