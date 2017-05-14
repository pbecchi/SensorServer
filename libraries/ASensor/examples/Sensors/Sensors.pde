/*
  AFrame - Arduino framework library for ASensor and AWind libraries
  Copyright (C)2014 Andrei Degtiarev. All right reserved
  
  You can always find the latest version of the library at 
  https://github.com/AndreiDegtiarev/ASensor

  This library is free software; you can redistribute it and/or
  modify it under the terms of the MIT license.
  Please see the included documents for further information.
*/
///Sensor example demonstrates basic measuremnt node that measures and sends via radio temerature and humidity results
///Installation: AFrame + https://github.com/markruys/arduino-DHT libraries have to be installed
///Wiring: DHT sensor should be connected to the pin 10
///Radio sender: see rf24 library
#define DEBUG_AWIND //!<remove comments if name of window is need to known during runtime. Be carrefull about SRAM

#define WITH_RF24_TRANSCEIVER //comment if you are not have RF24 transceiver

#include <DHT.h>

#include "AHelper.h"
#include "ISensor.h"

#ifdef WITH_RF24_TRANSCEIVER
#include <SPI.h>
#include <RF24.h>
#include "NRF24Transceiver.h"
#endif

#include "DHTHumiditySensor.h"
//#include "DustSensor.h"
//#include "MQ4MethaneGasSensor.h"

#include "SensorManager.h"
#include "MeasurementNode.h"

#include "LinkedList.h"

//pin on Arduino where temperature sensor is connected (in demo is meaningless)
int temperature_port=10;

//list where all sensors are collected
LinkedList<SensorManager> sensors;
//manager which controls the measurement process
#ifdef WITH_RF24_TRANSCEIVER
MeasurementNode measurementNode(sensors,new NRF24Transceiver(8,9));
#else
MeasurementNode measurementNode(sensors,NULL);
#endif

void setup()
{
	//setup log (out is wrap about Serial class)
	out.begin(57600);
	out<<F("Setup")<<endln;

	//sensors
	DHTTemperatureSensor *inTempr=new DHTTemperatureSensor(temperature_port-2);
	DHTHumiditySensor *inHumidity=new DHTHumiditySensor(inTempr);
	//sensor managers. Manager defines measurement limits and measurement delays
	sensors.Add(new SensorManager(inTempr,15,40,1000*10)); //0
	sensors.Add(new SensorManager(inHumidity,0,80,1000*10)); //1

	//sensors.Add(new SensorManager(new DustSensor(8,1000*10),0,10000,1000*5));
	//sensors.Add(new SensorManager(new MQ4MethaneGasSensor(A0),0,10000,1000*5));

	delay(1000); 
	measurementNode.SetupPLXLog();
	out<<F("End setup")<<endln;
}
void loop()
{
	//measure (if necessary -see delay parameter in sensor manager)
	if(measurementNode.Measure())
	{
		if(measurementNode.IsChanged())
		{
			measurementNode.SendData(); 
		//following if is only for debugging purposes
			measurementNode.LogResultsPLX();
		}

	}
}