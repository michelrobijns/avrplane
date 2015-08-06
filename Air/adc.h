/*
 * adc.h
 *
 * Created: 8/4/2015
 * Author: Michel Robijns
 * 
 * This file is part of avrplane which is released under the MIT license.
 * See the file LICENSE or go to http://opensource.org/licenses/MIT for full
 * license details.
 */

#ifndef ADC_H_INCLUDED
#define ADC_H_INCLUDED

extern uint16_t voltage;

void setupADC(void);
void startConversion(void);
void updateADCs(void);

#endif
