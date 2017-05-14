#pragma once
/*
  AFrame - Arduino framework library for ASensor and AWind libraries
  Copyright (C)2015 Andrei Degtiarev. All right reserved
  
  You can always find the latest version of the library at 
  https://github.com/AndreiDegtiarev/AFrame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the MIT license.
  Please see the included documents for further information.
*/
#include "ISensor.h"
#include "MeasurementStatus.h"
#include "SensorDataBuffer.h"
#include "ISensorHasDataEventReceiver.h"
#include "ISensorMeasuredEventReceiver.h"

///Manage operations with sensors: triggers measurements, check appplication alarm status and etc.
class SensorManager
{

protected: 
	ISensor *_sensor;
private:
	float _prev_value;                            //!< Prev measured value
	float _last_value;                           //!< Last measured value
	MeasurementStatus _status;                  //!<  Last measuremnt status
	float _low_application_limit;              //!< Defines application limit for measurements. Like inner temperature schould not be below 0 Celcius 
	float _high_application_limit;            //!< Defines application limit for measurements. Like humidity schould not be higher as 65 %
	unsigned long _time_last_measurement;    //!< Contains time as last succefull measurements were done
	unsigned long _pause_length;            //!< Pause between measurements [milliseconds]
	SensorDataBuffer *_secBuffer;          //!< Contains all last data
	SensorDataBuffer *_minBuffer;         //!< Contains last data with minute interval
	SensorDataBuffer *_howrsBuffer;      //!< Contains last data with hower interval
	ISensorHasDataEventReceiver *_eventReceiver;           //!< Receiver for results of meausurements that differs from previos one
	ISensorMeasuredEventReceiver *_eventMeasuredReceiver; //!< Receiver for results of last meausurements
public:
	///Constructor
	/**
	\param sensor derived from ISensor class
	\param low_application_limit defines application limit for measurements. Like inner temperature schould not be below 0 Celcius 
	\param high_application_limit defines application limit for measurements. Like humidity schould not be higher as 65 %
	\param pause_length pause between measurements [milliseconds]
	*/
	SensorManager(ISensor *sensor,
				  float low_application_limit,
				  float high_application_limit,
				  unsigned long pause_length)
	{
		_sensor=sensor;
		_low_application_limit=low_application_limit;
		_high_application_limit=high_application_limit;
		_pause_length = pause_length;
		_status=OK;
		_prev_value = 0;
		_last_value = 0;
		_time_last_measurement = 0.0;
		//reduce this value if you have problem with SRAM
#ifdef DEBUG_AWIND
		const int buf_size=20;
#else
		const int buf_size=24;
#endif
		_secBuffer=new SensorDataBuffer(1,pow(10,sensor->Precission()),buf_size);
		_minBuffer=new SensorDataBuffer(1/60.0,pow(10,sensor->Precission()),buf_size);
		_howrsBuffer=new SensorDataBuffer(1/(60.0*60.0),pow(10,sensor->Precission()),buf_size);
		_eventReceiver=NULL;
		_eventMeasuredReceiver=NULL;
	}
	///Registers receiver for sensor measurement if it differs from previos one
	void RegisterHasDataEventReceiver(ISensorHasDataEventReceiver *eventReceiver)
	{
		_eventReceiver=eventReceiver;
	}
	///Registers receiver for sensor measurement for every measurement event
	void RegisterMeasuredEventReceiver(ISensorMeasuredEventReceiver *eventReceiver)
	{
		_eventMeasuredReceiver=eventReceiver;
	}
	///Returns associated sensor
	ISensor *Sensor()
	{
		return _sensor;
	}
	///Sets pause between measurements
	/**
	\param pause_ms pause between measurements [milliseconds]
	*/
	void SetPause(unsigned long pause_ms)
	{
		_pause_length=pause_ms;
	}
	///Returns pause between measurements [milliseconds]
	unsigned long GetPause()
	{
		return _pause_length;
	}
	///Return internal buffer for all last measurements (check buf_size parameter)
	SensorDataBuffer *SecBuffer()
	{
		return _secBuffer;
	}
	///Return internal buffer for last measurements with minutes interval (check buf_size parameter)
	SensorDataBuffer *MinBuffer()
	{
		return _minBuffer;
	}
	///Return internal buffer for last measurements with howrs interval (check buf_size parameter)
	SensorDataBuffer *HowrsBuffer()
	{
		return _howrsBuffer;
	}
	///Returns true if measurements need to be performed
	bool IsReadyForMeasurement()
	{
		return millis()-_time_last_measurement>_pause_length;
	}
	///Returns true if there are new measurements
	bool IsChanged()
	{
		if(_status != Error && isDiffer(_prev_value,_last_value))
			return true;
		return false;
	}
	///Returns last measurements
	float GetData()
	{
		if(isDiffer(_prev_value,_last_value))
		{
			_prev_value=_last_value;
		}
		return _last_value;
	}
	///Triggers sensor measurements
	void Measure()
	{
		float value=0;
		_status=Error;
		if(_sensor->Measure(value))
		{
			_status=OK;
			if(value<_sensor->LowMeasurementLimit() || value >_sensor->HighMeasurementLimit())
			{
				_status=Error;
				return;
			}
			if(value<_low_application_limit || value >_high_application_limit)
			{
				_status=ApplicationAlarm;
			}
			if(_sensor->Precission() == 0)
				value=lrint(value);
			else
			{
				float factor=pow(10,_sensor->Precission());
				value=lrint(value*factor)/factor;
			}
			_time_last_measurement = millis();
			_last_value=value;
			unsigned long cursec=_time_last_measurement/1000;
			_secBuffer->AddValue(cursec,value);
			unsigned long last_sec_time = _minBuffer->X(_minBuffer->Size()-1);
			if(last_sec_time == 0 || (cursec-last_sec_time)>3600/_minBuffer->Size())
				_minBuffer->AddValue(cursec,value);
			unsigned long last_howr_time = _howrsBuffer->X(_howrsBuffer->Size()-1);
			if(last_howr_time == 0 || (cursec-last_howr_time)>3600)
				_howrsBuffer->AddValue(cursec,value);
			if(IsChanged() && _eventReceiver!=NULL)
				_eventReceiver->NotifySensorHasData(this);
			if(_eventMeasuredReceiver!=NULL)
				_eventMeasuredReceiver->NotifySensorMeasured(this);

		}
	}
	///Return status of last measurements
	MeasurementStatus Status()
	{
		return _status;
	}
private:
	///Return true if two values are differ
	bool isDiffer(float prev_value,float value)
	{
		return prev_value!=value;
	}

};
