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

///Base class for Transceiver objects (like NRF24Transceiver)
class Transceiver
{

public:
	Transceiver()
	{
	}
	virtual void setup()=0;
	virtual void send_data(const __FlashStringHelper *nodeID,const __FlashStringHelper *name,float value) = 0;
};