/*
 * timers.h
 *
 * Created: 8/4/2015
 * Author: Michel Robijns
 */

#ifndef TIMERS_H_INCLUDED
#define TIMERS_H_INCLUDED

#include <avr/io.h>

extern uint16_t pwm[10];

void setupTimers(void);
extern void doAt500Hz(void);

#endif
