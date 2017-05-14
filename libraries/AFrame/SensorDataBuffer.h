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

#include <math.h>
#include "Log.h"
#include "IDataBuffer.h"

///Implement sensor data buffer class. This buffer is used to collect data in form of queue. In order to save SRAM data is saved as integers and converted back in floata according scling factors
class SensorDataBuffer : public IDataBuffer
{
	int _size;
	unsigned int  *_data_x;
	int   *_data_y;
	int _startIndex;
	float _factor_x;
	float _factor_y;
	unsigned long _offset_x;
public:
	///Constructor
	/**
	\param factor_x scale factor for x values
	\param factor_y scale factor for y values
	\param size max  buffer size
	*/
	SensorDataBuffer(float factor_x,float factor_y,int size)
	{
		_offset_x = 0;
		_factor_x=factor_x;
		_factor_y=factor_y;
		_size=size;

		_data_x=new unsigned int[_size];
		_data_y=new int[_size];
		for(int i=0;i<_size;i++)
		{
			_data_x[i]=0;
			_data_y[i]=0;
		}
		_startIndex = _size;

	}
	///Returns pointer to internal buffer with x values 
	unsigned int *X()
	{
		return _data_x;
	}
	///Returns pointer to internal buffer with y values 
	int *Y()
	{
		return _data_y;
	}
	///Adds to the end of buffer x and y values
	void AddValue(float value_x,float value_y)
	{
		int offset_0=_data_x[0];
		_offset_x+=offset_0;
		unsigned int buf_value_x=value_x*_factor_x;
		int buf_value_y=value_y*_factor_y;
		if(_startIndex!=0)
			_startIndex--;
		for(int i=0;i<_size-1;i++)
		{
			_data_x[i]=_data_x[i+1]-offset_0;
			_data_y[i]=_data_y[i+1];
		}
		_data_x[_size-1]=buf_value_x-_offset_x;
		_data_y[_size-1]=buf_value_y;
	}
	///Implements function from base class
	void MinMax(float &min_x,float &max_x,float &min_y,float &max_y)
	{
 		min_x=_data_x[_size-1];
		max_x=_data_x[_size-1];
 		min_y=_data_y[_size-1];
		max_y=_data_y[_size-1];
		for(int i=_startIndex;i<_size-1;i++)
		{
			min_x=min(min_x,_data_x[i]);
			max_x=max(max_x,_data_x[i]);
			min_y=min(min_y,_data_y[i]);
			max_y=max(max_y,_data_y[i]);
		}
		min_x+=_offset_x;
		max_x+=_offset_x;
		min_x/=_factor_x;
		max_x/=_factor_x;
		min_y/=_factor_y;
		max_y/=_factor_y;
	}
	///Implements function from base class
	float X(unsigned int index)
	{
		if(index>=Size())
		{
			out<<F("Error: index outside of array bounds: ")<<index<<endln;
			return 0;
		}
		return (_data_x[index]+_offset_x)/_factor_x;
	}
	///Implements function from base class
	float Y(unsigned int index)
	{
		if(index>=Size())
		{
			out<<F("Error: index outside of array bounds: ")<<index<<endln;
			return 0;
		}
		return _data_y[index]/_factor_y;
	}
	///Implements function from base class
	unsigned int StartIndex()
	{
		return _startIndex;
	}
	///Implements function from base class
	unsigned int Size()
	{
		return _size;
	}
};
