/*
 * xBee.h
 *
 *  Created on: 27/mag/2015
 *      Author: massimo
 */

#ifndef XBEE_H_
#define XBEE_H_

/*
 * xbee.h
 *
 *  Created on: 08/mar/2015
 *      Author: massimo
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "header/def.h"

#define  XBEE_NOT_PRESENT	0
#define	 XBEE_PRESENT		1

#define	 TXRX				129
#define	 CMD				130

//la classe xBee permette di leggere le caratteristiche del modulo
// (indirizzi, potenza di trasmissione) e di modificarli
class xBee{
public:
	xBee();
	void test();
	void sendChar(char );
	void sendString (char *, int);
	/// occorre un metodo non bloccante che legge dalla seriale
	char* receive();
	void setBaud(uint32_t);


public:
	uint16_t 	present;
	uint16_t 	NumUart;
	/// velocita' di trasmissione
	uint32_t	m_Baud;

private:
	uint16_t 	status;
	/// indirizzi propri del modulo
	uint8_t 	m_ADD_H[4], m_ADD_L[4], MY[2];
	/// indirizzi del modulo ricevente
	uint8_t 	m_DESTADD_H[4], m_DESTADD_L[4];
	uint8_t 	m_DATA[32];


	/// metodi
	void readMyAdd(void);
	void readReg(uint8_t *reg);
	static inline unsigned char upRptr(unsigned char);

};





#endif /* XBEE_H_ */



