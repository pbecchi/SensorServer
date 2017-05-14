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
///Wrapper for DHT temperature and humidity sensor. To use this sensor https://github.com/markruys/arduino-DHT library has to be installed. Details to member functions see ISensor class documentation
class DHTTemperatureSensor : public ISensor
{
public:
	enum SensorModel
	{
		AUTO_DETECT,
		DHT11,
		DHT22
	};
private:
#ifndef DEMO_SENSORS
	DHT *_dht;
#endif
	//float _last_temperature;
	float _last_humidity;
	bool _isOK;
	int _pin;
public:
	DHTTemperatureSensor(int port,SensorModel sensorModel=AUTO_DETECT)
	{
		_pin=port;
#ifndef DEMO_SENSORS
		_dht=new DHT();
		DHT::DHT_MODEL_t dhtModel=DHT::AUTO_DETECT;
		if(sensorModel == DHT11)
			dhtModel=DHT::DHT11;
		else
			dhtModel=DHT::DHT22;
		_dht->setup(port,dhtModel);
#endif
		_last_humidity=0;
		_isOK=false;
	}

public:
	virtual const __FlashStringHelper* Name()
	{
		return F("DHT Temperature");
	}
	virtual float LowMeasurementLimit()
	{
		return -50;
	}
	virtual float HighMeasurementLimit()
	{
		return 50;
	}
	virtual int Precission()
	{
		return 1;
	}
	float LastHumidity()
	{
		return _last_humidity;
	}
	bool IsOK()
	{
		return _isOK;
	}
	virtual bool Measure(float &data)
	{
		_isOK=false;
#ifdef DEMO_SENSORS
		data =(float)rand()/RAND_MAX*5+20;
		_last_humidity = (float)rand()/RAND_MAX*10+60;
		_isOK=true;
#else
		DHT::DHT_ERROR_t status= DHT::ERROR_NONE;
		if(_dht!=NULL)
		{
			status=_dht->getStatus();
			if(status == DHT::ERROR_NONE || status ==DHT::ERROR_CHECKSUM) //test for check sum does not work stably
			{
				data = _dht->getTemperature();
				_last_humidity = _dht->getHumidity();
			}
			else
			{
				out<<F("DHT Status: ")<<_dht->getStatusString()<<endln;
				_dht->setup(_pin,_dht->getModel());
				//delay(_dht->getMinimumSamplingPeriod()*2); 
			}
		}
#ifdef DEBUG_AWIND
		out<<F("Status: ")<<_dht->getStatusString()<<F(" Tempr: ")<<data<<F(" Humidity: ")<<_last_humidity<<endln;
#endif
		_isOK=status == DHT::ERROR_NONE && !isnan(data) && !isnan(_last_humidity);
		//delay(_dht->getMinimumSamplingPeriod()); 
#endif
		return _isOK;
	}
};
