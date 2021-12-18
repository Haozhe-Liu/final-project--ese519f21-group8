
/*
 * uart.h
 *
 * Created: 2021/11/29 21:29:47
 *  Author: 18050
 */ 

#ifndef UART_H
#define UART_H

void UART_init(int prescale);

void UART_send( unsigned char data);

void UART_putstring(char* StringPtr);

#endif