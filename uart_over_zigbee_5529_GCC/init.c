/*
 * init.c
 *
 *  Created on: 27/mag/2015
 *      Author: massimo
 */

#include <msp430.h>
#include "init.h"

///
/// TIMER A1 INIT
void initTIMER(unsigned long FDCO, unsigned int tick){

	unsigned int divisore;
	volatile unsigned long int freq = FDCO;
	///FDCO		22118400
	/// F0 		1046000
	/// impostare il timer
	TA1CCTL0 = CCIE;                          		// CCR0 interrupt enabled
	// SMCLK, upmode, divide by 8, clear TAR
	//TA1CTL = TASSEL_2 + ID_3 + MC_1 + TACLR;
	/// usa la sorgente costituita dal cristallo a 32768Hz
	TA1CTL = TASSEL_1 +  MC_1 + TACLR;
	// FDCO = FDCO/8
	//freq =  freq / 8;
	//divisore = FDCO / (64 * 10);					/// 22118400 / 640 = 34560 count
													/// Fck = 22118400 / 64 = 345600 => 100 ms
	//TA1CCR0 = (FDCO * tick) / 8;						/// 200 ms @1,046 MHz
	divisore = (1000 / tick);
	TA1CCR0 = freq / divisore;						/// 100 ms @FDCO MHz
	//TA1EX0	= 7;								/// source clock divided by 8

	//TA1CTL = TASSEL_2 + MC_1 + TACLR;        		// SMCLK, upmode,  clear TAR
}

///
/// ADC init with INTeRnAL REFERENCE
void initADC(unsigned char channel){
	P6SEL |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6;
	/// set reference register for internal reference: enable, 1.5V
	REFCTL0 = REFMSTR + REFVSEL_0 + REFON;
	/// disable the AD unit
	ADC12CTL0 = 0;
	/// set the AD to have internal ck, not divided, with single channel operation
	ADC12CTL1 =  ADC12SHP;
	/// 12 bit conversion
	ADC12CTL2 = ADC12RES_2;
	if (channel < 12){
		if (channel == 10){
		/// use of internal temperature sensor. It requires 100us sampling time, with a clock of 5MHz:
		/// 100 us * 5MHz = 500, => ADC12SHT0x = 1010b: 512 clock sample time
			ADC12CTL0 |= ADC12SHT0_10 + ADC12ON;
		/// Vref and channel 10 (Temperature sensor)
			ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;

		}
		else{
			ADC12CTL0 |= ADC12SHT0_3 + ADC12ON;
			ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_5;
		}
		__delay_cycles(300);                      // 35us delay to allow Ref to settle
		                                          // based on default DCO frequency.
		                                          // See Datasheet for typical settle
		                                          // time.
		ADC12IE = ADC12IE0;                       // Enable ADC12IFG.0
		/// Enable of conversion
		ADC12CTL0 |= ADC12ENC;

	}

}


///
/// ADC init with INTeRnAL REFERENCE
void initADCmultiCH(unsigned char channel, unsigned char beg, unsigned char end){

	/// init port P6.0 ... P6.5
	P6SEL |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5;
	/// set reference register for internal reference: enable, 2.5V
	//REFCTL0 = REFMSTR + REFVSEL_2 + REFON;
	REFCTL0 = REFMSTR + REFON;
	/// disable the AD unit
	ADC12CTL0 = 0;
	/// set the AD to have internal ck, not divided, with sequence of channel operation
	/// sequence start at ADC12MEM0 memory cell.
	ADC12CTL1 =  ADC12SHP + ADC12CONSEQ_1;
	//ADC12CTL1 =  ADC12SHP;

	/// 12 bit conversion
	ADC12CTL2 = ADC12RES_2;

	/// time of conversion: 6us * 6 channels + 6 * 13 clk cycles (1 clk = 0,2us) = 36 + 6 * 26 = 36 + 156 = 192 us
	ADC12CTL0 |= ADC12SHT0_3 + ADC12ON;			/// sample time = 32 clk cycles. At 5MHz => 0.2u * 32 = 6us
	ADC12CTL0 |= ADC12MSC;						/// enable multiple conversion

	/// set the last channel in the sequence
	ADC12MCTL0 = ADC12SREF_0 + ADC12INCH_0;
	ADC12MCTL1 = ADC12SREF_0 + ADC12INCH_1;
	ADC12MCTL2 = ADC12SREF_0 + ADC12INCH_2;
	//ADC12MCTL3 = ADC12SREF_1 + ADC12INCH_3;
	//ADC12MCTL4 = ADC12SREF_1 + ADC12INCH_4;

	/// set the last channel in the sequence
	ADC12MCTL3 = ADC12SREF_0 + ADC12INCH_3 	+ ADC12EOS;

	__delay_cycles(300);                      	// 35us delay to allow Ref to settle
		                                        // based on default DCO frequency.

	/// Enable of conversion
	ADC12CTL0 |= ADC12ENC;

	ADC12IE = ADC12IE3;                           // Enable ADC12IFG.5
	///while (ADC12CTL1 & ADC12BUSY);
	///lettura = ADC12MEM0;

}

void release(){
	printf("SW.REL:1.0.1\n");
}
