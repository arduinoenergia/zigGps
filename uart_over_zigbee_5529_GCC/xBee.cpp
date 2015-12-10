/*
 * xBee.cpp
 *
 *  Created on: 27/mag/2015
 *      Author: massimo
 */



#include <msp430.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "xbee.h"
#include "init.h"


extern volatile uint8_t uart1buffer[16], RX_PTR1, READ_PTR1;
extern volatile uint8_t bufferRX0[], Wptr0, Rptr0, endMess0;
extern volatile unsigned int tempo1S;


xBee::xBee(){
	///
	/// usa l'uscita P1.6 come reset del chip, attivo basso.
	P1DIR |= BIT6;
	P1OUT &= ~BIT6;
	present = XBEE_NOT_PRESENT;
	Wptr0 = Rptr0 = 0;
	for(int i = 0; i < 4; i++){
		m_ADD_H[i] = 0;
		m_ADD_L[i] = 0;
	}
}

///
/// test sulla presenza del modulo radio
///
void xBee::test(){
	/// bufferRX0[BUFF_DIM], Wptr0 = 0, Rptr0 = 0, endMess0 = 0;
	volatile uint32_t rit;
	uint32_t i, stato = 0;
	/// toglie il reset
	P1OUT |= BIT6;
	/// inizializza la seriale 1 alla velocita' indicata
	initUART1(115200, FDCO_INT);
	m_Baud = 115200;
	for (rit = 1000; rit > 0; rit--);
	/// invia la richiesta di poll al modulo
	READ_PTR1 = RX_PTR1 = 0;
	for (i = 0; i < 3; i++){
		/// scrive nella uart.
		while(!(UCA0IFG & UCTXIFG));
		UCA0TXBUF = (unsigned char) '+';
	}
	i = 0;
	status = CMD;
	/// gira per 1 s in attesa della risposta
	tempo1S = 0;
	while (tempo1S < 2){

		if (Rptr0 != Wptr0 && bufferRX0[Rptr0] == 'O' && stato == 0){
			/// e' arrivato il primo carattere della risposta
			stato = 1;
			Rptr0++;
			Rptr0 &= BUFF_DIM - 1;
		}
		if (Rptr0 != Wptr0 && bufferRX0[Rptr0] == 'K' && stato == 1){
			/// e' arrivato il secondo carattere della rispota
			stato = 2;
			Rptr0++;
			Rptr0 &= BUFF_DIM - 1;

		}
		if (Rptr0 != Wptr0 && bufferRX0[Rptr0] == 0xD && stato == 2){
			/// finito il messaggio di risposta
			/// il modulo e' presente
			present = XBEE_PRESENT;
			NumUart = 1;
			Rptr0++;
			Rptr0 &= BUFF_DIM - 1;
			/// legge l'indirizzo proprio del modulo
			readMyAdd();
			/// deve inviare la chiusura dei messaggi: ATCN
			sendString ("ATCN", 4);
			sendChar(0x0D);
			/// e' nello stato ricezione trasmissione
			status = TXRX;

			return;
		}
	}
	present = XBEE_NOT_PRESENT;
}


///
/// invia un carattere tramite modulo radio
inline void xBee::sendChar(char c){
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = (unsigned char) c;
}

/// invia n caratteri tramite modulo radio
void xBee::sendString (char *s, int n){
	int i  = 0;

	while((*s) != 0 && i < n){
		sendChar(*s);
		s++;
	}
}


///
/// carica il vettore dell'indirizzo
void xBee::readReg(uint8_t *reg){
	/// l'indirizzo SH e' lungo 4 bytes ma viene restituito come
	/// 8 caratteri per ciascun semibyte
	uint8_t i = 0, val = 0;
	while(Rptr0 != Wptr0 && i < 8){
		uint8_t tmp = bufferRX0[Rptr0];
		if ((tmp - '0') > 9){
			val |= tmp - '0' + 9;
		}
		else
			val |= tmp - '0';

		if (i % 2 == 0){
			/// sposto il semibyte verso sinistra
			val <<= 4;
		}
		else{
			reg[i / 2] = val;
			val = 0;
		}
		i++;
		Rptr0 = upRptr(Rptr0);
	}
	/// adesso deve eliminare il carattere 'carriage return'
	Rptr0 = Wptr0;
}



///
/// legge l'indirizzo proprio del modulo
void xBee::readMyAdd(){

	/// puo' essere usato solo quando il modulo e' nello stato comando e non TXRX
	if (status == CMD){
		endMess0 = 0;
		sendString ("ATSH", 4);
		sendChar(0x0D);
		tempo1S = 0;
		while (endMess0 == 0 && tempo1S < 2);
		/// arrivata la fine messaggio
		if(endMess0){
			endMess0 = 0;
			readReg(m_ADD_H);
		}
		else{
			for(int i = 0; i < 4; i++)
				m_ADD_H[i] = 255;
		}

		/// legge la parte bassa dell'indirizzo
		endMess0 = 0;
		sendString ("ATSL", 4);
		sendChar(0x0D);
		tempo1S = 0;
		while (endMess0 == 0 && tempo1S < 2);
		/// arrivata la fine messaggio
		if(endMess0){
			endMess0 = 0;
			readReg(m_ADD_L);
		}
		else{
			for(int i = 0; i < 4; i++)
				m_ADD_L[i] = 255;
		}

	}
}

///
/// funzioncina statica che aggiorna una variabile globale
unsigned char xBee::upRptr(unsigned char p){

	p++;
	p &= BUFF_DIM - 1;
	return p;

}

///
/// neorizza la velocita' di trasmissione
void xBee::setBaud(uint32_t valore){
	m_Baud = valore;
}
