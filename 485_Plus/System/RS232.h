#ifndef __RS232_H
#define __RS232_H

#include "stdio.h"

void RS232_Init(void);
void RS232_SendByte(uint8_t Byte);
void RS232_SendString(char *String);
void RS232_Printf(char *format, ...);

#endif
