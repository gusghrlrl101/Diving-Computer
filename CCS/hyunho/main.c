#include "msp430fr5994.h"
#include "mem.h"
#include "lcd.h"
#include "timer.h"
#include "switch.h"

void main(void)
{
    power_init();
    switch_init();
    lcd_init();
    lcd_first();
    // make_sample_data();
}
