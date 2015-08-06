/*
 * serial.h
 *
 * Created: 8/4/2015
 * Author: Michel Robijns
 * 
 * This file is part of avrplane which is released under the MIT license.
 * See the file LICENSE or go to http://opensource.org/licenses/MIT for full
 * license details.
 */

#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

extern char rxBuffer[15];
extern char txBuffer[15];

void setupSerial(void);
void sendTxBuffer(void);

#endif
