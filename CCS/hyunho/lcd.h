#ifndef LCD_H_
#define LCD_H_

#include "msp430fr5994.h"
#include "mem.h"
#include <stdlib.h>

inline void data_high(void);
inline void data_low(void);
inline void clk_high(void);
inline void clk_low(void);
inline void reset_high(void);
inline void reset_low(void);
inline void I2C_Start(void);
inline void I2C_Stop(void);
inline void I2C_out(unsigned char j);
inline void show(unsigned char *text);
inline void nextline(void);
inline void lcd_init(void);
inline void clear_display(void);

inline void make_text_water();
inline void make_text_water1();
inline void make_text_water2();

inline void make_text_log();
inline void make_text_log1(unsigned char num);
inline void make_text_log2(unsigned char num);
inline void make_text_log3(unsigned char num);
inline void make_text_log4(unsigned char num);

unsigned char* itoc3(unsigned int num);
unsigned char* itoc4(unsigned int num);

const char Slave = 0x7C;
const char Comsend = 0x00;
const char Datasend = 0x40;
const char Line2 = 0xC0;

#define I2C_SDA BIT0   // Serial Data line
#define I2C_SCL BIT1   // Serial Clock line
#define RESET   BIT3   // Serial Clock line

unsigned char text_water1[] = { "hh:mm time]mm:ss" };
unsigned char text_water2[] = { "D]DP.Tm  T]tm.pC" };
unsigned char text_log1[] = { "[NO]  YYYY/MM/DD" };
unsigned char text_log2[] = { "hh:mm  DTTminute" };
unsigned char text_log3[] = { "A]tm.pC  m]tm.pC" };
unsigned char text_log4[] = { "A]DP.Tm  M]DP.Tm" };
unsigned char text_log5[] = { "ERROR: NO LOG !!" };

inline void data_high(void)
{
    P7OUT |= I2C_SDA;
}

inline void data_low(void)
{
    P7OUT &= ~I2C_SDA;
}

inline void clk_high(void)
{
    P7OUT |= I2C_SCL;
}

inline void clk_low(void)
{
    P7OUT &= ~I2C_SCL;
}

inline void reset_high(void)
{
    P6OUT |= RESET;
}

inline void reset_low(void)
{
    P6OUT &= RESET;
}

/* I2C communication starts when both the data and clock
 * lines go low, in that order. */
inline void I2C_Start(void)
{
    clk_high();
    data_high();
    data_low();
    clk_low();
}

/* I2C communication stops with both the clock and data
 * lines going high, in that order. */
inline void I2C_Stop(void)
{
    data_low();
    clk_low();
    clk_high();
    data_high();
}

/* Outputs 8-bit command or data via I2C lines. */
inline void I2C_out(unsigned char j)
{
    unsigned char d = j;
    int n = 8;
    while (n-- > 0)
    {
        if ((d & 0x80) == 0x80)
            data_high();
        else
            data_low();
        d <<= 1;
        clk_low();
        clk_high();
        clk_low();
    }
    clk_high();
    clk_low();
}

/* Writes a 20-char string to the RAM of the LCD. */
inline void show(unsigned char *text)
{
    int n;

    I2C_Start();
    I2C_out(Slave); // Slave address of panel.
    I2C_out(Datasend); // Control byte: data bytes follow, data is RAM data.

    for (n = 0; n < 16; n++)
    {
        I2C_out(*text);
        ++text;
    }

    I2C_Stop();
}

/* Move to Line 2 */
inline void nextline(void)
{
    I2C_Start();
    I2C_out(Slave);
    I2C_out(Comsend);
    I2C_out(Line2);
    I2C_Stop();
}

/* Initializes the LCD panel. */
inline void lcd_init(void)
{
    // LCD OUTPUT
    P7DIR |= I2C_SDA;
    P7DIR |= I2C_SCL;
    P6DIR |= RESET;

    // reset
    reset_low();
    delay(20);
    reset_high();

    // init function
    delay(50);
    I2C_Start();
    I2C_out(Slave);
    I2C_out(Comsend);
    delay(10);
    I2C_out(0x38); // 8-bit bus, 2-line display, normal instruction mode.
    __delay_cycles(27);
    I2C_out(0x39); // 8-bit bus, 2-line display, extension instruction mode.///////////////
    __delay_cycles(27);
    I2C_out(0x14); // Bias set to 1/5.
    __delay_cycles(27);
    I2C_out(0x78); // Contrast set.
    __delay_cycles(27);
    I2C_out(0x5C); // Icon display on, booster on, contrast set. //////////
    __delay_cycles(27);
    I2C_out(0x6F); // Follower circuit on, amplifier=
    delay(300);
    I2C_out(0x0C); // Display on, cursor off.
    __delay_cycles(27);
    I2C_out(0x01); // Clear display. ////////////
    __delay_cycles(27);
    I2C_Stop();
    __delay_cycles(27);
}

/* Sends the "clear display" command to the LCD. */
inline void clear_display(void)
{
    I2C_Start();
    I2C_out(Slave);
    I2C_out(Comsend); // Control byte: all following bytes are commands.
    delay(10);
    I2C_out(0x01); // Clear display.
    __delay_cycles(27);
    I2C_Stop();
}

inline void make_text_water()
{
    make_text_water1();
    make_text_water2();
}

inline void make_text_water1()
{
    unsigned char hour = (RTCTIM1 & 0xFF);
    text_water1[0] = hour / 10 + '0';
    text_water1[1] = hour % 10 + '0';

    unsigned char minute = (RTCTIM0 >> 8);
    text_water1[3] = minute / 10 + '0';
    text_water1[4] = minute % 10 + '0';

    // minute
    text_water1[11] = minute_water / 10 + '0';
    text_water1[12] = minute_water % 10 + '0';

    // second
    text_water1[14] = second_water / 10 + '0';
    text_water1[15] = second_water % 10 + '0';
}

inline void make_text_water2()
{
    unsigned char* tmp_text = itoc3(tmp_sensor);
    text_water2[2] = tmp_text[0];
    text_water2[3] = tmp_text[1];
    text_water2[5] = tmp_text[2];

    unsigned char* depth_text = itoc3(depth_sensor);
    text_water2[11] = depth_text[0];
    text_water2[12] = depth_text[1];
    text_water2[14] = depth_text[2];
}

inline void make_text_log()
{
    make_text_log1(log_num);
    make_text_log2(log_num);
    make_text_log3(log_num);
    make_text_log4(log_num);
}

inline void make_text_log1(unsigned char num)
{
    Divelog* temp = log_addr + num;

    // log num
    text_log1[1] = (num + 1) / 10 + '0';
    text_log1[2] = (num + 1) % 10 + '0';

    unsigned char* year = itoc4(temp->year);
    text_log1[6] = year[0];
    text_log1[7] = year[1];
    text_log1[8] = year[2];
    text_log1[9] = year[3];

    unsigned char* date = itoc4(temp->date);
    text_log1[11] = date[0];
    text_log1[12] = date[1];
    text_log1[14] = date[2];
    text_log1[15] = date[3];
}

inline void make_text_log2(unsigned char num)
{
    Divelog* temp = log_addr + num;

    unsigned char* startTime = itoc4(temp->startTime);
    text_log2[0] = startTime[0];
    text_log2[1] = startTime[1];
    text_log2[3] = startTime[2];
    text_log2[4] = startTime[3];

    unsigned char* diveTime = itoc3((unsigned int) temp->diveTime);
    text_log2[7] = diveTime[0];
    text_log2[8] = diveTime[1];
    text_log2[9] = diveTime[2];
}

inline void make_text_log3(unsigned char num)
{
    Divelog* temp = log_addr + num;

    unsigned char* tmp_avg = itoc3(temp->tmp_avg);
    text_log3[2] = tmp_avg[0];
    text_log3[3] = tmp_avg[1];
    text_log3[5] = tmp_avg[2];

    unsigned char* tmp_min = itoc3(temp->tmp_min);
    text_log3[11] = tmp_min[0];
    text_log3[12] = tmp_min[1];
    text_log3[14] = tmp_min[2];
}

inline void make_text_log4(unsigned char num)
{
    Divelog* temp = log_addr + num;

    unsigned char* depth_avg = itoc3(temp->depth_avg);
    text_log4[2] = depth_avg[0];
    text_log4[3] = depth_avg[1];
    text_log4[5] = depth_avg[2];

    unsigned char* depth_max = itoc3(temp->depth_max);
    text_log4[11] = depth_max[0];
    text_log4[12] = depth_max[1];
    text_log4[14] = depth_max[2];
}

unsigned char* itoc3(unsigned int num)
{
    unsigned char c[] = "123";
    c[0] = (num / 100) + '0';
    if (c[0] == '0')
        c[0] = ' ';
    num %= 100;
    c[1] = (num / 10) + '0';
    if (c[1] == '0')
        c[1] = ' ';
    c[2] = (num % 10) + '0';

    return c;
}

unsigned char* itoc4(unsigned int num)
{
    unsigned char c[] = "1234";
    c[0] = (num / 1000) + '0';
    num %= 1000;
    c[1] = (num / 100) + '0';
    num %= 100;
    c[2] = (num / 10) + '0';
    c[3] = (num % 10) + '0';

    return c;
}


#endif /* LCD_H_ */
