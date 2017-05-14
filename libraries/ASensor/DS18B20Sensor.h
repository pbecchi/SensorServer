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
///DS18B20-Temperature sensor. To use this sensor OneWire library has to be installed from http://www.pjrc.com/teensy/td_libs_OneWire.html. Details to member functions see ISensor class documentation
class DS18B20Sensor : public ISensor
{
#ifndef DEMO_SENSORS
	OneWire  *ds;
#endif
	int _wire_index;
public:
	DS18B20Sensor(int port,int wire_index)

	{
#ifndef DEMO_SENSORS
		ds=new OneWire(port);
#endif
		_wire_index=wire_index;
	}
	virtual const __FlashStringHelper* Name()
	{
		return F("DS18B20");
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
	virtual bool Measure(float & ret_data)
	{

#ifdef DEMO_SENSORS
		ret_data=(float)rand()/RAND_MAX*5+15;
		return true;
#else
		byte i;
		byte present = 0;
		byte type_s=0;
		byte data[12];
		byte addr[8];
		//byte addr[] = {0x28,0xB2,0x6F,0xA4,0x5,0x0,0x0,0x9B};
		float celsius, fahrenheit;

		bool isError=true;
		for(int i=0;i<_wire_index;i++)
		{
			for(int j=0;j<3;j++)
			{
				isError=!ds->search(addr);
				if(isError)
				{
					Serial.print(F("No more addresses:"));
					Serial.println(j);
					delay(250);
				}
				else
				{
					/*Serial.print("found ");
					Serial.print(i);
					Serial.println(_wire_index);*/
					break;
				}
			}
			if(isError)
				break;
		}
		if(isError)
		{
			ds->reset_search();
			return false;
		}
		/*Serial.print("ROM =");
		for( i = 0; i < 8; i++) {
		Serial.write(' ');
		Serial.print(addr[i], HEX);
		}
		Serial.println("");*/

		if (OneWire::crc8(addr, 7) != addr[7]) {
			Serial.println(F("CRC is not valid!"));
			return false;
		}
		//Serial.println();

		// the first ROM byte indicates which chip
		switch (addr[0]) {
		case 0x10:
			Serial.println(F("  Chip = DS18S20"));  // or old DS1820
			type_s = 1;
			break;
		case 0x28:
			//Serial.println("  Chip = DS18B20");
			type_s = 0;
			break;
		case 0x22:
			Serial.println(F("  Chip = DS1822"));
			type_s = 0;
			break;
		default:
			Serial.println(F("Device is not a DS18x20 family device."));
			return false;
		} 

		ds->reset();
		ds->select(addr);
		ds->write(0x44, 1);        // start conversion, with parasite power on at the end

		//delay(1000);     // maybe 750ms is enough, maybe not
		// we might do a ds.depower() here, but the reset will take care of it.

		present = ds->reset();
		ds->select(addr);    
		ds->write(0xBE);         // Read Scratchpad

		/*Serial.print("  Data = ");
		Serial.print(present, HEX);
		Serial.print(" ");*/
		for ( i = 0; i < 9; i++) {           // we need 9 bytes
			data[i] = ds->read();
			//Serial.print(data[i], HEX);
			//Serial.print(" ");
		}
		/*Serial.print(" CRC=");
		Serial.print(OneWire::crc8(data, 8), HEX);
		Serial.println();*/

		// Convert the data to actual temperature
		// because the result is a 16 bit signed integer, it should
		// be stored to an "int16_t" type, which is always 16 bits
		// even when compiled on a 32 bit processor.
		int16_t raw = (data[1] << 8) | data[0];
		if (type_s) {
			raw = raw << 3; // 9 bit resolution default
			if (data[7] == 0x10) {
				// "count remain" gives full 12 bit resolution
				raw = (raw & 0xFFF0) + 12 - data[6];
			}
		} else {
			byte cfg = (data[4] & 0x60);
			// at lower res, the low bits are undefined, so let's zero them
			if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
			else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
			else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
			//// default is 12 bit resolution, 750 ms conversion time
		}
		celsius = (float)raw / 16.0;
		fahrenheit = celsius * 1.8 + 32.0;
		/*Serial.print("  Temperature = ");
		Serial.print(celsius);
		Serial.print(" Celsius, ");
		Serial.print(fahrenheit);
		Serial.println(" Fahrenheit");*/
		ds->reset_search();
		if(!isnan(celsius))
		{
			ret_data=celsius;
#ifdef DEBUG_AWIND
			out<<F("DS18B20: ")<<ret_data<<endln;
#endif
			return true;
		}
#endif
		return false;
	}
};