/*
 * uart.h
 *
 *  Created on: 25/ott/2014
 *      Author: massimo
 */

#ifndef UART_H_
#define UART_H_

#include "def.h"

#ifdef __cplusplus
extern "C" {
#endif


void initUART1(unsigned long bitRate, unsigned long fdco);
void initUART0(unsigned long bitRate, unsigned long fdco);
void setUart(__selUart *uartPtr, int canale);

#ifdef __cplusplus
}
#endif


#endif /* UART_H_ */
