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
///Base class for sensor wrappers. Via ISensor external sensor source code can be plugged into ASensor and AWind libraries 
class ISensor
{
public:
	///Returns internal sensors name. Usefull for debugging
	virtual const __FlashStringHelper* Name()=0;
	///Return how many decimal places make sence for the sensor
	virtual int Precission()=0;
	///Return lowest possible measurement limit. If value outside of this limit, measurements treated as erroneous
	virtual float LowMeasurementLimit()=0;
	///Return highest possible measurement limit. If value outside of this limit, measurements treated as erroneous
	virtual float HighMeasurementLimit()=0;
	///Performs measurements 
	/**
	\param data reference to the variable that becoms result measurement
	\return true if success
	*/
	virtual bool Measure(float &data)=0;
};

