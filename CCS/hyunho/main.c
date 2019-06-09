#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"
#include "timer.h"
#include "switch.h"
#include "sensor.h"

void main(void)
{
    power_init();
    switch_init();
    lcd_init();
    lcd_first();
}

#pragma vector = EUSCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
    switch (__even_in_range(UCB1IV, USCI_I2C_UCBIT9IFG))
    {
    case 0:
        break;                           // Vector  0: No interrupts
    case 2:
        break;                           // Vector  2: ALIFG
    case 4:
        break;                           // Vector  4: NACKIFG
    case USCI_I2C_UCRXIFG0:                                  // Vector 10: RXIFG
        *PRxData++ = UCB1RXBUF;                 // Get RX'd byte into buffer
        RXByteCtr++;

        break;
    case USCI_I2C_UCTXIFG0:                                  // Vector 12: TXIFG
        if (TXByteCtr)                          // Check TX byte counter
        {
            UCB1TXBUF = TXData;                   // Load TX buffer
            TXByteCtr--;                          // Decrement TX byte counter
        }
        else
            UCB1CTLW0 |= UCTXSTP;                  // I2C stop condition
        break;
    default:
        break;
    }
}
