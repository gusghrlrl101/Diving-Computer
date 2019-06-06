#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"
#include "timer.h"
#include "switch.h"

#define RD_PROM 0xA0
#define RRESET   0x1E
#define CONV_P  0x40
#define CONV_T  0x50
#define READ    0x00

unsigned char RXData;
unsigned char TXData;
unsigned char TXByteCtr;

unsigned char *PRxData;                     // Pointer to RX data
unsigned char RXByteCtr;
volatile unsigned char RxBuffer[128];       // Allocate 128 byte of RAM

long _temperature_actual;
long _pressure_actual;

long temperature_reported;
long pressure_reported;

unsigned int coefficient[8];       // Coefficients;
unsigned long result_temp;
unsigned long result_pres;

double time__;

void read_prom();
void send_cmd(unsigned char data);
void recv_data();
void calc_data();

void read_prom()
{
    unsigned char j;
    for (j = 0; j <= 7; j++)
    {
        send_cmd(0xA0 + (j * 2));
        recv_data();
        coefficient[j] = (RxBuffer[0] << 8) | RxBuffer[1];
    }
}

void send_cmd(unsigned char data)
{
    TXData = data;
    //delay_long();
    TXByteCtr = 1;                          // Load TX byte counter

    while (UCB1CTLW0 & UCTXSTP)
        ;             // Ensure stop condition got sent
    UCB1CTLW0 |= (UCTR | UCTXSTT);             // I2C TX, start condition

    __enable_interrupt();
}

void recv_data()
{
    PRxData = (unsigned char *) RxBuffer;    // Start of RX buffer
    RXByteCtr = 0;                          // Clear RX byte count

    while (UCB1CTLW0 & UCTXSTP)
        ;             // Ensure stop condition got sent
    UCB1CTLW0 |= (~UCTR & UCTXSTT);                    // I2C start condition
    while (UCB1CTLW0 & UCTXSTT)
        ;              // Start condition sent?
    __delay_cycles(400);
    UCB1CTLW0 |= UCTXSTP;                    // I2C stop condition
}

void calc_data()
{
    //Create Variables for calculations
    long temp_calc;
    long pressure_calc;

    long dT;

    //Now that we have a raw temperature, let's compute our actual.
    dT = result_temp - ((long) coefficient[5] << 8);
    temp_calc = (((long long) dT * coefficient[6]) >> 23) + 2000;

    //Now we have our first order Temperature, let's calculate the second order.
    long long T2, OFF2, SENS2, OFF, SENS; //working variables

    if (temp_calc < 2000)
    // If temp_calc is below 20.0C
    {
        T2 = 3 * (((long long) dT * dT) >> 33);
        OFF2 = 3 * ((temp_calc - 2000) * (temp_calc - 2000)) / 2;
        SENS2 = 5 * ((temp_calc - 2000) * (temp_calc - 2000)) / 8;

        if (temp_calc < -1500)
        // If temp_calc is below -15.0C
        {
            OFF2 = OFF2 + 7 * ((temp_calc + 1500) * (temp_calc + 1500));
            SENS2 = SENS2 + 4 * ((temp_calc + 1500) * (temp_calc + 1500));
        }
    }
    else
    // If temp_calc is above 20.0C
    {
        T2 = 7 * ((unsigned long long) dT * dT)
                / ((unsigned long long) 1 << 37);
        OFF2 = ((temp_calc - 2000) * (temp_calc - 2000)) / 16;
        SENS2 = 0;
    }
    // Now bring it all together to apply offsets

    OFF = ((long long) coefficient[2] << 16)
            + (((coefficient[4] * (long long) dT)) >> 7);
    SENS = ((long long) coefficient[1] << 15)
            + (((coefficient[3] * (long long) dT)) >> 8);

    temp_calc = temp_calc - T2;
    OFF = OFF - OFF2;
    SENS = SENS - SENS2;
    // Now lets calculate the pressure

    pressure_calc = (((SENS * result_pres) / 2097152) - OFF) / 32768;

    _temperature_actual = temp_calc;
    _pressure_actual = pressure_calc; // 10;// pressure_calc;

    temperature_reported = _temperature_actual / 100.0f;
    pressure_reported = _pressure_actual;
    pressure_reported = pressure_reported / 10.0f;
    __no_operation();
}

void main(void)
{
    power_init();
    switch_init();
    lcd_init();
    lcd_first();

    // P3.4 -> SMCLK
    P3DIR |= BIT4;
    P3SEL1 |= BIT4;

    // P5.0 -> SDA (UCB1CTL1)
    P5SEL1 &= ~BIT0;
    P5SEL0 |= BIT0;

    // P5.1 -> SCL
    P5SEL1 &= ~BIT1;
    P5SEL0 |= BIT1;

    UCB1CTLW0 = (UCSWRST | UCSSEL_2 | UCMST | UCMODE_3 | UCSYNC);
    UCB1BRW = 12;   // bit clock prescaler
    UCB1I2CSA = 0x77;   // slave address
    UCB1CTLW0 &= ~UCSWRST;

    UCB1IE |= (UCTXIE | UCRXIE);

    while (1)
        send_cmd(RRESET);
    __delay_cycles(4000);
    send_cmd(READ);
    recv_data();
    result_temp = ((unsigned long) RxBuffer[0] << 16)
            + ((unsigned long) RxBuffer[1] << 8) + RxBuffer[2];

    send_cmd(CONV_P);
    __delay_cycles(4000);
    send_cmd(READ);
    recv_data();
    result_pres = ((unsigned long) RxBuffer[0] << 16)
            + ((unsigned long) RxBuffer[1] << 8) + RxBuffer[2];

    calc_data();

    // make_sample_data();
}

#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
    switch (__even_in_range(UCB1IV, 12))
    {
    case 0:
        break;                           // Vector  0: No interrupts
    case 2:
        break;                           // Vector  2: ALIFG
    case 4:
        break;                           // Vector  4: NACKIFG
    case 6:                                  // Vector  6: STTIFG
        UCB1IFG &= ~UCSTTIFG;
        break;
    case 8:                                  // Vector  8: STPIFG
        UCB1IFG &= ~UCSTPIFG;
        if (RXByteCtr)                          // Check RX byte counter
            __bic_SR_register_on_exit(LPM0_bits + GIE);
        break;
    case 10:                                  // Vector 10: RXIFG
        *PRxData++ = UCB1RXBUF;                 // Get RX'd byte into buffer
        RXByteCtr++;
        break;
    case 12:                                  // Vector 12: TXIFG
        if (TXByteCtr)                          // Check TX byte counter
        {
            UCB1TXBUF = TXData;                   // Load TX buffer
            TXByteCtr--;                          // Decrement TX byte counter
        }
        else
        {
            UCB1CTLW0 |= UCTXSTP;                  // I2C stop condition
            UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
            __bic_SR_register_on_exit(LPM0_bits + GIE); // Exit LPM0
        }
        break;
    default:
        break;
    }

}