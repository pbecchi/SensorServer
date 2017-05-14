#pragma once
/*
  AFrame - Arduino framework library for ASensor and AWind libraries
  Copyright (C)2015 Andrei Degtiarev. All right reserved
  

  You can always find the latest version of the library at 
  https://github.com/AndreiDegtiarev/AFrame


  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the 
  examples and tools supplied with the library.
*/
class SensorManager;
///Receiver for events that occurs after each measurement. Event will always generated event if measurements produce same results
class ISensorMeasuredEventReceiver
{
public:
	///In order to get event the function has to be overriden in the derived class
	virtual void NotifySensorMeasured(SensorManager *sensorManager)=0;
};