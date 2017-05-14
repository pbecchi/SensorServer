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
///Analogue magnet sensor. No any external libraries are neccesary. Details to member functions see ISensor class documentation
class MagnetSensor : public ISensor
{
	int _port;
public:
	MagnetSensor(int port)
	{
		_port=port;
	}
	const __FlashStringHelper* Name()
	{
		return F("Magnet");
	}
	float LowMeasurementLimit()
	{
		return -1;
	}
	float HighMeasurementLimit()
	{
		return 2;
	}
	int Precission()
	{
		return 0;
	}
	bool Measure(float &data)
	{
		data=(int)(analogRead(_port)>500?1:0);
		return true;
	}
};