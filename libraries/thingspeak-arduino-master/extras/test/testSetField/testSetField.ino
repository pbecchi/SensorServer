#line 2 "testSetField.ino"
/*
  testSetField unit test
  
  Unit Test for the writeFields and setField functions in the ThingSpeak Communication Library for Arduino

  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize and 
  analyze live data streams in the cloud.
  
  This test use the ArduinoUnit 2.1.0 unit test framework.  Visit https://github.com/mmurdoch/arduinounit to learn more.
  
  Copyright 2016, The MathWorks, Inc.
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the extras/documentation folder where the library was installed.
  See the accompaning licence.txt file for licensing information.
*/

//#define USE_WIFI101_SHIELD
//#define USE_ETHERNET_SHIELD

#if !defined(USE_WIFI101_SHIELD) && !defined(USE_ETHERNET_SHIELD) && !defined(ARDUINO_SAMD_MKR1000) && !defined(ARDUINO_AVR_YUN) && !defined(ARDUINO_ARCH_ESP8266)
  #error "Uncomment the #define for either USE_WIFI101_SHIELD or USE_ETHERNET_SHIELD"
#endif

#include <ArduinoUnit.h>
#include <ThingSpeak.h>

#if defined(ARDUINO_AVR_YUN)
    #include "YunClient.h"
    YunClient client;
#else
  #if defined(USE_WIFI101_SHIELD) || defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_ARCH_ESP8266)
    // Use WiFi
    #ifdef ARDUINO_ARCH_ESP8266
      #include <ESP8266WiFi.h>
    #else
      #include <SPI.h>
      #include <WiFi101.h>
    #endif
    char ssid[] = "<YOURNETWORK>";    //  your network SSID (name) 
    char pass[] = "<YOURPASSWORD>";   // your network password   
    int status = WL_IDLE_STATUS;
    WiFiClient  client;
  #elif defined(USE_ETHERNET_SHIELD)
    // Use wired ethernet shield
    #include <SPI.h>
    #include <Ethernet.h>
    byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    EthernetClient client;
  #endif
#endif

unsigned long testChannelNumber = 31461;
const char * testChannelWriteAPIKey = "LD79EOAAWRVYF04Y";

#define WRITE_DELAY_FOR_THINGSPEAK 15000

test(setFieldCase) 
{
  // Always wait to ensure that rate limit isn't hit
  delay(WRITE_DELAY_FOR_THINGSPEAK);

  // No fields set
  assertEqual(ERR_SETFIELD_NOT_CALLED, ThingSpeak.writeFields(testChannelNumber, testChannelWriteAPIKey));

  // Test basic multi-field write
  assertEqual(OK_SUCCESS,ThingSpeak.setField(1,(float)3.14159));  // float
  assertEqual(OK_SUCCESS,ThingSpeak.setField(2,-47)); // integer
  assertEqual(OK_SUCCESS,ThingSpeak.setField(3,(long)100000L)); // long
  assertEqual(OK_SUCCESS,ThingSpeak.setField(4,(float)NAN)); // Nan
  assertEqual(OK_SUCCESS,ThingSpeak.setField(5,"foobar")); // string
  assertEqual(OK_SUCCESS,ThingSpeak.setField(6,(float)INFINITY)); // + inf
  assertEqual(OK_SUCCESS,ThingSpeak.setField(7,(float)-INFINITY)); // + inf
  assertEqual(OK_SUCCESS,ThingSpeak.writeFields(testChannelNumber, testChannelWriteAPIKey));

  // Test write when not enough time has elapsed
  assertEqual(OK_SUCCESS,ThingSpeak.setField(1,(float)3.14159));  // float
  assertEqual(ERR_NOT_INSERTED,ThingSpeak.writeFields(testChannelNumber, testChannelWriteAPIKey));

  // Allow enough time to pass to make sure that it would work 
  delay(WRITE_DELAY_FOR_THINGSPEAK);

  // Test write to field out of range
  assertEqual(ERR_INVALID_FIELD_NUM,ThingSpeak.setField(0,(float)3.14159)); 
  assertEqual(ERR_INVALID_FIELD_NUM,ThingSpeak.setField(9,(float)3.14159)); 
}

#if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_SAMD_MKR1000)  // Only the mega and mkr1000 has enough memory for all these tests
test(setFieldFloatCase) 
{
  // Always wait to ensure that rate limit isn't hit
  delay(WRITE_DELAY_FOR_THINGSPEAK);

  // Test max range values
  assertEqual(OK_SUCCESS, ThingSpeak.setField(1, (float)-999999000000));
  assertEqual(OK_SUCCESS, ThingSpeak.setField(2, (float)999999000000));
  
  // Test high precision values
  assertEqual(OK_SUCCESS, ThingSpeak.setField(3, (float)3.14159));
  assertEqual(OK_SUCCESS, ThingSpeak.setField(4, (float)-3.14159));
  
  // Test passing NaN and Inf
  assertEqual(OK_SUCCESS, ThingSpeak.setField(5, (float)NAN));
  assertEqual(OK_SUCCESS, ThingSpeak.setField(6, (float)INFINITY));
  assertEqual(OK_SUCCESS, ThingSpeak.setField(7, (float)-INFINITY));
  
  // Test out of range values
  assertEqual(ERR_OUT_OF_RANGE, ThingSpeak.setField(8, (float)-1000000000000.0));
  assertEqual(ERR_OUT_OF_RANGE, ThingSpeak.setField(8, (float)1000000000000.0));
  
  assertEqual(OK_SUCCESS,ThingSpeak.writeFields(testChannelNumber, testChannelWriteAPIKey));
}

test(setFieldIntCase) 
{
  // Always wait to ensure that rate limit isn't hit
  delay(WRITE_DELAY_FOR_THINGSPEAK);

  // Test max range values
  assertEqual(OK_SUCCESS, ThingSpeak.setField(1, (int)-32768));
  assertEqual(OK_SUCCESS, ThingSpeak.setField(2, (int)32767));
  
  assertEqual(OK_SUCCESS,ThingSpeak.writeFields(testChannelNumber, testChannelWriteAPIKey));
}

test(setFieldLongCase) 
{
  // Always wait to ensure that rate limit isn't hit
  delay(WRITE_DELAY_FOR_THINGSPEAK);

  // Test max range values
  assertEqual(OK_SUCCESS, ThingSpeak.setField(1, (long)-2147483648L));
  assertEqual(OK_SUCCESS, ThingSpeak.setField(2, (long)2147483647L));
  
  assertEqual(OK_SUCCESS,ThingSpeak.writeFields(testChannelNumber, testChannelWriteAPIKey)); 
}

test(setFieldCharStarCase) 
{
  // Always wait to ensure that rate limit isn't hit
  delay(WRITE_DELAY_FOR_THINGSPEAK);

  // Test empty string
  assertEqual(OK_SUCCESS, ThingSpeak.setField(1, ""));

  char longString[300];

  // Test max string
  memset(longString, '0',255);
  longString[255] = 0;
  assertEqual(OK_SUCCESS, ThingSpeak.setField(2, longString));

  // Test long string
  memset(longString, '0',sizeof(longString)/sizeof(longString[0]) - 1);
  longString[sizeof(longString)] = 0;
 
  assertEqual(ERR_OUT_OF_RANGE, ThingSpeak.setField(3, longString));
  
  assertEqual(OK_SUCCESS,ThingSpeak.writeFields(testChannelNumber, testChannelWriteAPIKey));
}

test(setFieldStringCase) 
{
  // Always wait to ensure that rate limit isn't hit
  delay(WRITE_DELAY_FOR_THINGSPEAK);

  // Test empty string
  assertEqual(OK_SUCCESS, ThingSpeak.setField(1, String()));

  unsigned int numChar = 300;
  String longString;
  longString.reserve(numChar);

  // Test max string
  for(unsigned int i = 0; i < 255; i++)
  {
    longString += '0';
  }
  assertEqual(OK_SUCCESS, ThingSpeak.setField(2, longString));

  // Test long string
  longString.reserve(numChar);
  for(unsigned int i = 0; i < numChar; i++)
  {
    longString += '0';
  }
  
  assertEqual(ERR_OUT_OF_RANGE, ThingSpeak.setField(3, longString));
  
  assertEqual(OK_SUCCESS,ThingSpeak.writeFields(testChannelNumber, testChannelWriteAPIKey));
}
#endif // Mega and MKR1000 only tests

void setup()
{
  Serial.begin(9600);
  while(!Serial); // for the Arduino Leonardo/Micro only
  Serial.println("Starting test...");
  #ifdef ARDUINO_AVR_YUN
    Bridge.begin();
  #else
    #if defined(ARDUINO_ARCH_ESP8266) || defined(USE_WIFI101_SHIELD) || defined(ARDUINO_SAMD_MKR1000)
      WiFi.begin(ssid, pass);
    #else
      Ethernet.begin(mac);
    #endif
  #endif
  ThingSpeak.begin(client);
}

void loop()
{
  Test::run();
}

