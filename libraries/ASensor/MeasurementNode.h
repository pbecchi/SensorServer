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
#include "LinkedList.h"
#include "ICriticalProcess.h"
#include "Transceiver.h"
///Measurement node controls measurement process between different sensors
class MeasurementNode
{
	LinkedList<SensorManager> &_sensors;
	ICriticalProcess *_criticalProcess;

	Transceiver *_radio;
	const __FlashStringHelper *_nodeID;
public:
	///Constructor
	/**
	\param List of sensors
	\param tranceiver is not null if data need to be send over radio like RF24 sensor or Wifi 
	*/
	MeasurementNode(LinkedList<SensorManager> &sensors,Transceiver *tranceiver):_sensors(sensors)
	{
		//_radio= new NRF24Transceiver(8,9);
		_radio= tranceiver;
		_criticalProcess = NULL;
	}
	///Sets node id. The id is sended to the receiver
	void SetID(const __FlashStringHelper *nodeID)
	{
		_nodeID=nodeID;
	}
	///Setups radio module 
	void Initialize()
	{
		if(_radio!=NULL)
			_radio->setup();
	}
	///Sets pointer to the critical process like touch management
	void SetCriticalProcess(ICriticalProcess *criticalProcess)
	{
		_criticalProcess=criticalProcess;
	}
	///Perfroms measurements over all sensors
	bool Measure()
	{
		bool retCode=false;
		for(int i=0;i<_sensors.Count();i++)
		{
			if(_criticalProcess!=NULL)
				_criticalProcess->Idle();
			if(_sensors[i]->IsReadyForMeasurement())
			{
				_sensors[i]->Measure();
				retCode = true;
			}
		}
		return retCode;
	}
	///Return true if during the last measurement step at least one sensor contains data that differ from previos measurement step
	bool IsChanged()
	{
	  for(int i=0;i<_sensors.Count();i++)
		if(_sensors[i]->IsChanged())
		{
			return true;
		}
	  return false;
	}
	///Sends data over radio module
	void SendData()
	{
	  for(int i=0;i<_sensors.Count();i++)
	  {
		  SensorManager *sensorManager=_sensors[i];
		  if(sensorManager->Status()!=Error)
		  { 
			  if(_radio!=NULL)
		         _radio->send_data(_nodeID,sensorManager->Sensor()->Name(),sensorManager->GetData());
		  }
	  }
	}
	///Logs last measurement results
	void LogResults()
	{
	  for(int i=0;i<_sensors.Count();i++)
	  {
		  SensorManager *sensorManager=_sensors[i];
		  out<<sensorManager->Sensor()->Name()<<F(": ");
		  if(sensorManager->Status()!=Error)
			  out<<sensorManager->GetData();
		  else
			  out<<F("error");
		  out<<F("; ");
	  }
	  out<<endln;
	}
	///Very usefull excel logging tool https://www.parallax.com/downloads/plx-daq
	void SetupPLXLog()
	{
		out<<F("CLEARDATA")<<endln;
		out<<F("LABEL,Time,");
		for(int i=0;i<_sensors.Count();i++)
		{
		  SensorManager *sensorManager=_sensors[i];
		  out<<sensorManager->Sensor()->Name();
		  if(i<_sensors.Count()-1)
			out<<F(",");
		}
		out<<endln;
	}
	void LogResultsPLX()
	{
	  out<<"DATA,TIME,";
	  for(int i=0;i<_sensors.Count();i++)
	  {
		  SensorManager *sensorManager=_sensors[i];
		  if(sensorManager->Status()!=Error)
			  out<<sensorManager->GetData();
		  else
			  out<<F("error");
		  if(i<_sensors.Count()-1)
			out<<",";
	  }
	  out<<endln;
	}
	
};