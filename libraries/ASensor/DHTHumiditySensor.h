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
#include "DHTTemperatureSensor.h"
///Wrapper for DHT temperature and humidity sensor. Humidity is just read from DHTTemperatureSensor. DHTTemperatureSensor has to be referenced in the first constructor parameter. Details to member functions see ISensor class documentation
class DHTHumiditySensor : public ISensor
{

	DHTTemperatureSensor *_srcSensor;
public:
	/**
	\param src_sensor reference to existent DHTTemperatureSensor
	*/
	DHTHumiditySensor(DHTTemperatureSensor *src_sensor)
	{
		_srcSensor = src_sensor;
	}

public:
	virtual const __FlashStringHelper* Name()
	{
		return F("DHT Humidity");
	}
	virtual float LowMeasurementLimit()
	{
		return 1;
	}
	virtual float HighMeasurementLimit()
	{
		return 100;
	}
	virtual int Precission()
	{
		return 0;
	}
	virtual bool Measure(float &data)
	{
		data=_srcSensor->LastHumidity();
		return _srcSensor->IsOK();
	}
};
