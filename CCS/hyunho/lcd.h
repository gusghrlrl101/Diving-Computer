#ifndef LCD_H_
#define LCD_H_

#include "msp430fr5994.h"
#include "mem.h"
#include "timer.h"
#include <stdlib.h>

inline void data_high(void);
inline void data_low(void);
inline void clk_high(void);
inline void clk_low(void);
inline void reset_high(void);
inline void reset_low(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_out(unsigned char j);
void show(unsigned char *text);
void nextline(void);
void lcd_init(void);
void clear_display(void);

void make_text_log1(unsigned char num);
unsigned char* itoc4(unsigned int num);

const char Slave = 0x7C;
const char Comsend = 0x00;
const char Datasend = 0x40;
const char Line2 = 0xC0;

#define I2C_SDA BIT0   // Serial Data line
#define I2C_SCL BIT1   // Serial Clock line
#define RESET   BIT3   // Serial Clock line

unsigned char text1[] = { "1234567890123456" };
unsigned char text2[] = { "!@#$%^&*()!@#$%^" };
unsigned char text3[] = { "abcdefghijklmnop" };
unsigned char text4[] = { "ABCDEFGHIJKLMNOP" };

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
void I2C_Start(void)
{
    clk_high();
    data_high();
    data_low();
    clk_low();
}

/* I2C communication stops with both the clock and data
 * lines going high, in that order. */
void I2C_Stop(void)
{
    data_low();
    clk_low();
    clk_high();
    data_high();
}

/* Outputs 8-bit command or data via I2C lines. */
void I2C_out(unsigned char j)
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
void show(unsigned char *text)
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
void nextline(void)
{
    I2C_Start();
    I2C_out(Slave);
    I2C_out(Comsend);
    I2C_out(Line2);
    I2C_Stop();
}

/* Initializes the LCD panel. */
void lcd_init(void)
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
void clear_display(void)
{
    I2C_Start();

    I2C_out(0x78); // Slave address of panel.
    I2C_out(0x00); // Control byte: all following bytes are commands.
    I2C_out(0x01); // Clear display.

    I2C_Stop();
}

void make_text_log1(unsigned char num)
{
    Divelog* temp = log_addr + num;
    // line 1
    text1[0] = '[';
    text1[1] = num / 10 + '0';
    text1[2] = num % 10 + '0';
    text1[3] = ']';
    text1[4] = ' ';
    text1[5] = ' ';

    unsigned char* year = itoc4(temp->year);
    text1[6] = year[0];
    text1[7] = year[1];
    text1[8] = year[2];
    text1[9] = year[3];
    text1[10] = '/';

    unsigned char* date = itoc4(temp->date);
    text1[11] = date[0];
    text1[12] = date[1];
    text1[13] = '/';
    text1[14] = date[2];
    text1[15] = date[3];
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
