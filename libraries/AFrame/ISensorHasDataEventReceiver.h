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
class SensorManager;
///Interface for sensor has data event receiver. If application need to get data from sensor the receiver class has to implement this interface
class ISensorHasDataEventReceiver
{
public:
	///Event function that has to be overriden in derived class
	/**
	\param sensorManager pointer to corresponding sensor data conatiner
	*/
	virtual void NotifySensorHasData(SensorManager *sensorManager)=0;
};