/*
 * init.h
 *
 *  Created on: 27/mag/2015
 *      Author: massimo
 */

#ifndef INIT_H_
#define INIT_H_

#define	 FDCO_INT  24576000

#ifdef __cplusplus
extern "C" {
#endif

void initTIMER(unsigned long FDCO, unsigned int tick);
void initADCmultiCH(unsigned char channel, unsigned char beg, unsigned char end);
void initADC(unsigned char channel);

void initUART0(unsigned long bitRate, unsigned long fdco);
void initUART1(unsigned long bitRate, unsigned long fdco);

unsigned long int setDCO_XTAL(unsigned long int mhz);

#define		DIM_GPS_CMD				64
#define		DIM_GPS_BUFF			128
#define 	OK						1
#define 	TRUE					1
#define 	FALSE					0
#define		GPS_ON						0x31
#define		GPS_OFF						0x32
#define		DIM_UART1_RX_BUFF			256

#ifdef __cplusplus
}
#endif


#endif /* INIT_H_ */
