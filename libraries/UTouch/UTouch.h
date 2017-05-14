#include <XPT2046.h>
#ifndef UTouch_h
#define UTouch_h
#include <Arduino.h>
#define UTOUCH_VERSION	124

#define PORTRAIT			0
#define LANDSCAPE			1

#define PREC_LOW			1
#define PREC_MEDIUM			2
#define PREC_HI				3
#define PREC_EXTREME		4

extern XPT2046 touch;

class UTouch
{
public:
	int16_t	TP_X, TP_Y;
	XPT2046* touch;

	UTouch(uint8_t tclk, byte tcs, byte tdin, byte dout, byte irq);

	void	InitTouch(byte orientation=LANDSCAPE );
	void	read();
	bool	dataAvailable();
	int16_t	getX();
	int16_t	getY();
	void	setPrecision(byte precision);
	

	//void	calibrateRead();

};


#endif