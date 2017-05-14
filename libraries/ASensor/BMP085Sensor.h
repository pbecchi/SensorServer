#pragma once
/*
  ASensor - Sensor library. Can be used standalone or in conjunction with AWind library
  Copyright (C)2015 Andrei Degtiarev. All right reserved
  
  You can always find the latest version of the library at 
  https://github.com/AndreiDegtiarev/ASensor

  This library is free software; you can redistribute it and/or
  modify it under the terms of the MIT license.
  Please see the included documents for further information.
*/
///Wrapper for BMP085 air pressure sensor. To use this sensor Adafruit_BMP085_Unified library has to be installed. Details to member functions see ISensor class documentation
class BMP085Sensor : public ISensor
{
#ifndef DEMO_SENSORS
	Adafruit_BMP085_Unified *_bmp ;
#endif
public:
	BMP085Sensor()
	/*	:ISensor(10085,
					   700,
					   1300,
					   700,
					   1300,
					   0,pause_length)*/
	{
#ifndef DEMO_SENSORS
 		_bmp = new Adafruit_BMP085_Unified(10085);
		if(!_bmp->begin())
		{
			/* There was a problem detecting the BMP085 ... check your connections */
			Serial.print(F("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!"));
		}
#endif
	}
	virtual const __FlashStringHelper* Name()
	{
		return F("BMP085");
	}
	virtual float LowMeasurementLimit()
	{
		return 700;
	}
	virtual float HighMeasurementLimit()
	{
		return 1300;
	}
	virtual int Precission()
	{
		return 0;
	}
	virtual bool Measure(float &data)
	{
#ifdef DEMO_SENSORS
		data=(float)rand()/RAND_MAX*100+1000;
		return true;
#else
		 sensors_event_t event;
		 _bmp->getEvent(&event);
		 if (event.pressure)
		 {
			data=event.pressure;
#ifdef DEBUG_AWIND
			out<<F("Pressure: ")<<data<<endln;
#endif
			return true;
		 }
		 return false;
#endif
	}
};