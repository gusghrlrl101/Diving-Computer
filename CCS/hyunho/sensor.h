#ifndef SENSOR_H_
#define SENSOR_H_

#define RD_PROM 0xA0
#define RRESET   0x1E
#define CONV_P  0x44
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

unsigned int coefficient[8];       // Coefficients;
unsigned long result_temp;
unsigned long result_pres;

inline void read_prom();
inline void send_cmd(unsigned char data);
inline void recv_data2();
inline void recv_data3();
void calc_data();
inline void sensor_enable();

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

    tmp_sensor = _temperature_actual / 10.0f;
    if (_pressure_actual < 10130)
        _pressure_actual = 10130;
    depth_sensor = (unsigned int) (((float) _pressure_actual - 10130.0f)
            / 101.3f);

    alarm = 0;
    if (depth_sensor < depth_before && depth_before - depth_sensor > 3)
        alarm = 1;
    depth_before = depth_sensor;

    __no_operation();
}

inline void read_prom()
{
    unsigned char j;
    for (j = 0; j <= 7; j++)
    {
        send_cmd(0xA0 + (j * 2));
        __delay_cycles(100);
        recv_data2();
        __delay_cycles(3000);
        coefficient[j] = (RxBuffer[0] << 8) | RxBuffer[1];
    }
}

inline void send_cmd(unsigned char data)
{
    TXData = data;
    TXByteCtr = 1;                          // Load TX byte counter
    while (UCB1CTLW0 & UCTXSTP)
    {

    }
    UCB1CTLW0 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __no_operation();
    __enable_interrupt();
}

inline void recv_data2()
{
    UCB1TBCNT = 2;  // number of bytes to be received
    PRxData = (unsigned char *) RxBuffer;    // Start of RX buffer
    RXByteCtr = 1;                          // Clear RX byte count
    while (UCB1CTLW0 & UCTXSTP)
    {

    }
    UCB1CTLW0 &= ~UCTR;
    UCB1CTLW0 |= UCTXSTT;
}

inline void recv_data3()
{
    UCB1TBCNT = 3;  // number of bytes to be received

    PRxData = (unsigned char *) RxBuffer;    // Start of RX buffer
    RXByteCtr = 1;                          // Clear RX byte count

    while (UCB1CTLW0 & UCTXSTP)
    {

    }
    UCB1CTLW0 &= ~UCTR;
    UCB1CTLW0 |= UCTXSTT;
}

inline void sensor_init()
{
    // set sensor i2c
    UCB1CTLW0 = (UCSWRST | UCSSEL_2 | UCMST | UCMODE_3 | UCSYNC);
    UCB1BRW = 11;   // bit clock prescaler
    UCB1I2CSA = 0x77;   // slave address
    UCB1CTLW0 &= ~UCSWRST;
    UCB1CTLW1 |= UCASTP_2;
    UCB1IE |= (UCTXIE0 | UCRXIE0);

    // P5.0 -> SDA (UCB1CTL1)
    P5SEL1 &= ~BIT0;
    P5SEL0 |= BIT0;

    // P5.1 -> SCL
    P5SEL1 &= ~BIT1;
    P5SEL0 |= BIT1;

    __no_operation();
    __enable_interrupt();
    send_cmd(RRESET);
    __delay_cycles(8000);
}

inline void sensor_enable()
{
    __no_operation();
    __enable_interrupt();

    read_prom();
    __delay_cycles(3000);

    send_cmd(CONV_T);
    __delay_cycles(3000);
    send_cmd(READ);
    __delay_cycles(1000);
    recv_data3();
    __delay_cycles(3000);
    result_temp = ((unsigned long) RxBuffer[0] << 16)
            + ((unsigned long) RxBuffer[1] << 8) + RxBuffer[2];

    send_cmd(CONV_P);
    __delay_cycles(3000);
    send_cmd(READ);
    __delay_cycles(1000);
    recv_data3();
    __delay_cycles(3000);
    result_pres = ((unsigned long) RxBuffer[0] << 16)
            + ((unsigned long) RxBuffer[1] << 8) + RxBuffer[2];

    calc_data();
}

#endif /* SENSOR_H_ */
