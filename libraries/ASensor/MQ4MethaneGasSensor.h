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
///MQ4 methane gas sensor from sainsmart. No any external libraries are neccesary. Details to member functions see ISensor class documentation
class MQ4MethaneGasSensor : public ISensor
{
	int _port;

public:
	MQ4MethaneGasSensor(int port)
	{
		_port=port;
	}
	const __FlashStringHelper* Name()
	{
		return F("MQ4MethaneGas");
	}
	float LowMeasurementLimit()
	{
		return 0;
	}
	float HighMeasurementLimit()
	{
		return 1024;
	}
	int Precission()
	{
		return 0;
	}
	bool Measure(float &data)
	{
		data=(int)analogRead(_port);
		return true;
	}
};