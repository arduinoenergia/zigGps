/*
 * main.cpp
 *
 *  Created on: 08/mag/2015
 *      Author: massimo
 */

//	test on digi xbee radio module and flying test

#include <msp430x552x.h>
#include <stdio.h>
#include <math.h>
#include "init.h"
#include "xBee.h"
#include "header/uart.h"
#include "gps.h"


extern volatile unsigned char bufferRX0[], Wptr0, Rptr0, endMess0;
extern volatile int valore, flag;
extern volatile uint8_t uart1TXbuffer[], TX_PTR1;

__selUart uart;


int main(){

	volatile float temp;
	volatile int temp1;
	char str[24] = "$PMTK251,38400*/0", check;
	char str1[24] = "$PMTK251,57600*/0";
	char setup = 0;


	xBee modRadio;
	GPS posizione;
	WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
	setDCO_XTAL(FDCO_INT);
	/* inizialiizzazioni */
	P4DIR |= BIT7;
	P1DIR |= BIT0;
	P4OUT |= BIT7;
	P1OUT |= BIT0;

	/// comunicazione con zigbee e pc
	modRadio.test();

	/// selezione uart 1 per comunicazioni con PC
	setUart(&uart, 1);
	for (temp1 = 20000; temp1 > 0; temp1--);
	/// inizializza il timer 1 con clock del cristallo da 32768 a generare intervalli da 1000 ms (=1s)
    initTIMER(32768, 1000);
    ///inizializzazione del sensore di temperatura
	initADC(10);
	printf("\r\n*** *** *** \r\n*** Modulo test zigBee ***\r\n***\r\n");
	printf("*** Test GPS ***\r\n\r\n");
	///avvia le interruzioni
	__bis_SR_register(GIE);
	/// controlla se il modulo radio e' presente
	//modRadio.test();
	if (modRadio.present == XBEE_PRESENT){
		printf("modulo radio presente \r\n");
		P1OUT &= ~BIT0;
	}
	else
		printf("modulo radio ASSENTE! \r\n");
	setUart(&uart, 0);
	/// avvio del primo campionamento
	ADC12CTL0 |= ADC12SC;
	while(1)
		if (setup == 0){
			setup = 1;
			///imposta i messaggi di interesse: RMC e GGA
			posizione.composeCMD(posizione.ms, "$PMTK", "314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*", 42);
			check = posizione.checksum(posizione.ms);
			posizione.appendChecksum(check, posizione.ms);
			posizione.append(posizione.ms, "\r\n\0", 3);
			for (volatile int i = 0; i < 10000; i++);
			printf("%s", posizione.ms);
			for (volatile int i = 0; i < 10000; i++);
		}
	while(1){
		if(endMess0 == 1){
			if (posizione.isPresent == GPS_NOT_PRESENT){
				/// appena arrivato un messaggio dal GPS
				/// impsota la velocita' di trasmissione.
				posizione.isPresent = GPS_PRESENT;
				endMess0 = 0;
				//printf("stringa\r\n");
				check = posizione.checksum(posizione.ms);
				posizione.appendChecksum(check, posizione.ms);
				posizione.append(posizione.ms, "\r\n\0", 3);
				printf("%s", posizione.ms);
	//			check = posizione.checksum(str1);
	//			posizione.appendChecksum(check, str1);
	//			posizione.append(str1, "\r\n\0", 3);
	//			printf("%s", str1);
				for (volatile int i = 0; i < 20000; i++);
				initUART0(57600, FDCO_INT);
				///imposta i messaggi di interesse: RMC e GGA
				posizione.composeCMD(posizione.ms, "$PMTK", "314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*", 42);
				check = posizione.checksum(posizione.ms);
				posizione.appendChecksum(check, posizione.ms);
				posizione.append(posizione.ms, "\r\n\0", 3);
				for (volatile int i = 0; i < 10000; i++);
				printf("%s", posizione.ms);
				for (volatile int i = 0; i < 10000; i++);
			}

			if (flag){
				/// c'e' il dato dal convertitore
				temp = float(valore) / 4095 * 45.82;
				temp = ceil(temp);
				temp1 = (int) temp;
				//printf("temperatura: %d\r\n", temp1);
				flag = 0;
	//			for(valore = 0; pippo < 4; pippo++){
	//				uart1TXbuffer[pippo] = '0' + pippo;
	//			}
	//			uart1TXbuffer[4] = 0;
	//			TX_PTR1 = 0;
	//			UCA1IE |= UCTXIE; // enable  USCI_A1 TX interrupt

			}
		}
	}

	return 0;
}

