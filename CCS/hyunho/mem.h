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

#define MAX_LOG 30

// memory map
Divelog* log_addr = (Divelog*) 0x1800;  // 1800 ~ 19DF

unsigned int* year_addr = (unsigned int*) 0x19F8;   // assume current yead
unsigned int* date_addr = (unsigned int*) 0x19FA;   // assume current date
unsigned int* time_addr = (unsigned int*) 0x19FC;   // assume current time
unsigned int* log_size_addr = (unsigned int*) 0x19FE;

// funtion prototype
void insert_log(unsigned int startTime, unsigned int tmp_avg,
                unsigned int tmp_min, unsigned int depth_avg,
                unsigned int depth_max);
void delete_log(unsigned char num);
inline unsigned int convert_time(unsigned int time);

// insert log
void insert_log(unsigned int startTime, unsigned int tmp_avg,
                unsigned int tmp_min, unsigned int depth_avg,
                unsigned int depth_max)
{
    // calculate diveTime
    unsigned int finishTime = convert_time(*time_addr);
    unsigned char diveTime = finishTime - convert_time(startTime);

    // if full, delete first log
    if (*log_size_addr == MAX_LOG)
        delete_log((unsigned char) 0);

    Divelog* temp = log_addr + *log_size_addr;

    // insert datas
    temp->diveTime = diveTime;
    temp->year = *year_addr;
    temp->date = *date_addr;
    temp->startTime = startTime;
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
