#pragma once
/*
  AFrame - Arduino framework library for ASensor and AWind libraries
  Copyright (C)2015 Andrei Degtiarev. All right reserved
  
  You can always find the latest version of the library at 
  https://github.com/AndreiDegtiarev/AFrame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the MIT license.
  Please see the included documents for further information.

#if defined __arm__ //DUE
#include <Arduino.h>
#else
#include "HardwareSerial.h"
#endif
*/
#include <Arduino.h>
class Endln {};
///Wrapper about Arduino HardwareSerial class
class Log
{
	bool _is_initialized;
public:
	Log()
	{
		_is_initialized=false;
	}
	///Initializes serial interface
	/**
	\param baud symbol rate in bauds
	*/
	void begin(unsigned long baud)
	{
		Serial.begin(baud);
		_is_initialized=true;
	}
	///Overload << operator with end of line command
	friend Log& operator<<(Log &out,Endln &value)
	{
		if(out.IsInitialized())
			Serial.println();
		return out;
	}
	///Overload << operator for general values
	template<class T>
	friend Log& operator<<(Log &out,T value)
	{
		if(out.IsInitialized())
			Serial.print(value);
		return out;
	}
private:
	bool IsInitialized()
	{
		return _is_initialized;
	}

};
extern Log out;
extern Endln endln;
