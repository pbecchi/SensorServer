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
///Dust sensor from seeedstudio. No any external libraries are neccesary. Details to member functions see ISensor class documentation
class DustSensor : public ISensor
{
	int _port;
	unsigned long _starttime;
	unsigned long _sampletime_ms;
	unsigned long _lowpulseoccupancy;
	float _concentration;
public:
	DustSensor(int port,unsigned long sample_time_ms)
	{
		_port=port;
		_sampletime_ms=sample_time_ms;
		_lowpulseoccupancy=0;
		_concentration=0;
		pinMode(_port,INPUT);
		_starttime = millis();//get the current time;
	}
	const __FlashStringHelper* Name()
	{
		return F("Dust");
	}
	float LowMeasurementLimit()
	{
		return 0;
	}
	float HighMeasurementLimit()
	{
		return 100000;
	}
	int Precission()
	{
		return 0;
	}
	bool Measure(float &data)
	{
		unsigned long duration = pulseIn(_port, LOW);
		_lowpulseoccupancy += duration;
		unsigned long cur_sample_time=millis()-_starttime;
		if ((cur_sample_time > _sampletime_ms && _lowpulseoccupancy>0) || (cur_sample_time/_sampletime_ms)>10)//if the sampel time == 30s
		{
			float ratio = _lowpulseoccupancy/(cur_sample_time*10.0);  // Integer percentage 0=>100
			_concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
			_lowpulseoccupancy = 0;
			_starttime = millis();
		}
		data=_concentration;
		return true;
	}
};