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
#include "Log.h"
///Implements number of different helper functions
class AHelper
{
public:
	///Logs free SRAM memory. SRAM memory is a critical issue especially for application that perform data logging
	static void LogFreeRam ()
	{
#ifndef _VARIANT_ARDUINO_DUE_X_
		extern int __heap_start, *__brkval; 
		int v; 
		int fr = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
		out<<F("Free ram: ");
		out<<fr<<endln;
#endif


	}
	///Calculates number of charachters in a number.
	/**
	\param number target number
	\param prec number of decimal points
	\return number of charachters that represents input value
	*/
	static int GetNumberLength(float number,int prec)
	{
		return (abs(number)<1?0:log10(abs(number)))+2+prec+(number<0?1:0);
	}
	///Compares PROGMEM strings. I didn't manged to use strcmp_P function
	static bool compare_F(const __FlashStringHelper *str1,const __FlashStringHelper *str2)
	{
		size_t len1 = strlen_P((const char PROGMEM *)str1);
		size_t len2 = strlen_P((const char PROGMEM *)str2);
		if(len1!=len2)
			return false;
		unsigned char c1,c2;
		for (size_t i=0; i<len1; i++)
		{
			c1 = pgm_read_byte(&((const char PROGMEM *)str1)[i]);
			c2 = pgm_read_byte(&((const char PROGMEM *)str2)[i]);
			if(c1!=c2)
				return false;
		}
		return true;
	}
};