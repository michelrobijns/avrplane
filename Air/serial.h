/*
 * serial.h
 *
 * Created: 8/4/2015
 * Author: Michel Robijns
 */

#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

extern char rxBuffer[15];
extern char txBuffer[15];

void setupSerial(void);
void sendTxBuffer(void);

#endif
