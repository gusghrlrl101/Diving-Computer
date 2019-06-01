/*
 * switch.h
 *
 *  Created on: 2019. 6. 1.
 *      Author: gusgh
 */

#ifndef SWITCH_H_
#define SWITCH_H_

#include "msp430fr5994.h"

void switch_init(void);

void switch_init(void)
{
    P5DIR &= ~BIT3;
    P5SEL1 &= ~BIT3;
    P5SEL0 &= ~BIT3;

    P5IE |= BIT3;
    P5IES &= ~BIT3;
    P5IFG &= ~BIT3;
}

#pragma vector=PORT5_VECTOR
__interrupt void _switch1(void)
{
    P5IFG &= ~BIT3;
    P1OUT ^= BIT2;
}

#endif /* SWITCH_H_ */
