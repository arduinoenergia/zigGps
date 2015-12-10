/*
 * def.h
 *
 *  Created on: 25/ott/2014
 *      Author: massimo
 */

#ifndef DEF_H_
#define DEF_H_

#define BUFF_DIM 		128
#define	MAX_TOKEN		3

void tokCpm(int cont, char *tok, int * val);

///
/// struttura dati che permette di selezionare la UART su cui tramsettere e ricevere.
///
typedef struct{
	unsigned int 	selectedUart;
}__selUart;///


#endif /* DEF_H_ */
