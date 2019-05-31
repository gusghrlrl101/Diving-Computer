#include "msp430fr5994.h"

#define I2C_SDA BIT0   // Serial Data line
#define I2C_SCL BIT1   // Serial Clock line
#define RESET   BIT3   // Serial Clock line

void timer0_init(void);
void switch_init(void);
void delay(int time);
void flash_mem_erase(unsigned int * addr);
void flash_mem_write_int(unsigned int * addr, unsigned int data);

struct dive_log
{
    int log_no;
    int dive_date_year;
    int dive_date_day;
    int dive_start_time;
    int dive_time;
    int tmp_avrg;
    int tmp_max;
    int dpth_avrg;
    int dpth_max;
};

volatile unsigned int tick_1ms = 0;
volatile unsigned int delay_1ms = 0;

unsigned int * mem_start_addr = 0x1800; // start information memory address bit of MSP430F5529
unsigned int * mem_end_addr = 0x19ff; // end information memory address bit of MSP430F5529
unsigned int * mem_target_addr = 0x1800;
unsigned int * mem_read_addr = 0x1840;
unsigned int * log_pointer_addr = 0x1900;
unsigned int log_pointer = 0x1820;

const char Slave = 0x7C;
const char Comsend = 0x00;
const char Datasend = 0x40;
const char Line2 = 0xC0;

unsigned char text1[] = { "0513 14:30 25.5m" };
unsigned char text2[] = { "D:15.5m  T:17.3c" };
unsigned char text3[] = { "test test test  " };

void data_high(void)
{
//    P7DIR |= I2C_SDA; // float pin to go high
    P7OUT |= I2C_SDA;
//    P7DIR &= ~I2C_SDA;
//    delay(10);
}

void data_low(void)
{
    P7OUT &= ~I2C_SDA; // assert low
//    P7DIR |= I2C_SDA;
//    delay(10);
}

void clk_high(void)
{
//    P7DIR |= I2C_SCL;  // float pin to go high
    P7OUT |= I2C_SCL;
//    P7DIR &= ~I2C_SCL;
//    delay(10);
}

void clk_low(void)
{
    P7OUT &= ~I2C_SCL;  // assert low
//    P7DIR |= I2C_SCL;
 //   delay(10);
}

void reset_high(void)
{
//    P6DIR |= RESET; // float pin to go high
    P6OUT |= RESET;
//    P6DIR &= ~RESET;
//    delay(10);
}

void reset_low(void)
{
//    P6OUT &= ~RESET; // assert low
//    P6DIR |= RESET;
    P6OUT &= RESET;
//    delay(10);
}

/* Outputs 8-bit command or data via I2C lines. */
void I2C_out(unsigned char j)
{
    int n;
    unsigned char d;
    d = j;
    for (n = 0; n < 8; n++)
    {
        if ((d & 0x80) == 0x80)
            data_high();
        else
            data_low();
        d = (d << 1);
        clk_low();
        clk_high();
        clk_low();
    }
    clk_high();

//    while(P7OUT & I2C_SDA){
//        clk_low();
//        clk_high();
//    }
    clk_low();
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

void nextline(void)  // Move to Line 2
{
    I2C_Start();
    I2C_out(Slave);
    I2C_out(Comsend);
    I2C_out(Line2);
    I2C_Stop();
}

/* Initializes the LCD panel. */
void init_LCD(void)
{
//    P7SEL1 &= ~I2C_SDA;
//    P7SEL0 |= I2C_SDA;

//    P7SEL1 &= ~I2C_SCL;
//    P7SEL0 |= I2C_SCL;

    delay(800);
    I2C_Start();
    delay(100);
    I2C_out(Slave);
    delay(100);
    I2C_out(Comsend);
    delay(100);
    I2C_out(0x38); // 8-bit bus, 2-line display, normal instruction mode.
    delay(100);
    I2C_out(0x39); // 8-bit bus, 2-line display, extension instruction mode.///////////////
    delay(100);
    I2C_out(0x14); // Bias set to 1/5.
    delay(100);
    I2C_out(0x78); // Contrast set.
    delay(100);
    I2C_out(0x5C); // Icon display on, booster on, contrast set. //////////
    delay(100);
    I2C_out(0x6F); // Follower circuit on, amplifier=
    delay(400);
    I2C_out(0x0C); // Display on, cursor off.
    delay(100);
    I2C_out(0x01); // Clear display. ////////////
    delay(100);
    I2C_Stop();
    delay(100);
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

/*
void make_sample_data()
{
    struct dive_log sample_log[4];

    sample_log[0].log_no = 1;
    sample_log[0].dive_date_year = 2019;
    sample_log[0].dive_date_day = 153;
    sample_log[0].dive_start_time = 740;
    sample_log[0].dive_time = 1874;
    sample_log[0].tmp_avrg = 111;
    sample_log[0].tmp_max = 125;
    sample_log[0].dpth_avrg = 123;
    sample_log[0].dpth_max = 241;

    sample_log[1].log_no = 2;
    sample_log[1].dive_date_year = 2019;
    sample_log[1].dive_date_day = 154;
    sample_log[1].dive_start_time = 740;
    sample_log[1].dive_time = 1844;
    sample_log[1].tmp_avrg = 122;
    sample_log[1].tmp_max = 115;
    sample_log[1].dpth_avrg = 124;
    sample_log[1].dpth_max = 242;

    sample_log[2].log_no = 3;
    sample_log[2].dive_date_year = 2019;
    sample_log[2].dive_date_day = 155;
    sample_log[2].dive_start_time = 740;
    sample_log[2].dive_time = 1814;
    sample_log[2].tmp_avrg = 133;
    sample_log[2].tmp_max = 125;
    sample_log[2].dpth_avrg = 125;
    sample_log[2].dpth_max = 243;

    sample_log[3].log_no = 4;
    sample_log[3].dive_date_year = 2019;
    sample_log[3].dive_date_day = 156;
    sample_log[3].dive_start_time = 740;
    sample_log[3].dive_time = 1834;
    sample_log[3].tmp_avrg = 144;
    sample_log[3].tmp_max = 125;
    sample_log[3].dpth_avrg = 126;
    sample_log[3].dpth_max = 244;

    flash_mem_erase((unsigned int *) mem_target_addr);
    flash_mem_erase((unsigned int *) log_pointer_addr);

    flash_mem_write_int((unsigned int *) log_pointer_addr, mem_target_addr);

    if ((*(unsigned int *) log_pointer_addr == 0x0000)
            || (*(unsigned int *) log_pointer_addr == 0xffff))
        flash_mem_write_int((unsigned int *) log_pointer_addr, 0x1820);

    log_pointer = *(unsigned int *) log_pointer_addr;

    //flash_mem_erase((unsigned int *) 0x1890);

    //flash_mem_write_int((unsigned int *) log_pointer_addr, log_pointer);
    //flash_mem_write_int((unsigned int *) log_pointer_addr+1, log_pointer);
    int i = 0;
    for (; i < 4; i++)
    {
        flash_mem_write_int((unsigned int *) mem_target_addr,
                            sample_log[i].log_no);
        flash_mem_write_int((unsigned int *) mem_target_addr + 0x01,
                            sample_log[i].dive_date_year);
        flash_mem_write_int((unsigned int *) mem_target_addr + 0x02,
                            sample_log[i].dive_date_day);
        flash_mem_write_int((unsigned int *) mem_target_addr + 0x03,
                            sample_log[i].dive_start_time);
        flash_mem_write_int((unsigned int *) mem_target_addr + 0x04,
                            sample_log[i].dive_time);
        flash_mem_write_int((unsigned int *) mem_target_addr + 0x05,
                            sample_log[i].tmp_avrg);
        flash_mem_write_int((unsigned int *) mem_target_addr + 0x06,
                            sample_log[i].tmp_max);
        flash_mem_write_int((unsigned int *) mem_target_addr + 0x07,
                            sample_log[i].dpth_avrg);
        flash_mem_write_int((unsigned int *) mem_target_addr + 0x08,
                            sample_log[i].dpth_max);
        mem_target_addr = (unsigned int *) mem_target_addr + 0x09;
    }

}

void flash_mem_erase(unsigned int * addr)
{
    //__disable_interrupt();
    unsigned int * info_flash_area;

    // should work in only information memory area
    if ((addr >= (unsigned int *) mem_start_addr)
            && (addr <= (unsigned int *) mem_end_addr))
    {
        info_flash_area = (unsigned int *) addr;
    }
    else
    {
        return;
    }

    //set flash control register
//    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
//    FCTL3 = FWKEY;                            // Clear Lock bit

    *(unsigned char *) info_flash_area = 0;

//    FCTL1 = FWKEY;
//    FCTL3 = FWKEY;
    //__enable_interrupt();
}

void flash_mem_write_int(unsigned int * addr, unsigned int data)
{
    //__disable_interrupt();
    unsigned int * info_flash_area;

    // should work in only information memory area
    if ((addr >= (unsigned int *) mem_start_addr)
            && (addr <= (unsigned int *) mem_end_addr))
    {
        info_flash_area = (unsigned int *) addr;
    }
    else
    {
        return;
    }

    //set flash control register
//    FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
//    FCTL3 = FWKEY;
    *(unsigned int *) info_flash_area = data;
//    FCTL1 = FWKEY;
//    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit

    //__enable_interrupt();
}

void make_text1(struct dive_log log)
{
    int log_no = log.log_no;
    int month = log.dive_date_day / 30;
    int day = log.dive_date_day % 30;
    int start_time_h = log.dive_start_time / 60;
    int start_time_m = log.dive_start_time % 60;
    int dpth_max = log.dpth_max;

    text1[0] = 'N';
    text1[1] = 'o';
    text1[2] = '.';
    text1[3] = (log_no / 10) + '0';
    text1[4] = (log_no % 10) + '0';
    text1[5] = ' ';
    text1[6] = (month / 10) + '0';
    text1[7] = (month % 10) + '0';
    text1[8] = (day / 10) + '0';
    text1[9] = (day % 10) + '0';
    text1[10] = ' ';
    text1[11] = (start_time_h / 10) + '0';
    text1[12] = (start_time_h % 10) + '0';
    text1[13] = ':';
    text1[14] = (start_time_m / 10) + '0';
    text1[15] = (start_time_m % 10) + '0';
}

void make_text2(struct dive_log log)
{
    int dive_time_m = log.dive_time / 60;
    int dive_time_s = log.dive_time % 60;
    int dpth_max = log.dpth_max;

    text2[0] = 'M';
    text2[1] = 'D';
    text2[2] = ':';
    text2[3] = (dpth_max / 100) + '0';
    text2[4] = (dpth_max % 100) / 10 + '0';
    text2[5] = '.';
    text2[6] = dpth_max % 10 + '0';
    text2[7] = 'm';
    text2[8] = ' ';
    text2[9] = ' ';
    text2[10] = (dive_time_m / 10) + '0';
    text2[11] = (dive_time_m % 10) + '0';
    text2[12] = 'M';
    text2[13] = (dive_time_s / 10) + '0';
    text2[14] = (dive_time_s % 10) + '0';
    text2[15] = 'S';
}

void make_text3(struct dive_log log)
{
    int dpth = log.dpth_avrg;
    int tmp = log.tmp_avrg;

    text3[0] = ' ';
    text3[1] = 'D';
    text3[2] = ':';
    text3[3] = (dpth / 100) + '0';
    text3[4] = (dpth % 100) / 10 + '0';
    text3[5] = '.';
    text3[6] = dpth % 10 + '0';
    text3[7] = 'm';
    text3[8] = ' ';
    text3[9] = 'T';
    text3[10] = ':';
    text3[11] = tmp / 100 + '0';
    text3[12] = (tmp % 100) / 10 + '0';
    text3[13] = '.';
    text3[14] = tmp % 10 + '0';
    text3[15] = 'c';
}

*/

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;
//    DCORSEL = 0;
//    DCOFSEL = 0;

    /*
    struct dive_log display_log;
    make_sample_data();

    display_log.log_no = *mem_start_addr;
    display_log.dive_date_year = *(mem_start_addr + 1);
    display_log.dive_date_day = *(mem_start_addr + 2);
    display_log.dive_start_time = *(mem_start_addr + 3);
    display_log.dive_time = *(mem_start_addr + 4);
    display_log.tmp_avrg = *(mem_start_addr + 5);
    display_log.tmp_max = *(mem_start_addr + 6);
    display_log.dpth_avrg = *(mem_start_addr + 7);
    display_log.dpth_max = *(mem_start_addr + 8);

    make_text1(display_log);
    make_text2(display_log);
    make_text3(display_log);
    */

    // LED OUTPUT
    P1DIR |= BIT2;
    P1DIR |= BIT3;

    // MCLK OUTPUT
    P5DIR |= BIT7;

    // LCD OUTPUT
    P7DIR |= I2C_SDA;
    P7DIR |= I2C_SCL;
    P6DIR |= RESET;

    __bis_SR_register(GIE);
    switch_init();
    timer0_init();

    reset_low();
    delay(20);
    reset_high();

    init_LCD();
    delay(2);
    show(text1);
    nextline();
    show(text2);
    delay(2500);

    //unsigned int * pointer = mem_start_addr;

    while (1){
//        P7DIR |= I2C_SDA;
//        delay(1000);
//        clk_high();
//        clk_low();
        P1DIR ^= BIT2;
    }
}

void timer0_init(void)
{
    TA0CCTL0 = CCIE;
    TA0CCR0 = 999;
    TA0CTL = TASSEL_2 + MC_2;
}

void switch_init(void)
{
    P5DIR &= ~BIT3;
    P5SEL1 &= ~BIT3;
    P5SEL0 &= ~BIT3;

    P5IE |= BIT3;
    P5IES &= ~BIT3;
    P5IFG &= ~BIT3;
}

void delay(int time)
{
    TA1CTL = TASSEL_2 + MC_2;
    TA1CCTL0 = CCIE;
    TA1CCR0 = 999;

    delay_1ms = 0;
    while (delay_1ms < time);
    TA1CTL = MC_0;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void _tick_1sec(void)
{
    if (++tick_1ms >= 1000)
    {
        // tick
        P1OUT ^= BIT3;

        tick_1ms = 0;
    }
    TA0CCR0 += 999;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void _delay(void)
{
    delay_1ms++;
    TA1CCR0 += 999;
}

#pragma vector=PORT5_VECTOR
__interrupt void _switch1(void)
{
    P5IFG &= ~BIT3;
//    P1OUT ^= BIT2;
}
