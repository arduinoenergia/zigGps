#ifndef _GPS_H_
#define _GPS_H_

#include <stdint.h>

///
/// serve a definire lo stato di un messaggio GPS. Ad esempio un TEST va confermato dal modulo
/// ma viene confermato subito o dopo un po?
///
//typedef struct{
//	char test;
//} _GPSmessStatus;
//
//typedef _GPSmessStatus GPSmessStatus;

#define	 GPS_NOT_PRESENT	0
#define	 GPS_PRESENT		1

class GPS{
public:
	GPS();
	void testGPS();
	void composeCMD(char *st, const char *preamble, const char *cmd, char lun);
	/// extract checksum from an incoming string
	unsigned char extractCheck(const char *);
	/// verify a checksum
	char verifyChecksum( const char *st, unsigned char value);


	char 		test;
	char 		isPresent;
	char 		ms[64];
	uint32_t 	mBaud;

//private:
	/// extract a checksum from string, after cut $ and * symbols
	unsigned char  checksum(char *);


	/// this function build a command
	void append(char *, const char *, char);
	void appendChecksum(unsigned char , char *);



};


///#ifndef __GNUC__
#ifdef __cplusplus
extern "C" {
#endif
///
/// GPS data struct with relevant infos
///
typedef struct{
	char newMESS;
	char isCHOK;
	char cmd;
	char UTCtime[16];
	char UTCdate[12];
	char latitude[16];
	char longitude[16];
	char NS[2];
	char EW[2];
	char elevation[8];
	char altitude[8];
	char diffWGS84[8];
	char numSat[4];
	char fixValid[2];
	char HDOP[8];
	char PDOP[8];
	char VDOP[8];
	char DOP[8];
	char speedKn[8];
	char speedKM[8];
	char gpsbuff[128];
	/// da completare 
} _GPSvalue;

///
/// GPS status with info field enabled
///
typedef struct {
	char status;		/// stato del messaggio ricevuto, da non confondere con ack di un comando
	char mode;
	char GGA;				///global positionig system: fixed data
	char GLL;				/// geographical position: latitude and longitude
	char RMC;				/// recommended minimum specific GNSS data
	char GSV;				/// Geographical satelites in view
	char GSA;				/// DOP and active satellites 
	char VTG;				/// Course over ground and ground speed
	char ZDA;				/// Zone time and date
} GPSstatus;

typedef _GPSvalue data;


///
/// gps functions header
///

#define   	PREAMBLE					"$PMTK"
#define 	POST						"*"
#define 	TEST						"000*"
#define		FULL_COLD_START				"104"
#define		SET_BAUD_RATE				"251"
///  	4800		9600		14400		19200  38400  57600   115200


#define		SET_NMEA_OUTPUT		"314"

#define 	NONE 						0
#define		DATA						'G'
#define		VALID						1
#define		NOTVALID					254
#define		AUTONOMOUS					2
#define		NOTAUTONOMOUS				253

#define		NOTESTMESS					0
#define		TESTMESSREQPENDING			1
#define		TESTMESSACK					2






unsigned char parseGPSMess(data *, GPSstatus *, char *);



#ifdef __cplusplus
}
#endif


#endif /*_GPS_H_*/
