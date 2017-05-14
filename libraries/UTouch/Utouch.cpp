// 
// 
// 
#include <Arduino.h>
#include <XPT2046.h>
#include "UTOUCH.h"

UTouch::UTouch(byte tclk, byte tcs, byte tdin, byte dout, byte irq)
	{
		XPT2046 touch(/*cs=*/ tcs, /*irq=*/ irq);
	}

	void	UTouch::InitTouch(byte orientation ) {
		touch->begin(240,320);
		touch->setRotation(touch->ROT270);
		touch->setCalibration(176, 256, 1739, 1791);//banggood3.2"
		if(orientation==1)touch->setRotation(touch->ROT0);
		else touch->setRotation(touch->ROT270);
		

	}
	void	UTouch::read() { 
		uint16_t x, y;
			touch->getPosition(x,y);
			TP_X = x;
			TP_Y = y;
	}
	bool	UTouch::dataAvailable() { return touch->isTouching(); }
	int16_t UTouch::getX() { return UTouch::TP_X; }
	int16_t	UTouch::getY() { return UTouch::TP_Y; }
	void	UTouch::setPrecision(byte precision) {}


