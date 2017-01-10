/***
	bitlash.h

	Bitlash is a tiny language interpreter that provides a serial port shell environment
	for bit banging and hardware hacking.

	See the file README for documentation.

	Bitlash lives at: http://bitlash.net
	The author can be reached at: bill@bitlash.net

	Copyright (C) 2008-2013 Bill Roy

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

***/
#ifndef _BITLASH_H
#define _BITLASH_H

#if defined(__x86_64__) || defined(__i386__)
#define UNIX_BUILD 1
#elif defined(__SAM3X8E__)
#define ARM_BUILD 1
#elif (defined(__MK20DX128__) || defined(__MK20DX256__)) && defined (CORE_TEENSY)
  // Teensy 3
  #define ARM_BUILD 2
#elif defined(PART_LM4F120H5QR) //support Energia.nu - Stellaris Launchpad / Tiva C Series 
#define ARM_BUILD  4 //support Energia.nu - Stellaris Launchpad / Tiva C Series  
#else
#define AVR_BUILD 1
#endif
#include<Arduino.h>
#ifndef ESP8266
#if defined(AVR_BUILD)
#include "avr/io.h"
#include "avr/pgmspace.h"
#include "avr/interrupt.h"
#endif

#if defined(AVR_BUILD) || defined(ARM_BUILD)
#include "string.h"
#include "ctype.h"
#include "setjmp.h"
#endif

// Unix includes
#if defined(UNIX_BUILD)
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "ctype.h"
#include "setjmp.h"
#include <time.h>
#include <sys/types.h>
#include <errno.h>
//#include <unistd.h>
#endif
#endif //esp8266
#ifndef byte
#define byte uint8_t
#endif


////////////////////////////////////////////////////
// GLOBAL BUILD OPTIONS
////////////////////////////////////////////////////
//
// Enable LONG_ALIASES to make the parser recognize analogRead(x) as well as ar(x), and so on
// cost: ~200 bytes flash
//#define LONG_ALIASES 1

//
// Enable PARSER_TRACE to make ^T toggle a parser trace debug print stream
// cost: ~400 bytes flash
//#define PARSER_TRACE 1



////////////////////////////////////////////////////
//
//	ARDUINO BUILD OPTIONS
//
////////////////////////////////////////////////////
//
#if defined(HIGH) || defined(ARDUINO)		// this detects the Arduino build environment

#define ARDUINO_BUILD 1

//#define ARDUINO_VERSION 14	// working
//#define ARDUINO_VERSION 15 	// working
#define ARDUINO_VERSION 16		// working, released
#include <pgmspace.h>
// the serial support, she is changing all the time
#ifndef ESP8266
#if ARDUINO_VERSION >= 15
#define beginSerial Serial.begin
#define serialAvailable Serial.available
#define serialRead Serial.read

#if defined(ARDUINO) && ARDUINO >= 100
	#define serialWrite Serial.write
#else
	#define serialWrite Serial.print
#endif

#endif
#endif
// Arduino version: 11 - enable by hand if needed; see bitlash-serial.h
//#define ARDUINO_VERSION 11


#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
	#define prog_char char PROGMEM
	#define prog_uchar char PROGMEM
#else
	#include "WProgram.h"
	#include "WConstants.h"
#endif


// Enable Software Serial tx support for Arduino
// this enables "setbaud(4, 4800); print #4:..."
// at a cost of about 400 bytes (for tx only)
//
#define SOFTWARE_SERIAL_TX 1
#define HARDWARE_SERIAL_TX 1

#define MINIMUM_FREE_RAM 50

#else
#define HIGH 1
#define LOW 0
#endif		// HIGH: arduino build


///////////////////////////////////////////////////////
//
// ARDUINO ETHERNET BUILD OPTIONS
//
///////////////////////////////////////////////////////
//
// Enable WIZ_ETHERNET true to build for telnet access to the official Arduino
// WIZ-5100 Ethernet shield
//#define WIZ_ETHERNET 1
//
// Enable AF_ETHERNET to build for telnet access to the Adafruit Ethernet shield 
// configured per the pinout below
//
//#define AF_ETHERNET 1
//

///////////////////////////////////////////////////////
//	WIZNET ETHERNET CONFIGURATION
//
#ifdef WIZ_ETHERNET

//
// You'll need these two lines in your sketch, as of Arduino-0022:
//
//	#include <SPI.h>
//	#include <Ethernet.h>
//

byte mac[] 		= { 'b','i','t','l','s','h' };
byte ip[]  		= { 192, 168, 1, 27 };
byte gateway[] 	= { 192, 168, 1, 1 };
byte subnet[] 	= {255,255,255,0};
#define PORT 8080
Server server = Server(PORT);

#define beginSerial beginEthernet
#define serialAvailable server.available
#define serialRead server.available().read
#define serialWrite server.write
void beginEthernet(unsigned long baud) {
	Ethernet.begin(mac, ip, gateway, subnet);
	server.begin();
}
#endif	// WIZ_ETHERNET


///////////////////////////////////////////////////////
// ADAFRUIT XPORT ETHERNET CONFIGURATION
//
#ifdef AF_ETHERNET
#define NET_TX 2
#define NET_RX 3
#define SOFTWARE_SERIAL_RX 1
#define RXPIN NET_RX
#undef HARDWARE_SERIAL_TX		// sorry, no room for pin 0/1 hard uart
#define DEFAULT_OUTPIN NET_TX
#define BAUD_OVERRIDE 9600
#endif	// AF_ETHERNET



///////////////////////////////////////////////////////
//
// SANGUINO BUILD
//
///////////////////////////////////////////////////////
//
// SANGUINO is auto-enabled to build for the Sanguino '644
// if the '644 define is present
//
#if defined(__AVR_ATmega644P__)
#define SANGUINO

//void beginSerial(unsigned long baud) { Serial.begin(baud); }
//char serialAvailable(void) { return Serial.available(); }
//char serialRead(void) { return Serial.read(); }
//void serialWrite(char c) { return Serial.print(c); }

#ifndef beginSerial
#define beginSerial Serial.begin
#define serialAvailable Serial.available
#define serialRead Serial.read
#define serialWrite Serial.print
#endif

// Sanguino has 24 digital and 8 analog io pins
#define NUMPINS (24+8)

// Sanguino primary serial tx output is on pin 9 (rx on 8)
// Sanguino alternate hardware serial port tx output is on pin 11 (rx on 10)
#define SANGUINO_DEFAULT_SERIAL 9
#define SANGUINO_ALTERNATE_SERIAL 11
#define DEFAULT_OUTPIN SANGUINO_DEFAULT_SERIAL
#define ALTERNATE_OUTPIN SANGUINO_ALTERNATE_SERIAL

#endif	// defined (644)


///////////////////////////////////////////////////////
//
// MEGA BUILD
//
//	Note: These are speculative and untested.  Feedback welcome.
//
///////////////////////////////////////////////////////
//
// MEGA is auto-enabled to build for the Arduino Mega or Mega2560
// if the '1280/'2560 define is present
//
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define MEGA 1
#endif

#if defined(MEGA)
#define beginSerial Serial.begin
#define serialAvailable Serial.available
#define serialRead Serial.read
#define serialWrite Serial.print

// MEGA has 54 digital and 16 analog pins
#define NUMPINS (54+16)

// Mega primary serial tx output is on pin 1 (rx on 0)
// Mega alternate hardware serial port tx output is on pin 18 (rx on 19)
// TODO: Support for hardware serial uart2 and uart3
//
#define MEGA_DEFAULT_SERIAL 1
#define MEGA_ALTERNATE_SERIAL 18
#define DEFAULT_OUTPIN MEGA_DEFAULT_SERIAL
#define ALTERNATE_OUTPIN MEGA_ALTERNATE_SERIAL

#endif	// defined (1280)

#if defined(__AVR_ATmega64__)

#define beginSerial Serial1.begin
#define serialAvailable Serial1.available
#define serialRead Serial1.read
#define serialWrite Serial1.print

#define NUMPINS (53)
#endif
#define TINY_BUILD 1


///////////////////////////////////////////////////////
//
//	TINY BUILD OPTIONS
//
#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny84__)
#define TINY_BUILD 1
#undef MINIMUM_FREE_RAM
#define MINIMUM_FREE_RAM 20
#define NUMPINS 6
//#undef HARDWARE_SERIAL_TX
#undef SOFTWARE_SERIAL_TX
//#define SOFTWARE_SERIAL_TX 1

//#include "usbdrv.h"

#endif		// TINY_BUILD



///////////////////////////////////////////////////////
//
//	AVROPENDOUS and TEENSY BUILD OPTIONS
//
#if defined(__AVR_AT90USB162__)

//#define AVROPENDOUS_BUILD
#if defined(AVROPENDOUS_BUILD)
#define MINIMUM_FREE_RAM 20
#define NUMPINS 24
#undef HARDWARE_SERIAL_TX
#undef SOFTWARE_SERIAL_TX
void beginSerial(unsigned long baud) { ; }
#define serialAvailable usbAvailable
#define serialRead usbRead
#define serialWrite usbWrite
#include <util/delay.h>
#endif	// defined AVRO

#define TEENSY
#ifdef TEENSY
#endif	// defined TEENSY

#endif	// defined '162


///////////////////////////////////////////////////////
//
//	ATMega32U4 BUILD OPTIONS
//
#if defined(__AVR_ATmega32U4__)

//#define AVROPENDOUS_BUILD
#if defined(AVROPENDOUS_BUILD)
#define MINIMUM_FREE_RAM 50
#define NUMPINS 40
#undef HARDWARE_SERIAL_TX
#undef SOFTWARE_SERIAL_TX
void beginSerial(unsigned long baud) { ; }
#define serialAvailable usbAvailable
#define serialRead usbRead
#define serialWrite usbWrite
#include <util/delay.h>
#endif	// AVRO

#define TEENSY2
#if defined(TEENSY2)
#endif	// TEENSY2

#endif	// defined '32U4


///////////////////////////////////////////////////////
//
// SD CARD SUPPORT: Enable the SDFILE define for SD card script-in-file support
//
//#define SDFILE


///////////////////////////////////////////////////////
//
//	Unix build options
//	(not working)
//
//	> gcc bitlash.cpp -D UNIX_BUILD
//
#ifdef UNIX_BUILD
#define MINIMUM_FREE_RAM 200
#define NUMPINS 32
#undef HARDWARE_SERIAL_TX
#undef SOFTWARE_SERIAL_TX
#define beginSerial(x)

#define E2END 2047

#define uint8_t unsigned char
#define uint32_t unsigned long int
#define prog_char char
#define prog_uchar unsigned char
#define strncpy_P strncpy
#define strcmp_P strcmp
#define strlen_P strlen

#define PROGMEM
#define OUTPUT 1

#define pgm_read_byte(addr) (*(char*) (addr))
#define pgm_read_word(addr) (*(int *) (addr))

unsigned long millis(void);

#endif	// defined unix_build


////////////////////
//
//	ARM BUILD
#if defined(ARM_BUILD)
 #define prog_char char
#define prog_uchar byte
#define PROGMEM
#define pgm_read_byte(b) (*(char *)(b))
#define pgm_read_word(b) (*(int *)(b))
#define strncpy_P strncpy
#define strcmp_P strcmp
#define strlen_P strlen
#if ARM_BUILD==1
  #define E2END 4096
#else
  // Teensy 3
  #define E2END 2048
#endif

#endif


// numvar is 32 bits on Arduino and 16 bits elsewhere
#if (defined(ARDUINO_BUILD) || defined(UNIX_BUILD)) && !defined(TINY_BUILD)
typedef long int numvar;
typedef unsigned long int unumvar;
#else
typedef int numvar;
typedef unsigned int unumvar;
#endif		// arduino_build


#ifdef AVROPENDOUS_BUILD
// USB integration
uint8_t usbAvailable(void);
int usbRead(void);
void usbWrite(uint8_t);
void usbMouseOn(void);
void usbMouseOff(void);
void connectBitlash(void);
#endif	// avropendous


// Function prototypes


#if !USE_GPIORS
byte sym;			// current input symbol
byte inchar;		// Current parser character
#endif

/////////////////////////////////////////////
// bitlash-api.c
//
void initBitlash(unsigned long baud);	// start up and set baud rate
void runBitlash(void);					// call this in loop(), frequently
numvar doCommand(char *);					// execute a command from your sketch
void doCharacter(char);					// pass an input character to the line editor

//void flash(unsigned int, int);


/////////////////////////////////////////////
// bitlash-arduino.c
//
//#ifndef ARDUINO_BUILD
#if 0
void digitalWrite(uint8_t, uint8_t);
int digitalRead(uint8_t);
int analogRead(uint8_t);
void pinMode(uint8_t, uint8_t);
unsigned long millis(void);
void delay(unsigned long);
void delayMicroseconds(unsigned int);
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define INPUT 0
#define OUTPUT 1
#endif


/////////////////////////////////////////////
// bitlash-cmdline.c
//
#ifdef TINY_BUILD
byte putlbuf(char);
void initlbuf(void);
#endif

// String value buffer size
#ifdef AVR_BUILD
  #define STRVALSIZE 120
#else
  #define STRVALSIZE 512
#endif
#define STRVALLEN (STRVALSIZE-1)
#define LBUFLEN STRVALSIZE

extern char *lbufptr;
extern char lbuf[LBUFLEN];


/////////////////////////////////////////////
// bitlash-eeprom.c
//
int findKey(char *key);				// return location of macro keyname in EEPROM or -1
int getValue(char *key);			// return location of macro value in EEPROM or -1

int findoccupied(int);
int findend(int);
void eeputs(int);

#define EMPTY ((uint8_t)255)
#define STARTDB 0
#define FAIL ((int)-1)

/////////////////////////////////////////////
// External EEPROM (I2C)
//
//#define EEPROM_MICROCHIP_24XX32A	// Uncomment to enable EEPROM via I2C
									// Supports a Microchip 24xx32A EEPROM module attached to the I2C bus
									// http://ww1.microchip.com/downloads/en/DeviceDoc/21713J.pdf
									// Specifically, the DigiX has such a module onboard
									// https://digistump.com/wiki/digix/tutorials/eeprom

#define EEPROM_ADDRESS 0x50			// default EEPROM address for DigiX boards

////////////////////////
//
// EEPROM database begin/end offset
//
// Use the predefined constant from the avr-gcc support file
//
#define E2END 2048
#if defined(EEPROM_MICROCHIP_24XX32A)
	#define ENDDB 4095
	#define ENDEEPROM 4095
#else
	#define ENDDB E2END
	#define ENDEEPROM E2END
#endif

/////////////////////////////////////////////
// bitlash-error.c
//
#ifndef ESP8266
extern jmp_buf env;
#endif
#define X_EXIT 1
void fatal2(char c, char s) {};
void overflow(byte c) {};
void underflow(byte c) {};
void expected(byte c) {};
void expectedchar(byte c) {};
void unexpected(byte c) {};
void missing(byte c) {};
//void oops(int);						// fatal exit


/////////////////////////////////////////////
// bitlash-functions.c
//
typedef numvar (*bitlash_function)(void);
//void show_user_functions(void);

void dofunctioncall(byte);
//numvar func_free(void);
void make_beep(unumvar, unumvar, unumvar);

extern const /*prog_*/char  functiondict[]; //PROGMEM;  //==============================================
extern const prog_char aliasdict[];//PROGMEM;     //===========================================

void stir(byte);

/////////////////////////////////////////////
// bitlash-program.c
//
extern char startup[];


/////////////////////////////////////////////
// bitlash-serial.c
//
void printIntegerInBase(unumvar, uint8_t, numvar, byte);
void printInteger(numvar,numvar, byte);
void printHex(unumvar);
void printBinary(unumvar);
void spb(char c) { };//Serial.print(c);  }
void sp(const char * c) {};
void speol(void) {};

numvar func_printf(void); 
numvar func_printf_handler(byte,byte);

#ifdef SOFTWARE_SERIAL_TX_MIO
numvar setBaud(numvar, unumvar);
void resetOutput(void);
#endif

// serial override handling
#define SERIAL_OVERRIDE
#ifdef SERIAL_OVERRIDE_MIO
typedef void (*serialOutputFunc)(byte);
byte serialIsOverridden(void);
void setOutputHandler(serialOutputFunc);
void resetOutputHandler(void);
extern serialOutputFunc serial_override_handler;
#endif

#ifdef ARDUINO_BUILD_MIO
void chkbreak(void);
void cmd_print(void);
#endif
numvar func_printf_handler(byte, byte);


/////////////////////////////////////////////
// bitlash-taskmgr.c
//
#ifdef MIO
void initTaskList(void);
void runBackgroundTasks(void);
void stopTask(byte);
void startTask(int, numvar);
void snooze(unumvar);
void showTaskList(void);
extern byte background;
extern byte curtask;
extern byte suspendBackground;
#endif

/////////////////////////////////////////////
// eeprom.c
// they must live off piste due to aggressive compiler inlining.
//
#ifdef ESP8266
void eewrite(int, byte) {};
byte eeread(int) { return 0; }
#elif defined(AVR_BUILD)
void eewrite(int, byte) __attribute__((noinline));
byte eeread(int) __attribute__((noinline));

#elif defined(ARM_BUILD)
void eewrite(int, byte);
byte eeread(int);
extern char virtual_eeprom[];
void eeinit(void);
#endif


/////////////////////////////////////////////
// bitlash-interpreter.c
//
#ifdef MIO
numvar getstatementlist(void);
void domacrocall(int);
#endif

/////////////////////////////////////////////
// bitlash-parser.c
//
void vinit(void);							// init the value stack
void vpush(numvar);							// push a numvar on the stack
numvar vpop(void);							// pop a numvar
extern byte vsptr;
#define vsempty() vsptr==0
extern numvar *arg;								// argument frame pointer
numvar getVar(uint8_t id) ;					// return value of bitlash variable.  id is [0..25] for [a..z]
void assignVar(uint8_t id, numvar value) ;	// assign value to variable.  id is [0..25] for [a..z]
numvar incVar(uint8_t id) ;					// increment variable.  id is [0..25] for [a..z]

void fetchc();
void primec();
numvar espressione(char  st[]);
											
// parse context types
#define SCRIPT_NONE		0
#define SCRIPT_RAM 		1
#define SCRIPT_PROGMEM 	2
#define SCRIPT_EEPROM 	3
#define SCRIPT_FILE		4
// bitlash_instrem---------------
byte findscript(char *);
byte scriptfileexists(char *);  
numvar execscript(byte, numvar, char *);
void callscriptfunction(byte, numvar);

typedef struct {
	numvar fetchptr;
	byte fetchtype;
} parsepoint;

void markparsepoint(parsepoint *); //-----not used
void returntoparsepoint(parsepoint *, byte); //----------not used
//void primec(void);     //----------used by fetchc only
int stptr = 0;
byte inpStr[20];// = { "5+(1+2)*(4+6)/2;" };
    //---------------redefine______________________________________________________

void getsym(void);    ////local
void traceback(void);

numvar func_fprintf(void);

const prog_char *getmsg(byte); //============================================
void parsestring(void (*)(char));
void msgp(byte);
void msgpl(byte);
numvar getnum(void);
#ifdef MIO
void calleeprommacro(int);
#endif
void getexpression(void);
byte hexval(char);
byte is_end(void);
numvar getarg(numvar);
numvar isstring(void);
numvar getstringarg(numvar);
void releaseargblock(void);
void parsearglist(void);
extern const char reservedwords[];//=============================================



// Interpreter globals
extern byte fetchtype;		// current script type
extern numvar fetchptr;		// pointer to current char in input buffer
extern numvar symval;		// value of current numeric expression

#define USE_GPIORS defined(AVR_BUILD)

#ifndef GPIOR0 || GPIOR1
	#undef USE_GPIORS
#endif
#if (defined USE_GPIORS)
#define sym GPIOR0
#define inchar GPIOR1
#else
extern byte sym;			// current input symbol
extern byte inchar;		// Current parser character
#endif

#ifdef PARSER_TRACE
extern byte trace;
void tb(void);
#endif


// Expression result
extern byte exptype;				// type of expression: s_nval [or s_sval]
extern numvar expval;				// value of numeric expr or length of string

// Temporary buffer for ids
#define IDLEN 12
extern char idbuf[IDLEN+1];


// Strings live in PROGMEM to save ram
//
#define M_expected		0
#define M_unexpected	1
#define M_missing		2
#define M_string		3
#define M_underflow		4
#define M_overflow		5
#define M_ctrlc			6
#define M_ctrlb			7
#define M_ctrlu			8
#define M_exp			9
#define M_op			10
#define M_pfmts			11
#define M_eof			12
#define M_var			13
#define M_number		14
#define M_rparen		15
#define M_saved			16
#define M_eeprom		17
#define M_defmacro		18
#define M_prompt		19
#define M_char			20
#define M_stack			21
#define M_startup		22
#define M_id			23
#define M_promptid		24
#define M_functions		25
#define M_oops			26
#define M_arg			27
#define M_function		28


//	Names for symbols
//
//	Each symbol in the grammar is parsed to a unique symval enumerated here
//
//	One character symbols take their ascii value as their symval
//	Complex symbols have the high bit set so start at 128 (0x80)
//
#define s_eof			0
#define s_undef			(0 | 0x80)
#define s_nval			(1 | 0x80)
#define s_sval			(2 | 0x80)
#define s_nvar			(3 | 0x80)
#define s_le			(4 | 0x80)
#define s_ge			(5 | 0x80)
#define s_logicaland	(6 | 0x80)
#define s_logicalor		(7 | 0x80)
#define s_logicaleq		(8 | 0x80)
#define s_logicalne		(9 | 0x80)
#define s_shiftleft		(10 | 0x80)
#define s_shiftright	(11 | 0x80)
#define s_incr			(12 | 0x80)
#define s_decr			(13 | 0x80)
#define s_nfunct		(14 | 0x80)
#define s_if			(15 | 0x80)
#define s_while			(16 | 0x80)
#define s_apin			(17 | 0x80)
#define s_dpin			(18 | 0x80)
#define s_define		(19 | 0x80)
#define s_function		(20 | 0x80)
#define s_rm			(21 | 0x80)
#define s_run			(22 | 0x80)
#define s_ps			(23 | 0x80)
#define s_stop			(24 | 0x80)
#define s_boot			(25 | 0x80)
#define s_peep			(26 | 0x80)
#define s_help			(27 | 0x80)
#define s_ls			(28 | 0x80)
#define s_print			(29 | 0x80)
#define s_switch		(30 | 0x80)
#define s_return		(31 | 0x80)
#define s_returning		(32 | 0x80)
#define s_arg			(33 | 0x80)
#define s_else			(34 | 0x80)
#define s_script_eeprom	(35 | 0x80)
#define s_script_progmem (36 | 0x80)
#define s_script_file	(37 | 0x80)
#define s_comment		(38 | 0x80)


// Names for literal symbols: these one-character symbols 
// are represented by their 7-bit ascii char code
#define s_semi			';'
#define s_add			'+'
#define s_sub			'-'
#define s_mul			'*'
#define s_div			'/'
#define s_mod			'%'
#define s_lparen		'('
#define s_rparen		')'
#define s_dot			'.'
#define s_lt			'<'
#define s_gt			'>'
#define s_equals		'='
#define s_bitand		'&'
#define s_bitor			'|'
#define s_comma			','
#define s_bitnot		'~'
#define s_logicalnot	'!'
#define s_xor			'^'
#define s_colon			':'
#define s_pound			'#'
#define s_quote			'"'
#define s_dollars		'$'
#define s_lcurly		'{'
#define s_rcurly		'}'


#endif	// defined _BITLASH_H

// Get a statement
numvar getstatement(void) {
	numvar retval = 0;

#if !defined(TINY_BUILD) && !defined(UNIX_BUILD)
	chkbreak();
#endif

	if (sym == s_while) {
		// at this point sym is pointing at s_while, before the conditional expression
		// save fetchptr so we can restart parsing from here as the while iterates
		parsepoint fetchmark;
		markparsepoint(&fetchmark);
		for (;;) {
			returntoparsepoint(&fetchmark, 0);
			getsym(); 						// fetch the start of the conditional
			if (getnum()) {
				retval = getstatement();
				if (sym == s_returning) break;	// exit if we caught a return
			}
			else {
				//			skipstatement();
				break;
			}
		}
	}

	else if (sym == s_if) {
		getsym();			// eat "if"
		if (getnum()) {
			retval = getstatement();
			if (sym == s_else) {
				getsym();	// eat "else"
							//			skipstatement();
			}
		}
		else {
			//		skipstatement();
			if (sym == s_else) {
				getsym();	// eat "else"
				retval = getstatement();
			}
		}
	}
	else if (sym == s_lcurly) {
		getsym(); 	// eat "{"
		while ((sym != s_eof) && (sym != s_returning) && (sym != s_rcurly)) retval = getstatement();
		if (sym == s_rcurly) getsym();	// eat "}"
	}
	else if (sym == s_return) {
		getsym();	// eat "return"
		if ((sym != s_eof) && (sym != s_semi)) retval = getnum();
		sym = s_returning;		// signal we're returning up the line
	}

#if !defined(TINY_BUILD)
	else if (sym == s_switch) retval = getswitchstatement();


	else if (sym == s_function) cmd_function();
#endif
	else if (sym == s_run) {	// run macroname
		getsym();
		if ((sym != s_script_eeprom) && (sym != s_script_progmem) &&
			(sym != s_script_file)) unexpected(M_id);

		// address of macroid is in symval via parseid
		// check for [,snoozeintervalms]
		getsym();	// eat macroid to check for comma; symval untouched
		if (sym == s_comma) {
			vpush(symval);
			getsym();			// eat the comma
			getnum();			// get a number or else
								//		startTask(vpop(), expval);
		}
		//	else startTask(symval, 0);
	}

	else if (sym == s_stop) {
		getsym();
#if !defined(TINY_BUILD)
		if (sym == s_mul) {						// stop * stops all tasks
			initTaskList();
			getsym();
		}
		else if ((sym == s_semi) || (sym == s_eof)) {
			if (background) stopTask(curtask);	// stop with no args stops the current task IF we're in back
			else initTaskList();				// in foreground, stop all
		}
		else
#endif
			//			stopTask(getnum());
	}

	else if (sym == s_rm) {		// rm "sym" or rm *
		getsym();
#if !defined(TINY_BUILD)

		if (sym == s_script_eeprom) {
			eraseentry(idbuf);
		}
		else if (sym == s_mul) nukeeeprom();
		else
#endif

			if (sym != s_undef) expected(M_id);
		getsym();
	}

#if !defined(TINY_BUILD)
	else if (sym == s_ls) { getsym(); cmd_ls(); }
	else if (sym == s_boot) cmd_boot();
	else if (sym == s_ps) { getsym();	showTaskList(); }
	else if (sym == s_peep) { getsym(); cmd_peep(); }
	else if (sym == s_help) { getsym(); cmd_help(); }
#endif
	else if (sym == s_print) {
		getsym(); //cmd_print();
	}
	else if (sym == s_semi) { ; }	// ;)

#ifdef HEX_UPLOAD
									// a line beginning with a colon is treated as a hex record
									// containing data to upload to eeprom
									//
									// TODO: verify checksum
									//
	else if (sym == s_colon) {
		// fetchptr points at the byte count
		byte byteCount = gethex(2);		// 2 bytes byte count
		int addr = gethex(4);			// 4 bytes address
		byte recordType = gethex(2);	// 2 bytes record type; now fetchptr -> data
		if (recordType == 1) reboot();	// reboot on EOF record (01)
		if (recordType != 0) return;	// we only handle the data record (00)
		if (addr == 0) nukeeeprom();	// auto-clear eeprom on write to 0000
		while (byteCount--) eewrite(addr++, gethex(2));		// update the eeprom
		gethex(2);						// discard the checksum
		getsym();						// and re-prime the parser
	}
#endif

	else {
		getexpression();
		retval = expval;
	}

	if (sym == s_semi) getsym();		// eat trailing ';'
	return retval;
}
#undef STRINGPOOL
#ifndef ESP8266
#if defined(AVR_BUILD)
#include "avr/eeprom.h"
#endif
#endif
// Interpreter globals
byte fetchtype;		// current script type
numvar fetchptr;	// pointer to current char in script
numvar symval;		// value of current numeric expression

					// Expression result
byte exptype;				// type of expression: s_nval [or s_sval]
numvar expval;				// value of numeric expr or length of string

							// Temporary buffer for ids
char idbuf[IDLEN + 1];



const char strings[] = {
#if defined(TINY_BUILD)
	"exp \0unexp \0mssng \0str\0 uflow \0oflow \0\0\0\0exp\0op\0\0eof\0var\0num\0)\0\0eep\0:=\"\0> \0char\0stack\0startup\0id\0prompt\0\r\n\0\0\0"
#else
	"expected \0unexpected \0missing \0string\0 underflow\0 overflow\0^C\0^B\0^U\0exp\0op\0:xby+-*/\0eof\0var\0number\0)\0saved\0eeprom\0:=\"\0> \0char\0stack\0startup\0id\0prompt\0\r\nFunctions:\0oops\0arg\0function\0"
#endif
};
const char *msg = strings;
// get the address of the nth message in the table
const char *getmsg(byte id) {
	//const prog_char *msg = strings;
	while (id) { msg += *(msg)+1; id--; }
	return msg;
}


//#if defined(HARDWARE_SERIAL_TX) || defined(SOFTWARE_SERIAL_TX)
// print the nth string from the message table, e.g., msgp(M_missing);
void msgp(byte id) {
	const char	*msg = getmsg(id);
	for (;;) {
		char c = pgm_read_byte(msg++);
		if (!c) break;
		spb(c);
	}
}
void msgpl(byte msgid) { msgp(msgid); speol(); }
//#endif



// Token type dispatcher: based on initial character in the symbol
#define TOKENTYPES 9
typedef void(*tokenhandler)(void);

void skpwhite(void), parsenum(void), parseid(void), eof(void), badsym(void);
void chrconst(void), litsym(void), parseop(void);

tokenhandler tokenhandlers[TOKENTYPES] = {
	skpwhite,		// 0: whitespace -> skip
	parsenum,		// 1: digit -> number
	parseid,		// 2: letter -> identifier
	eof,			// 3: end of string
	badsym,			// 4: illegal starting char
	chrconst,		// 5: ' -> char const
	0,				// [deprecated/free was 6: " -> string constant]
	litsym,			// 7: sym is inchar itself
	parseop			// 8: single and multi char operators (>, >=, >>, ...) beginning with [&|<>=!+-]
};

//	The chartypes array contains a type code for each ascii character in the range 0-127.
//	The code corresponding to a character specifies which of the token handlers above will
//	be called when the character is seen as the initial character in a symbol.
#define np(a,b) ((a<<4)+b)
const char chartypes[] = {    											//    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	np(3,4), np(4,4),  np(4,4), np(4,4),  np(4,0), np(0,4),  np(4,0), np(4,4),	//0  NUL SOH STX ETX EOT ENQ ACK BEL BS  HT  LF  VT  FF  CR  SO  SI
	np(4,4), np(4,4),  np(4,4), np(4,4),  np(4,4), np(4,4),  np(4,4), np(4,4),	//1  DLE DC1 DC2 DC3 DC4 NAK SYN ETB CAN EM  SUB ESC FS  GS  RS  US
	np(0,8), np(7,7),  np(4,7), np(8,5),  np(7,7), np(7,8),  np(7,8), np(7,8),	//2   SP  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   slash
	np(1,1), np(1,1),  np(1,1), np(1,1),  np(1,1), np(8,7),  np(8,8), np(8,4),	//3   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
	np(4,2), np(2,2),  np(2,2), np(2,2),  np(2,2), np(2,2),  np(2,2), np(2,2),	//4   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
	np(2,2), np(2,2),  np(2,2), np(2,2),  np(2,2), np(2,4),  np(4,4), np(7,2),	//5   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
	np(4,2), np(2,2),  np(2,2), np(2,2),  np(2,2), np(2,2),  np(2,2), np(2,2),	//6   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
	np(2,2), np(2,2),  np(2,2), np(2,2),  np(2,2), np(2,7),  np(8,7), np(7,4) 	//7   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ DEL
};

// Return the chartype for a given char
byte chartype(byte c) {
	if (c > 127) return 4;	// illegal starting char but allowed in strconst
	byte entry = pgm_read_byte(chartypes + (c / 2));
	if (c & 1) return entry & 0xf;
	else return (entry >> 4);		// & 0xf;
}


byte isdigit(byte c) { return (chartype(c) == 1); }
byte isalpha(byte c) { return (chartype(c) == 2); }
byte isalnum(byte c) { return isalpha(c) || isdigit(c); }
byte tolower(byte c) {
	return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : c;
}
byte is_end(void) { return ((sym == s_eof) || (sym == s_semi)); }


// Tests on the symbol type
byte isrelop(void) {
	return ((sym == s_lt) || (sym == s_le)
		|| (sym == s_logicaleq) || (sym == s_logicalne)
		|| (sym == s_gt) || (sym == s_ge));
}
byte ishex(char c) {
	return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
}
byte hexval(char c) {
	if ((c >= '0') && (c <= '9')) return c - '0';
	return tolower(c) - 'a' + 10;
}


#ifdef PARSER_TRACE
byte trace;
#endif


//	Parse the next token from the input stream.
void getsym(void) {
	//	Serial.print(inchar);
	// dispatch to handler for this type of char
	(*tokenhandlers[chartype(inchar)])();
	//	Serial.print('>'); Serial.print(sym);
#ifdef PARSER_TRACE
	if (trace) {
		sp(" sym="); printInteger(sym, 0, ' '); sp(" v="); printInteger(symval, 0, ' '); spb(' ');
	}
#endif
}


#ifdef PARSER_TRACE
void tb(void) {		// print a mini-trace
	if (!trace) return;
	sp("@"); printHex((unsigned long)fetchptr); spb(' ');
	sp("s"); printHex(sym); spb(' ');
	sp("i"); printHex(inchar); speol();
}
#endif



////////////////////
///
///		Expression evaluation stack
///
#if defined(MEGA) || defined(UNIX_BUILD) || defined(ARM_BUILD)
#define VSTACKLEN 256
#else
#define VSTACKLEN 64
#endif
byte vsptr;			  		// value stack pointer
numvar *arg;				// argument frame pointer
numvar vstack[VSTACKLEN];  	// value stack


							//#define STRING_POOL
							////////////////////
							///
							///	String Pool
							///
							///		The string pool lives in the far end of the vstack.
							///		It holds string constants parsed from arg blocks.
							///		The pointers themselves are passed as arg(n) values.
							///		The whole string pool for an argblock is deallocated
							///		when the argblock is released.
							///
#if defined(STRING_POOL)

char *stringPool;

// push a character into the string pool
void spush(char c) {
	if (stringPool >= (char *)&vstack[vsptr]) overflow(M_string);
	*stringPool++ = c;
}

// push a string into the string pool
void strpush(char *ptr) {
	while (*ptr) spush(*ptr++);
	spush(0);
}

#endif	// STRING_POOL


void vinit(void) {
	vsptr = VSTACKLEN - 3;	// reserve two slots for callername and calleename
	arg = &vstack[vsptr];	// point the argblock at the stack base
	vpush(0);				// push a 0 there so arg(0) is 0 at the top
#if defined(STRING_POOL)
	stringPool = (char *)vstack;	// stringPool starts at unused base of vstack
	*stringPool = 0;				// make it look empty
#endif
}

void vpush(numvar x) {

#if defined(STRING_POOL)
	if ((char *)&vstack[vsptr] < stringPool) overflow(M_exp);
#else
	if (vsptr <= 0) overflow(M_exp);
#endif
	vstack[vsptr--] = x;
}

numvar vpop(void) {
	if (vsptr >= VSTACKLEN - 1) underflow(M_exp);
	return vstack[++vsptr];
}

void vop(byte op) {
	numvar x, y;
	x = vpop(); y = vpop();
	switch (op) {
	case s_add:			vpush(y + x);	break;
	case s_sub:			vpush(y - x);	break;
	case s_mul:			vpush(y * x);	break;
	case s_div:			vpush(y / x);	break;
	case s_mod:			vpush(y % x);	break;
	case s_lt:			vpush(y < x);	break;
	case s_gt:			vpush(y > x);	break;
	case s_le:	 		vpush(y <= x);	break;
	case s_ge:	 		vpush(y >= x);	break;
	case s_logicalne: 	vpush(y != x);	break;
	case s_logicaland:	vpush(y && x);	break;
	case s_logicalor:	vpush(y || x);	break;
	case s_logicaleq:	vpush(y == x);	break;
	case s_bitor:		vpush(y | x);	break;
	case s_bitand:		vpush(y & x);	break;
	case s_xor:			vpush(y ^ x);	break;
	case s_shiftleft:	vpush(y << x);	break;
	case s_shiftright:	vpush(y >> x);	break;
	default: 			unexpected(M_op);
	}
}



////////////////////
///
/// Argument Block handling
///

numvar getarg(numvar which) {
	if (which > arg[0]) missing(M_arg);
	return arg[-which];
}

#if defined(STRING_POOL)
numvar isstringarg(numvar which) {		// isstringarg() api for C user functions
	return ((arg[2] & (1 << (which - 1))) != 0);
}

//	Bitlash test function for isstr():
//	function stringy {i=1;while (i<=arg(0)) {print i,arg(i),isstr(i);i++}}

numvar isstring(void) {					// isstr() for Bitlash functions
										// we are interested in the type of args in our
										// parent's stack frame, the caller of isstr()
	numvar *parentarg = (numvar *)arg[3];
	return ((parentarg[2] & (1 << (getarg(1) - 1))) != 0);
}

numvar getstringarg(numvar which) {
	if (!isstringarg(which)) expected(M_string);
	return getarg(which);
}
#endif


void parsearglist(void) {
	vpush((numvar)arg);				// save base of current argblock
#if defined(STRING_POOL)
	vpush(0);							// argtype: argument type vector, initially 0
	vpush((numvar)stringPool);			// save stringPool base for later release
	strpush(idbuf);						// save called function's name as arg[-1]
#endif
	numvar *newarg = &vstack[vsptr];	// move global arg pointer to base of new block
	vpush(0);							// initialize new arg(0) (a/k/a argc) to 0

	if (sym == s_lparen) {
		getsym();		// eat arglist '('
		while ((sym != s_rparen) && (sym != s_eof)) {

#if defined(STRING_POOL)
			if (sym == s_quote) {
				vpush((numvar)stringPool);	// push the string pointer
				parsestring(&spush);		// parse it into the pool
				spush(0);					// and terminate it
				getsym();					// eat closing "

											// bug: more than 32 args fails here
				newarg[2] |= (1 << newarg[0]);	// argtype: set string bit for this arg
			}
			else
#endif
				vpush(getnum());				// push the value
			newarg[0]++;					// bump the count
			if (sym == s_comma) getsym();	// eat arglist ',' and go around
			else break;
		}
		if (sym == s_rparen) getsym();		// eat the ')'
		else expected(M_rparen);
	}
	arg = newarg;		// activate new argument frame
}


// release the top argblock once its execution context has expired
//
void releaseargblock(void) {
	vsptr += arg[0] + 1;				// pop all args en masse, and the count

#if defined(STRING_POOL)
										// deallocate the string pool slab used by this function
										// by popping the saved caller's stringPool
	stringPool = (char *)vpop();

	vpop();		// argtype: pop argument type storage
#endif

	arg = (numvar *)vpop();			// pop parent arg frame and we're back
}



// Statement labels
//
//	MUST BE IN ALPHABETICAL ORDER!
//
#if defined(TINY_BUILD)
const char reservedwords[] = { "boot\0if\0run\0stop\0switch\0while\0" };
const char reservedwordtypes[] = { s_boot, s_if, s_run, s_stop, s_switch, s_while };
#else
const char reservedwords[] = { "arg\0boot\0else\0function\0help\0if\0ls\0peep\0print\0ps\0return\0rm\0run\0stop\0switch\0while\0" };
const char reservedwordtypes[] = { s_arg, s_boot, s_else, s_function, s_help, s_if, s_ls, s_peep, s_print, s_ps, s_return, s_rm, s_run, s_stop, s_switch, s_while };
#endif

// find id in PROGMEM wordlist.  result in symval, return true if found.
byte findindex(char *id, const /*prog_char*/ char *wordlist, byte sorted) {
	symval = 0;
	//Serial.print(id);
	while (*(wordlist)) {
		//Serial.print(wordlist);
		int result = strcmp(id, wordlist);
		if (!result) { return 1;}
		else if (sorted && (result < 0)) break;	// only works if list is sorted!
		else {
			symval++;
			wordlist += strlen(wordlist) + 1;
		}
	}
	//Serial.print(symval);
	return 0;
}

// return the pin number from a2 or d13
#if defined(TINY_BUILD)
#define pinnum(id) (id[1] - '0')
#else
byte pinnum(char id[]) {
	//return atoi(id + 1);		// atoi is 60 bytes. this saves 36 net.
	char n = id[1] - '0';
	if (id[2]) n = (n * 10) + id[2] - '0';
	return n;
}
#endif



//////////
//
// Pin alias variables
//
// A pin alias is another way to refer to a digital or analog pin.
// The table below associates symbols with pins so you can say:
// 	> led=1		... instead of
//  > d13=1		... and similarly for analog pinvars like a5
//
//
//#define PIN_ALIASES 1

#ifdef PIN_ALIASES

#define PV_ANALOG 0x80	// bit flag for analog alias
#define PV_VAR 0x40		// bit flag for variable alias
#define PV_MASK 0x3f

const prog_char pinnames[] PROGMEM = {
	"tx\0rx\0led\0vin\0zed\0"
};
const prog_uchar pinvalues[] PROGMEM = {
	0, 1, 13, (PV_ANALOG | 1), (PV_VAR | 25)
};

byte findpinname(char *alias) {
	if (!findindex(alias, (const prog_char *)pinnames, 0)) return 0;		// sets symval
	byte pin = pgm_read_byte(pinvalues + symval);
	//sym = (pin & PV_ANALOG) ? s_apin : s_dpin;
	sym = (pin & PV_ANALOG) ? s_apin : ((pin & PV_VAR) ? s_nvar : s_dpin);
	symval = pin & PV_MASK;
	return 1;
}
#endif


// Numeric variables
#ifndef NUMVARS
#define NUMVARS 26
#endif
numvar vars[NUMVARS];		// 'a' through 'z'
void assignVar(byte id, numvar value) { vars[id] = value; }
numvar getVar(byte id) { return vars[id]; }
numvar incVar(byte id) { return ++vars[id]; }


// Token handlers to parse the various token types

// Skip to next nonblank and return the symbol therefrom
void skpwhite(void) {
	while (chartype(inchar) == 0) fetchc();
	getsym();
}

// Comment: Skip from // to end of line, return next symbol
void skipcomment(void) {
	while (sym == s_comment) {
		while (inchar && (inchar != '\n') && (inchar != '\r')) fetchc();
		if (!inchar) {
			sym = s_eof;
			return;
		}
		else {
			fetchc();	// eat \r or \n
			getsym();	// and tee up what's next
		}
	}
}

// Handle unexpected character
void badsym(void) {
	unexpected(M_char);
#if !defined(TINY_BUILD)
	printHex(inchar); speol();
#endif
}

// Parse a character constant of the form 'c'
void chrconst(void) {
	fetchc();
	symval = inchar;
	sym = s_nval;
	fetchc();
	if (inchar != '\'') expectedchar('\'');
	fetchc();		// consume "
}


const char twochartokens[] = { "&&||==!=++--:=>=>><=<<//" };
const char twocharsyms[] = {
	s_logicaland, s_logicalor, s_logicaleq, s_logicalne, s_incr,
	s_decr, s_define, s_ge, s_shiftright, s_le, s_shiftleft, s_comment
};

// Parse a one- or two-char operator like >, >=, >>, ...	
//const prog_char *tk = twochartokens;
void parseop(void) {
	sym = inchar;		// think horse not zebra
	fetchc();			// inchar has second char of token or ??
	const char *tk = twochartokens;
	byte index = 0;
	for (;;) {
		byte c1 = *tk++;
		if (!c1) return;
		byte c2 = *tk++;
		const char *ts = twocharsyms;
		if ((sym == c1) && (inchar == c2)) {
			sym = (byte)*(ts + index);
			fetchc();
			if (sym == s_comment) skipcomment();
			return;
		}
		index++;
	}
}

//	One-char literal symbols, like '*' and '+'.
void litsym(void) {
	sym = inchar;
	fetchc();
}

// End of input
void eof(void) {
	sym = s_eof;
}

// Parse a numeric constant from the input stream
void parsenum(void) {
	byte radix;
	radix = 10;
	symval = inchar - '0';
	for (;;) {
		fetchc();
		inchar = tolower(inchar);
		if ((radix == 10) && (symval == 0)) {
			if (inchar == 'x') { radix = 16; continue; }
			else if (inchar == 'b') { radix = 2; continue; }
		}
		if (isdigit(inchar)) {
			inchar = inchar - '0';
			if (inchar >= radix) break;
			symval = (symval*radix) + inchar;
		}
		else if (radix == 16) {
			if ((inchar >= 'a') && (inchar <= 'f'))
				symval = (symval*radix) + inchar - 'a' + 10;
			else break;
		}
		else break;
	}
	sym = s_nval;
}


// Parse an identifier from the input stream
void parseid(void) {
	char c = *idbuf = tolower(inchar);
	byte idbuflen = 1;
	fetchc();
	while (isalnum(inchar) || (inchar == '.') || (inchar == '_')) {
		if (idbuflen >= IDLEN) overflow(M_id);
		idbuf[idbuflen++] = tolower(inchar);
		fetchc();
	}
	idbuf[idbuflen] = 0;

	// do we have a one-char alpha nvar identifier?
	if ((idbuflen == 1) && isalpha(c)) {
		sym = s_nvar;
		symval = c - 'a';
	}

	// a pin identifier 'a'digit* or 'd'digit*?
	else if ((idbuflen <= 3) &&
		((c == 'a') || (c == 'd')) &&
		isdigit(idbuf[1]) && (
#if !defined(TINY_BUILD)
			isdigit(idbuf[2]) ||
#endif
			(idbuf[2] == 0))) {
		sym = (c == 'a') ? s_apin : s_dpin;
		symval = pinnum(idbuf);
	}

	// reserved word?
	else if (findindex(idbuf, (const prog_char *)reservedwords, 1)) {
		sym = pgm_read_byte(reservedwordtypes + symval);	// e.g., s_if or s_while
	}

	// function?
	else
		if (findindex(idbuf, (const prog_char *)functiondict, 1)) {
			sym = s_nfunct;
		}

#ifdef LONG_ALIASES
	else if (findindex(idbuf, (const prog_char *)aliasdict, 0)) sym = s_nfunct;
#endif

#ifdef PIN_ALIASES
	else if (findpinname(idbuf)) { ; }		// sym and symval are set in findpinname
#endif

	//else if (find_user_function(idbuf)) sym = s_nfunct;

	else findscript(idbuf);
}


//////////
//
//	findscript: look up a script, with side effects
//
byte findscript(char *idbuf) {
	/*
	// script function in eeprom?
	if ((symval = findKey(idbuf)) >= 0) sym = s_script_eeprom;

	#if !defined(TINY_BUILD)
	// script function in a file?
	else if (scriptfileexists(idbuf)) sym = s_script_file;

	// script in the built-ins table?
	//	else if (findbuiltin(idbuf)) {;}
	#endif

	else
	*/ {
		sym = s_undef;		// huh?
		return 0;
	}
return sym;
}


#define ASC_QUOTE		0x22
#define ASC_BKSLASH		0x5C


// Parse a "quoted string" from the input.
//
// Enter with sym = s_quote therefore inchar = first char in string
// Exit with inchar = first char past closing s_quote
//
// Callers will need to call getsym() to resume parsing
//
void parsestring(void(*charFunc)(char)) {

	for (;;) {

		if (inchar == ASC_QUOTE) {				// found the string terminator
			fetchc();							// consume it so's we move along
			break;								// done with the big loop
		}
		else if (inchar == ASC_BKSLASH) {		// bkslash escape conventions per K&R C
			fetchc();
			switch (inchar) {

				// pass-thrus
			case ASC_QUOTE:				break;	// just a dbl quote, move along
			case ASC_BKSLASH:			break;	// just a backslash, move along

												// minor translations
			case 'n': 	inchar = '\n';	break;
			case 't': 	inchar = '\t';	break;
			case 'r':	inchar = '\r';	break;

			case 'x':			// bkslash x hexdigit hexdigit	
				fetchc();
				if (ishex(inchar)) {
					byte firstnibble = hexval(inchar);
					fetchc();
					if (ishex(inchar)) {
						inchar = hexval(inchar) + (firstnibble << 4);
						break;
					}
				}
				unexpected(M_char);
				inchar = 'x';
				break;
			}
		}
		// Process the character we just extracted
		(*charFunc)(inchar);

		fetchc();
		if (!inchar) unexpected(M_eof);		// get next else end of input before string terminator
	}
}



void getexpression(void);

//
//	Recursive descent parser, old-school style.
//
void getfactor(void) {
	numvar thesymval = symval;
	byte thesym = sym;
	getsym();		// eat the sym we just saved

	switch (thesym) {
	case s_nval:
		vpush(thesymval);
		break;

	case s_nvar:
		if (sym == s_equals) {		// assignment, push is after the break;
			getsym();
			assignVar(thesymval, getnum());
		}
		else if (sym == s_incr) {	// postincrement nvar++
			vpush(getVar(thesymval));
			assignVar(thesymval, getVar(thesymval) + 1);
			getsym();
			break;
		}
		else if (sym == s_decr) {	// postdecrement nvar--
			vpush(getVar(thesymval));
			assignVar(thesymval, getVar(thesymval) - 1);
			getsym();
			break;
		}
		vpush(getVar(thesymval));			// both assignment and reference get pushed here
		break;

	case s_nfunct:
			dofunctioncall(thesymval);			// get its value onto the stack
		break;

		// Script-function-returning-value used as a factor
	case s_script_eeprom:				// macro returning value
										//	callscriptfunction(SCRIPT_EEPROM, findend(thesymval));
		break;

	case s_script_progmem:
		//	callscriptfunction(SCRIPT_PROGMEM, thesymval);
		break;

	case s_script_file:
		//	callscriptfunction(SCRIPT_FILE, (numvar)0);	// name implicitly in idbuf!
		break;

	case s_apin:					// analog pin reference like a0
		if (sym == s_equals) { 		// digitalWrite or analogWrite
			getsym();
			analogWrite(thesymval, getnum());
			vpush(expval);
		}
		else vpush(analogRead(thesymval));
		break;

	case s_dpin:					// digital pin reference like d1
		if (sym == s_equals) { 		// digitalWrite or analogWrite
			getsym();
			digitalWrite(thesymval, getnum());
			vpush(expval);
		}
		else vpush(digitalRead(thesymval));
		break;

	case s_incr:
		if (sym != s_nvar) expected(M_var);
		assignVar(symval, getVar(symval) + 1);
		vpush(getVar(symval));
		getsym();
		break;

	case s_decr:		// pre decrement
		if (sym != s_nvar) expected(M_var);
		assignVar(symval, getVar(symval) - 1);
		vpush(getVar(symval));
		getsym();
		break;

	case s_arg:			// arg(n) - argument value
		if (sym != s_lparen) expectedchar(s_lparen);
		getsym(); 		// eat '('
		vpush(getarg(getnum()));
		if (sym != s_rparen) expectedchar(s_rparen);
		getsym();		// eat ')'
		break;

	case s_lparen:  // expression in parens
		getexpression();
		if (exptype != s_nval) expected(M_number);
		if (sym != s_rparen) missing(M_rparen);
		vpush(expval);
		getsym();	// eat the )
		break;

		//
		// The Family of Unary Operators, which Bind Most Closely to their Factor
		//
	case s_add:			// unary plus (like +3) is kind of a no-op
		getfactor();	// scan a factor and leave its result on the stack
		break;			// done

	case s_sub:			// unary minus (like -3)
		getfactor();
		vpush(-vpop());	// similar to above but we adjust the stack value
		break;

	case s_bitnot:
		getfactor();
		vpush(~vpop());
		break;

	case s_logicalnot:
		getfactor();
		vpush(!vpop());
		break;

	case s_bitand:		// &var gives address-of-var; &macro gives eeprom address of macro
		if (sym == s_nvar) vpush((numvar)&vars[symval]);
		else if (sym == s_script_eeprom) vpush(symval);
		else expected(M_var);
		getsym();		// eat the var reference
		break;

	case s_mul:			// *foo is contents-of-address-foo; *foo=bar is byte poke assignment

						/*****
						// what is really acceptable for an lvalue here? ;)
						//	*y = 5 is failing now by assigning 5 to y before the * is dereferenced
						//	due to calling getfactor
						//	everything else works :(
						*****/
		getfactor();
#if 0
		if (sym == s_equals) {
			getsym();	// eat '='
			getexpression();
			*(volatile byte *)vpop() = (byte)expval;
			vpush((numvar)(byte)expval);
		}
		else
#endif
			vpush((numvar)(*(volatile byte *)vpop()));
		break;

	default:
		unexpected(M_number);
	}

}


// parseReduce via a template
// saves 100 bytes but eats stack like crazy
// while we're ram-starved this stays off
//#define USE_PARSEREDUCE

#ifdef USE_PARSEREDUCE
void parseReduce(void(*parsefunc)(void), byte sym1, byte sym2, byte sym3) {
	(*parsefunc)();
	while ((sym == sym1) || (sym == sym2) || (sym == sym3)) {
		byte op = sym;
		getsym();
		(*parsefunc)();
		vop(op);
	}
}
#endif


void getterm(void) {
#ifdef USE_PARSEREDUCE
	parseReduce(&getfactor, s_mul, s_div, s_mod);
#else
	getfactor();

	while ((sym == s_mul) || (sym == s_div) || (sym == s_mod)) {
		byte op = sym;
		getsym();
		getfactor();
		vop(op);
	}
#endif
}

void getsimpexp(void) {
#ifdef USE_PARSEREDUCE
	parseReduce(&getterm, s_add, s_sub, s_sub);
#else
	getterm();
	while ((sym == s_add) || (sym == s_sub)) {
		byte op = sym;
		getsym();
		getterm();
		vop(op);
	}
#endif
}

void getshiftexp(void) {
#ifdef USE_PARSEREDUCE
	parseReduce(&getsimpexp, s_shiftright, s_shiftleft, s_shiftleft);
#else
	getsimpexp();
	while ((sym == s_shiftright) || (sym == s_shiftleft)) {
		byte op = sym;
		getsym();
		getsimpexp();
		vop(op);
	}
#endif
}

void getrelexp(void) {
	getshiftexp();
	while (isrelop()) {
		byte op = sym;
		getsym();
		getshiftexp();
		vop(op);
	}
}

void getbitopexp(void) {
#ifdef USE_PARSEREDUCE
	parseReduce(&getrelexp, s_bitand, s_bitor, s_xor);
#else
	getrelexp();
	while ((sym == s_bitand) || (sym == s_bitor) || (sym == s_xor)) {
		byte op = sym;
		getsym();
		getrelexp();
		vop(op);
	}
#endif
}

// Parse an expression.  Result to expval.
void getexpression(void) {

#ifdef USE_PARSEREDUCE
	parseReduce(&getbitopexp, s_logicaland, s_logicalor, s_logicalor);
#else
	getbitopexp();
	while ((sym == s_logicaland) || (sym == s_logicalor)) {
		byte op = sym;
		getsym();
		getbitopexp();
		vop(op);
	}
#endif
	exptype = s_nval;
	expval = vpop();
}


// Get a number from the input stream.  Result to expval.
numvar getnum(void) {
	getexpression();
	if (exptype != s_nval) expected(M_number);
	return expval;
}

void fetchc(void) {
	fetchptr++;
	stptr++;
	//	inchar = *(char *)fetchptr;
	//	inchar = inpStr[fetchptr];
	primec();
#ifdef DEBUG_P
	Serial.print((char)inchar); Serial.print(',');
#endif
}
void primec() {
	inchar = inpStr[fetchptr];
}
numvar espressione(char  stringa[]) {
	fetchptr = 0;
	inchar = stringa[0];
	for (byte i = 0; i<strlen(stringa); i++)
		inpStr[i] = stringa[i];
	getexpression();
	return expval;
}