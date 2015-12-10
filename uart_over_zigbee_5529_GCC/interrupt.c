/*
 * interrupt.c
 *
 *  Created on: 08/mag/2015
 *      Author: massimo
 */

#include <msp430x552x.h>
#include <stdint.h>
#include <string.h>
#include "header/def.h"

#define 	S0	0
#define 	S1	1
#define 	S2	2
#define 	S3	3



volatile uint8_t uart1buffer[BUFF_DIM], uart1TXbuffer[BUFF_DIM], RX_PTR1 = 0, READ_PTR1 = 0, TX_PTR1 = 0, endMess1 = 0;
volatile uint8_t uart1tmpBuf[BUFF_DIM], sendMsg = 0, ind = 0;
volatile uint8_t GPSbuff[BUFF_DIM][2];
volatile unsigned char bufferRX0[BUFF_DIM], Wptr0 = 0, Rptr0 = 0, endMess0 = 0;
volatile unsigned char bufferTX0[BUFF_DIM];

// Echo back RXed character, confirm TX buffer is ready first
//#pragma vector=USCI_A0_VECTOR
//__interrupt void USCI_A0_ISR(void){

volatile unsigned char statoMsgB0 = S0;

void __attribute__((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR(void){
//#pragma vector=USCI_A0_VECTOR
//__interrupt void USCI_A0_ISR(void){

  //switch(__even_in_range(UCA0IV,4))
	switch(UCA0IV) {

  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG

	bufferRX0[Wptr0] = UCA0RXBUF;
	//UCA1TXBUF = bufferRX0[Wptr0];
#ifdef __CONSOLE__
	/// E' utile l'eco
	while (!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = bufferRX0[Wptr0];
#endif

	/// copia il carattere del GPS sul buffer della seriale 1
	if(bufferRX0[Wptr0] == '$' && statoMsgB0 == S0){
		/// e' arrivato il primo carattere della stringa NMEA
		/// e si inizia a memorizzare e processare
		statoMsgB0 = S1;
	}
	if (statoMsgB0 == S1){
		/// copiato il carattere nel buffer
		uart1TXbuffer[TX_PTR1++] = bufferRX0[Wptr0];
		TX_PTR1 &= BUFF_DIM - 1;
	}

#ifdef __WRAPPER__
	/// ripete il carattere ricevuto sulla seriale 1
	//if (UCA1IFG & UCTXIFG)
	//	UCA1TXBUF = bufferRX0[Wptr0];
#endif
	if (bufferRX0[Wptr0] == 0xD)
		statoMsgB0 = S2;
	if(bufferRX0[Wptr0] == 0xA && statoMsgB0 == S2){
		statoMsgB0 = S0;
		/// ricevuta una fine messaggio
		uart1TXbuffer[TX_PTR1++] = '\n';
		uart1TXbuffer[TX_PTR1++] = '\0';
		sendMsg = TX_PTR1 - RX_PTR1;
		strncpy(uart1tmpBuf, uart1TXbuffer, sendMsg);
		TX_PTR1 = RX_PTR1 = 0;
		UCA1TXBUF = '\r';
		UCA1IE |= UCTXIE; // Enable USCI_A1 TX interrupt
	}
   // while (!(UCA0IFG&UCTXIFG));
   // UCA0TXBUF =  bufferRX0[Wptr0];
//#ifdef HYPER
//    UCA1TXBUF = bufferRX0[Wptr0];           	// TX -> RXed character
//    if (bufferRX0[Wptr0] == '\r'){
//    	while (!(UCA0IFG&UCTXIFG));
//    	UCA0TXBUF = '\n';
//    	endMess0 = 1;
//    }
//#endif
	//UCA0TXBUF = bufferRX0[Wptr0];
	if (bufferRX0[Wptr0] == 0x0D)
		endMess0 = 1;
	Wptr0++;
	Wptr0 &= BUFF_DIM - 1;
	break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}

// Echo back RXed character, confirm TX buffer is ready first
__attribute__((interrupt(USCI_A1_VECTOR)))
static void USCI_A1_ISR(void){
//#pragma vector=USCI_A1_VECTOR
//__interrupt void USCI_A1_ISR(void){

  //switch(__even_in_range(UCA1IV,4))
	unsigned int stato;
	/// salva lo stato del registro dei flag
	stato = UCA1IV;
	stato &= BIT2 | BIT1;

	if (stato & BIT1)	 // Vector 2 - RXIFG
  //switch(UCA1IV)
	  {
	  //case 0:break;                             // Vector 0 - no interrupt
	  //case 2:                                   // Vector 2 - RXIFG
	#ifdef HYPER
		while (!(UCA1IFG & UCTXIFG));             // USCI_A0 TX buffer ready?
	#endif
		uart1buffer[RX_PTR1] = UCA1RXBUF;
	#ifdef HYPER
		UCA1TXBUF = uart1buffer[RX_PTR1];           	// TX -> RXed character
		if (uart1buffer[RX_PTR1] == '\r'){
			while (!(UCA1IFG & UCTXIFG));
			UCA1TXBUF = '\n';
			endMess1 = 1;
		}
	#endif
		if (uart1buffer[RX_PTR1] == '\r' || uart1buffer[RX_PTR1] == '\n')
			endMess1 = 1;
		RX_PTR1++;
		RX_PTR1 &= BUFF_DIM - 1;
		//break;
	  //case 4:break;                             // Vector 4 - TXIFG
	  ///default: break;
	  }

	if (stato & BIT2){	 // Vector 4 - TXIFG
		if (sendMsg == 0){
			UCA1IE &= ~UCTXIE; // Disable USCI_A1 TX interrupt
			ind = 0;
		}
		/// trasmette il carattere del buffer finche' non arriva all'ultimo carattere
		UCA1TXBUF = uart1tmpBuf[ind++];
		sendMsg--;
	}
}


volatile unsigned int contatore = 0, tempo1S = 0;

// Timer1 A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void){
	/// l'interruzione avviene ogni 1000 ms e quindi ogni interruzione
	/// e' passato un secondo.

//	if (contatore < 9){
//		contatore++;
//	}
//	else{
		contatore = 0;
		tempo1S++;
		P4OUT ^= 0x80;
		//P6OUT ^= BIT3;
	//}
		/// avvio del  campionamento
		ADC12CTL0 |= ADC12SC;
}


volatile int valore, adcResults[6], flag = 0;
//#pragma vector = ADC12_VECTOR
//__interrupt void ADC12ISR (void){

__attribute__((interrupt(ADC12_VECTOR))) void ADC12_ISR(void){


	switch(ADC12IV){
		case  0: break;                           	// Vector  0:  No interrupt
		case  2: break;                           	// Vector  2:  ADC overflow
		case  4: break;                          	// Vector  4:  ADC timing overflow
		case  6:
			valore = ADC12MEM0;
			flag = 1;
		break;                           			// Vector  6:  ADC12IFG0
		case  8:
			valore = ADC12MEM1;
		break;										// Vector  8:  ADC12IFG1
		case 10: break;                           	// Vector 10:  ADC12IFG2
		case 12:
			adcResults[0] = ADC12MEM0;
			adcResults[1] = ADC12MEM1;
			adcResults[2] = ADC12MEM2;
			adcResults[3] = ADC12MEM3;

		break;                           	// Vector 12:  ADC12IFG3
		case 14: break;                           	// Vector 14:  ADC12IFG4
		case 16:									// Vector 16:  ADC12IFG5

			adcResults[0] = ADC12MEM0;
			adcResults[1] = ADC12MEM1;
			adcResults[2] = ADC12MEM2;
			adcResults[3] = ADC12MEM3;
			adcResults[4] = ADC12MEM4;
			adcResults[5] = ADC12MEM5;
			/// segnala la necessita' di aggiornare la media
			/*lettDistanzaPronta = TRUE;*/
			P4OUT ^= BIT1;


		break;
		case 18: break;                           // Vector 18:  ADC12IFG6
		case 20: break;                           // Vector 20:  ADC12IFG7
		case 22: break;                           // Vector 22:  ADC12IFG8
		case 24: break;                           // Vector 24:  ADC12IFG9
		case 26: break;                           // Vector 26:  ADC12IFG10
		case 28: break;                           // Vector 28:  ADC12IFG11
		case 30: break;                           // Vector 30:  ADC12IFG12
		//case 32: break;                           // Vector 32:  ADC12IFG13
		case 34: break;                           // Vector 34:  ADC12IFG14
		default: break;
	 }
}

