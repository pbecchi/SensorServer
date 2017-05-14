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
///Interface class that define the way how application interats with data buffer 
class IDataBuffer
{
public:
	///Return start index in the buffer
	virtual unsigned int StartIndex()=0;
	///Returns buffer size
	virtual unsigned int Size()=0;
	///Return min max value 
	virtual void MinMax(float &mix_x,float &max_x,float &mix_y,float &max_y)=0;
	///Returns x value corresponding to the specified index in buffer
	virtual float X(unsigned int index)=0;
	///Returns y value corresponding to the specified index in buffer
	virtual float Y(unsigned int index)=0;
};
