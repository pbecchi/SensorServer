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
///User should derive his class from ICriticalProcess if such class has time critical functions, like check whether user touch the display 
class ICriticalProcess
{
public:
	///In derived class contains time critical source code 
	void virtual Idle()=0;
};