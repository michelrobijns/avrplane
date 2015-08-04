/*
 * adc.h
 *
 * Created: 8/4/2015
 * Author: Michel Robijns
 */

#ifndef ADC_H_INCLUDED
#define ADC_H_INCLUDED

extern uint16_t voltage;

void setupADC(void);
void startConversion(void);
void updateADCs(void);

#endif

