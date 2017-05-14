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
#include "ISensor.h"
#include "TimeSerieBuffer.h"

///Voltmeter sensor. No any external libraries are neccesary. Allows measurements not only single value but time serie as well. Details to member functions see ISensor class documentation
class VoltmeterSensor : ISensor
{
	int _port;
	TimeSerieBuffer *_dataBuffer;
	int _time_step_mus;
public:
	///Constructor
	/**
	\param port analogue pin where signal should measured
	\param reserved_buffer_size max possible (reserved) buffer size
	\param actual_size initial buffer size
	*/
	VoltmeterSensor(int port,int reserved_buffer_size,int actual_size)
	{
		_port=port;
		_dataBuffer=new TimeSerieBuffer(1,1023/5.0,reserved_buffer_size,actual_size);
		pinMode(port,INPUT);
		_time_step_mus=100;
	}
	///return buffer contains measured data
	TimeSerieBuffer *Buffer()
	{
		return _dataBuffer;
	}
	///Returns internal sensors name. Usefull for debugging
	virtual const __FlashStringHelper* Name()
	{
		return F("Voltmeter");
	}
	///Return lowest possible measurement limit. If value outside of this limit, measurements treated as erroneous
	virtual float LowMeasurementLimit()
	{
		return -50;
	}
	///Return highest possible measurement limit. If value outside of this limit, measurements treated as erroneous
	virtual float HighMeasurementLimit()
	{
		return 50;
	}
	///Return how many decimal places make sence for the sensor
	virtual int Precission()
	{
		return 1;
	}
	///Sets sample time between measurements [microseconds]
	void SetTimeStep(int time_step_us)
	{
		_time_step_mus=time_step_us;
	}
	///Returns time step [microseconds]
	float TimeStep()
	{
		return _time_step_mus;
	}
	///Returns time serie length [seconds]
	float TimeLength()
	{
		return (_time_step_mus*_dataBuffer->Size())/1e6;
	}
	///Performs measurements 
	/**
	\param data reference to the variable that becoms result measurement
	\return true if success
	*/
	virtual bool Measure(float &data)
	{
		data=analogRead(_port);
		return true;
	}
	///Performs measurements of time serie
	void MeasureBuffer()
	{
		int size=_dataBuffer->Size();
		_dataBuffer->SetTimeStep(_time_step_mus/1.0e6);
		int * data_y=_dataBuffer->Y();
		for(int i=0;i<size;i++)
		{
			data_y[i]=analogRead(_port);
			delayMicroseconds(_time_step_mus);
		}
	}

};