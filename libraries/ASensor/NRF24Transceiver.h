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
#include "Transceiver.h"
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0x7365727631LL };   

int serial_putc( char c, FILE * )
{ 
	Serial.write( c );
	return c; 
}
///Warpper about RF24 library
class NRF24Transceiver : public Transceiver
{
	RF24 *_radio;

	char receivePayload[32]; 
public:
	NRF24Transceiver(int cepin, int cspin)
	{
		_radio = new RF24(cepin,cspin);
	}
	virtual void setup()
	{
		fdevopen( &serial_putc, 0 );
		printf("Sending nodeID & 1 sensor data\n\r");   
		_radio->begin();   
		// Enable this seems to work better 
		_radio->enableDynamicPayloads();   
		_radio->setDataRate(RF24_1MBPS); 
		_radio->setPALevel(RF24_PA_MAX); 
		_radio->setChannel(76); 
		_radio->setRetries(15,15);   
		_radio->openWritingPipe(pipes[0]); 
		_radio->openReadingPipe(1,pipes[1]);   
		// Dump the configuration of the rf unit for debugging 
		_radio->printDetails(); 
	}
	void send_data(const __FlashStringHelper *nodeID,const __FlashStringHelper *name,float value)
	{
		bool timeout=0;
		unsigned long send_time, rtt = 0;
		char outBuffer[40]=""; 
		char floatBuf[6];
		dtostrf(value,5,2,floatBuf);
		sprintf(outBuffer,"%s %s %s",nodeID,name,floatBuf);
		//Serial.println(outBuffer);
		send_time = millis();
		// Stop listening and write to radio 
		_radio->stopListening();  
		// Send to hub 
		for(int i=5;i>=0;i--)
		{
			if ( _radio->write( outBuffer, strlen(outBuffer)) )
			{ 
				//printf("Send successful\n\r"); 
				break;
			}
			else 
			{
				printf("Send failed. Attempt:%d\n\r",i); 
				delay(200);
			}
		}
		_radio->startListening();
		delay(20);
		while ( _radio->available() && !timeout )
		{   
			uint8_t len = _radio->getDynamicPayloadSize();
			_radio->read( receivePayload, len);
			receivePayload[len] = 0;
			//printf("inBuffer:  %s\n\r",receivePayload); 
			// Compare receive payload with outBuffer 
			if ( ! strcmp(outBuffer, receivePayload) ) 
			{ 
				rtt = millis() - send_time;
				printf("inBuffer --> rtt: %i \n\r",rtt);
			} 
			// Check for timeout and exit the while loop 
			if ( millis() - send_time > _radio->getMaxTimeout() ) 
			{ 
				Serial.println("Timeout!!!");
				timeout = 1;
			}
			delay(10);
		} 
	}
};