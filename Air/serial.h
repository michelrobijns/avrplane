/*
 * serial.h
 *
 * Created: 8/4/2015
 * Author: Michel Robijns
 */

#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

#define RX_BUFFER_SIZE 15
#define TX_BUFFER_SIZE 15

extern char rxBuffer[RX_BUFFER_SIZE];
extern char txBuffer[TX_BUFFER_SIZE];

void setupSerial(void);
void sendtxBuffer(void);

#endif
