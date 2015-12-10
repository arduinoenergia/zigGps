/*
 * uart.h
 *
 *  Created on: 08/mag/2015
 *      Author: massimo
 */

#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
extern "C" {
#endif

void initUART0(unsigned long bitRate, unsigned long fdco);
void initUART1(unsigned long bitRate, unsigned long fdco);

#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */
