
#include "gps.h"
#include "init.h"
#include <string.h>
#include <msp430x552x.h>

//  "$PMTK251,38400*/0",
//				"$PMTK251,115200*/0"  */

GPS::GPS(){

	composeCMD(ms, "$PMTK", "251,57600*", 11);
	isPresent = GPS_NOT_PRESENT;
	/// velocita' iniziale al reset del modulo
	mBaud = 9600;
	/// deve provare due velocita: 9600 oppure 38400
	testGPS();
	if (isPresent != GPS_PRESENT){
		mBaud = 38400;
		testGPS();
	}

}

extern volatile unsigned int tempo1S;

/// test the presence of GPS
void GPS::testGPS(){
	
	/// il test del GPS puo' essere molto semplice: si imposta la seriale alla
	/// velocita' di default e si attende il messaggio
	uint8_t stato = 0;
	/// uart 0 per comunicazioni con modulo GPS
	initUART0(mBaud, FDCO_INT);
	// Enable USCI_A0 RX interrupt
	UCA0IE |= UCRXIE;
	tempo1S = 0;

	while(tempo1S < 3 && stato != 2){
		if (UCA0IFG & UCRXIFG){
			///arrivato un carattere
			if (UCA0RXBUF == '$')
				stato = 1;
			if (UCA0RXBUF == '\r' && stato == 1){
				/// la comunicazione dovrebbe essere stabilita

				stato = 2;
			}
		}
	}
	if (stato == 2)
		isPresent = GPS_PRESENT;
	else
		isPresent = GPS_NOT_PRESENT;
//
//	unsigned char cksum;
//	composeCMD(cmd,PREAMBLE,TEST,3);
//	//append(cmd, POST, 1);
//	cksum = checksum(cmd);
//	//append(cmd, POST, 1);
//	appendChecksum(cksum, cmd);
//	append(cmd, "\r\n\0", 3);
//
//	/// ADD HERE PRINTF(cmd) and wait reply
//	test = TESTMESSREQPENDING;
}


///
/// biuld a command following module syntax
///
void GPS::composeCMD(char *st, const char *preamble, const char *cmd, char lun){

	int i;
	/// clear mem
	for (i = 0; i< DIM_GPS_CMD; i++)
		*(st + i) = 0;
	st = (char *) memcpy(st, preamble, 5);
	st = strncat(st, cmd, lun);
	//st = strncat(st, "*", 1);
	//append(st, POST, 1);
}

/// 
/// append checksum at the end of string in BCD mode (eg 25 => 2 5) 
void GPS::appendChecksum(unsigned char check, char *st){
	short int  low, hi;
	/// scan string
	while (*st != NULL)
		st++;
	/// now add 2 ascii char in st
	low = (char) check % 16;
	hi = (char) check >> 4;
	///high checksum char
	if (hi >= 0 && hi < 10)
		*st = (char) hi + '0';
	else
		if (hi >= 10 && hi < 16)
			*st = (char) hi + 'A' - 10;
	st++;
	/// low checksum char
	if ( low >= 0 && low < 10)
		*st = (char) low + '0';
	else
		if (low >= 10 && low < 16)
			*st = (char) low + 'A' - 10;
	st++;
	/// NULL terminator
	*st = 0;
}


///
/// append cmd char after null terminator of string st
///
void GPS::append(char *st, const char *cmd, char lun){
	
	st = strncat(st, cmd, lun);
}

void copia(char *st1, char *st2){

	unsigned char i = 0;
	while (*st1 != 0){
		*(st2 + i) = *(st1 + i);
		i++;
	}
}

void copia1(char st1[], char st2[]){

	unsigned char i = 0;
	while (st1[i] != 0){
		st2[i] = st1[i];
		i++;
	}
}

///
/// compute the checksum of a string '*' terminated and append it after '*' symbol
///
unsigned char GPS::checksum(char *st){

	unsigned char cont = 0, check = 0;
	char *tok;
	//short int  low, hi;
	/// extract the string between $ and *
	tok = strtok(st, "$*");
	if (tok != NULL){
	while (*tok != 0 && cont++ < DIM_GPS_BUFF){
		check ^= *tok;
		tok++;
	}
	/// put again the symbol lost during operation
	*tok++ = '*';
	/// termina la stringa
	*tok = 0;
//	*tok++ = POST;
//		/// now add 2 ascii char in st
//		low = (char) check % 16;
//		hi = (char) check >> 4;
//		///high checksum char
//		if (hi >= 0 && hi < 10)
//			*tok = (char) hi + '0';
//		else
//			if (hi >= 10 && hi < 16)
//				*tok = (char) hi + 'A';
//		tok++;
//		/// low checksum char
//		if ( low >= 0 && low < 10)
//			*tok = (char) low + '0';
//		else
//			if (low >= 10 && low < 16)
//				*tok = (char) low + 'A';
//		tok++;
//		/// NULL terminator
//		*tok = 0;
	}
	return check;
}

/// 
/// try to extract checksum from an incoming string
///

//unsigned char extractCheck(const char *st){
//	
//	unsigned char cont = 0, low, hi;
//	if (st == NULL)
//		return 0;
//	while(*st != '\0' && cont < DIM_UART1_RX_BUFF){
//		if (*st == '*')
//		/// char before * found so exit from while
//			break;
//	  cont++;
//	  st++;
//	}
//	if (cont == DIM_UART1_RX_BUFF || *st == '\0')
//		cont = 0;
//	else{
//		/// string is well formed and it extract the checksum
//		low = *(st + 2);
//		hi = *(st + 1);
//		if (low >= 0x30 && low <= 0x39)
//			low -= '0';
//		else
//			low -= 'A';
//		if (hi >= 0x30 && hi <= 0x39)
//			hi -= '0';
//		else
//			hi -= 'A';
//	
//		cont = (hi << 4) + low;
//	}
//	return cont;
//}

///
/// verify if checksum in correct
///
char GPS::verifyChecksum( const char *st, unsigned char value){

	unsigned char chC;
	unsigned char ch1, ch2;
	ch1 = *st;
	if (ch1 >= 0x30 && ch1 <= 0x39)
		ch1 -= '0';
	else
		ch1 = ch1 - 'A' + 10;
	ch2 = *(st + 1);
	if (ch2 >= 0x30 && ch2 <= 0x39)
		ch2 -= '0';
	else
		ch2 = ch2 - 'A' + 10;
	chC = (ch1 << 4) + ch2;
	if (value == chC)
		return (char) OK;
	else
		return (char) 0;
}

//extern volatile unsigned char uart1RXBUFF[], NUM_MESS;
//extern volatile unsigned int U1_Rec_PTR, U1_Read_PTR, U1_end_MESS;
//extern volatile unsigned char FINE_GPS_MESS, INIZIO_GPS_MESS;
//static unsigned char newString = 0;
/////
///// analisi dei messaggi del gps
/////
//unsigned char parseGPSMess( data *d, GPSstatus *stPtr, GPSmessStatus *gpsMSt){
//	unsigned char  i, cksum;
//	int end_MESS, numTocopy;
//	char  *ptr, *ck, *lPtr;
//	/// e' arrivato un messaggio dal GPS
//	if (FINE_GPS_MESS || NUM_MESS){
//		/// bytes da dove ha scritto il gps a dove sono arrivato a leggere.
//		numTocopy = U1_end_MESS - U1_Read_PTR;
//		if (numTocopy > 128)
//			numTocopy = 128;
//		/// segnalo che il messaggio è stato elaborato e non e' piu' valido
//		FINE_GPS_MESS = FALSE;
//		NUM_MESS--;
//		/// copio in un buffer locale il messaggio arrivato
//		/// in modo da eliminare la dipendeza temporale tra messaggio ricevuto
//		/// e messaggio analizzato.
//		if (numTocopy >= 0){
//		/// non sono andato a capo nel buffer circolare
//			end_MESS = newString + numTocopy;
//			if (end_MESS > 128)
//				end_MESS = 128;
//			for (i = newString; i < end_MESS; i++)
//				d->gpsbuff[i] = uart1RXBUFF[U1_Read_PTR++];
//		}
//		else {
//			/// valore finale da raggiungere
//			end_MESS = newString + DIM_UART1_RX_BUFF - U1_Read_PTR;
//			if (end_MESS > 128)
//				end_MESS = 128;
//			/// qui carico da newString ad end_MESS
//			for (i = newString; i < end_MESS; i++)
//				/// primo blocco da copiare
//				d->gpsbuff[i] = uart1RXBUFF[U1_Read_PTR++];
//			if (end_MESS < 128){
//				/// puo' continuare a percorrere il buffer altrimenti occorre
//				///fermarsi e ricaricarlo al prossimo giro
//				U1_Read_PTR = 0;
////				for(; U1_Read_PTR < numTocopy - (end_MESS - newString) + DIM_UART1_RX_BUFF; i++)
////					d->gpsbuff[i] = uart1RXBUFF[U1_Read_PTR++];
//				for(; (U1_Read_PTR < numTocopy - (end_MESS - newString) + DIM_UART1_RX_BUFF) && (i < 128); i++)
//					d->gpsbuff[i] = uart1RXBUFF[U1_Read_PTR++];
//			}
//		}
//		/// ptr contiene la stringa senza $ e *
//		/// first of all, we set ptr on position of '$' char
//		ptr = strpbrk(d->gpsbuff, "$");
//		if (ptr == NULL){
//			d->isCHOK = 0;
//			return NUM_MESS;
//		}
//		ptr = strtok(ptr, "$*");
//		/// ck contiene la stringa a partire dal checksum
//		ck = strtok(NULL, "$*");
//		/// estratto tutto cio' che e' compreso tra $ e *
//		/// calcolo del checksum
//		cksum = checksum(ptr);
//		/// deve calcolare il checksum e verificare che torni
//		//char verifyChecksum( const char *st, unsigned char value)
//		if ( verifyChecksum( ck, cksum) == OK){
//			/// checksum is OK
//			d->isCHOK = 0;
//			/// torna, quindi vado con l'analisi della stringa;
//			ptr = strtok(ptr, ",");
//			/****  ANALISI DEL MESSAGGIO ****/
//			/// esce il comando: PMTK, GGL..., ....
//			if (strncmp(ptr, "PMTK001", 7) == 0){
//				/// e' un ack ad un precedente messaggio
//				lPtr = (char *) gpsMSt;
//				/// comando PTMK: ack e occore differenziare a quale comando si sta dando
//				/// l'ack ad esempio scorrendo la struct *gpsMSt
//				ptr = strtok(NULL, ",");
//				/// extract the command to ack to and check what command will be ack.
//				if (strncmp(ptr, "0", 1) == 0){
//					//gpsMSt->test = TESTMESSACK;
//					ptr = strtok(NULL, ",");
//					if (strncmp(ptr, "3",1) == 0){
//						/// ack con valore 3 significa che e' OK e lo restituisco alla parte
//						/// piu' alta del programma.
//						*lPtr = TESTMESSACK;
//						lPtr++;
//						stPtr->status = GPS_ON;
//					}
//				}
//			} else /// se non e' un camando PMTK e' un messaggio
//			if (strncmp(ptr, "GPGGA", 5) == 0){
//			///	$GPGGA,hhmmss.dd,xxmm.dddd,<N|S>,yyymm.dddd,<E|W>,v,s,d.d,h.h,M,g.g,M,a.a,xxxx*hh<CR><LF>
//			/// e.g. $GPGGA,114353.000,6016.3245,N,02458.3270,E,1,10,0.81,35.2,M,19.5,M,,*50
//			/// dati dal gps, occorre estrarre la latitudine ed inviarla al pc
//			/// occorre separare le virgole senza testo dal resto, perche' il modulo puo' esprimersi anche cosi'
//				char * cPtr = ptr;
//			  ptr = strtok(NULL, ",");
//				strcpy(d->UTCtime, ptr);
//				cPtr = ptr;
//				/// scorre UTCtime e si posiziona in fondo
//				cPtr = cPtr + strlen(cPtr);
//				if (*(cPtr + 1) == ','){
//					///dato non valido
//					stPtr->GGA = NOTVALID;
//					goto replaceGPSBuff;
//				}
//				ptr = strtok(NULL, ",");
//				strcpy(d->latitude, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->NS, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->longitude, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->EW, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->fixValid, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->numSat, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->HDOP, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->altitude, ptr);
//				i = 0;
//				while (i++ < 2)
//					ptr = strtok(NULL, ",");
//				strcpy(d->diffWGS84, ptr);
//				stPtr->GGA = VALID;
//			}
//			/// end of "GPGGA" message
//			else
//			if (strncmp(ptr, "GPGLL", 5) == 0){
//			/// dati dal gps, occorre estrarre la latitudine ed inviarla al pc
//				/*Example:
//				$GPGLL,6012.5674,N,02449.6545,E,072022.000,A,A*50
//				Format: $GPGLL,xxmm.dddd,<N|S>,yyymm.dddd,<E|W>,hhmmss.ddd,S,M*hh<CR><LF>;*/
//				ptr = strtok(NULL, ",");
//				strcpy(d->latitude, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->NS, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->longitude, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->EW, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->UTCtime, ptr);
//				ptr = strtok(NULL, ",");
//				if (*ptr == 'A')
//					stPtr->GLL |= VALID;
//				else
//				  stPtr->GLL &= NOTVALID;
//				ptr = strtok(NULL, ",");
//				if (*ptr == 'A')
//					stPtr->GLL |= AUTONOMOUS;
//				else
//					stPtr->GLL &= NOTAUTONOMOUS;
//			}
//			/// end of "GPGLL" message
//			else
//			if (strncmp(ptr, "GPRMC", 5) == 0){
//				/* 	Example:  $GPRMC,114353.000,A,6016.3245,N,02458.3270,E,0.01,0.00,121009,,,A*69
//						Format:   $GPRMC,hhmmss.dd,S,xxmm.dddd,<N|S>,yyymm.dddd,<E|W>,s.s,h.h,ddmmyy,d.d, <E|W>,M*hh<CR><LF> */
//				ptr = strtok(NULL, ",");
//				strcpy(d->UTCtime, ptr);
//				ptr = strtok(NULL, ",");
//
//				if (*ptr == 'A')
//					stPtr->RMC |= VALID;
//				else{
//					stPtr->RMC &= ~NOTVALID;
//					goto replaceGPSBuff;
//				}
//
//				ptr = strtok(NULL, ",");
//				strcpy(d->latitude, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->NS, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->longitude, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->EW, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->speedKn, ptr);
//				i = 0;
//				while (i++ < 2)
//					ptr = strtok(NULL, ",");
//				strcpy(d->UTCdate, ptr);
//				i = 0;
//				//while (i++ < 3)
//				ptr = strtok(NULL, ",");
//				if (*ptr == 'A')
//					stPtr->RMC |= AUTONOMOUS;
//				else
//					stPtr->RMC &= NOTAUTONOMOUS;
//			}
//			/// end of "GPRMC" message
//			else
//			if (strncmp(ptr, "GPGSV", 5) == 0){
//				/// al momento non lo gestisco.
//				stPtr->GSV &= NOTVALID;
//			}
//			/// end of "GPGSV" message
//			else
//			if (strncmp(ptr, "GPGSA", 5) == 0){
//				/* 	Example: $GPGSA,A,3,02,21,30,04,16,05,10,12,31,29,,,1.33,0.81,1.06*02
//						Format: $GPGSA,a,b,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,p.p,h.h,v.v*hh<CR><LF>
//				*/
//				i = 0;
//				while (i++ < 2)
//					ptr = strtok(NULL, ",");
//				if ( *(ptr + 2 ) == ','){
//					stPtr->GSA = NOTVALID;
//					goto replaceGPSBuff;
//				}
//				*(ck - 1) = 0;
//				ptr = ck - 15;
////				while (i++ < 13)
//				ptr = strtok(ptr, ",");
//				strcpy(d->PDOP, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->HDOP, ptr);
//				ptr = strtok(NULL, ",");
//				strcpy(d->VDOP, ptr);
//				stPtr->GSA = VALID;
//			}
//			/// end of "GPGSA" message
//			else
//			if (strncmp(ptr, "GPVTG", 5) == 0){
//				/* 	Example: $GPVTG,0.00,T,,M,0.00,N,0.00,K,A*3D
//						Format: $GPVTG,h.hh,T,m.m,M,s.ss,N,s.ss,K,M*hh<CR><LF>
//				*/
//				i = 0;
//				while (i++ < 5)
//					ptr = strtok(NULL, ",");
//				strcpy(d->speedKn, ptr);
//				i = 0;
//				while (i++ < 2)
//					ptr = strtok(NULL, ",");
//				strcpy(d->speedKM, ptr);
//				ptr = strtok(NULL, ",");
//				if (*ptr == 'A')
//				  stPtr->VTG |= VALID;
//				else
//					stPtr->VTG &= NOTVALID;
//			}
//			/// end of "GPVTG" message
//			else
//			if (strncmp(ptr, "GPZDA", 5) == 0){
//				/// non implementato
//				stPtr->ZDA &= NOTVALID;
//			}
//		}
//		else{
//			/// check sum errato.
//			d->isCHOK = 0;
//		}
//		/// riallineamento della stringa eliminando le parti analizzate.
//		unsigned char diff;
//	replaceGPSBuff:
//		if (NUM_MESS == 0){
//			newString = 0;
//		}
//		else{
//		diff = (ck + 4) - d->gpsbuff;									/// non dovrebbe essere mai negativo !!
//		for (i = 0; i < 128 - diff; i++)							/// se acccadesse, andrea a scrivere fuori dal vettore. Attenzione!!
//			d->gpsbuff[i] = d->gpsbuff[i + diff];
//		newString = i;
//		d->gpsbuff[0] ='$';
//		}
//	}
//	return NUM_MESS;
//}
//

