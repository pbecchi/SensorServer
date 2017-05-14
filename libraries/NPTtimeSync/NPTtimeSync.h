#pragma once

#include <Time\TimeLib.h> 
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#else
#include <Ethernet.h>
#include <EthernetUdp.h>
#endif
#include <RTCLIB.h>
/////////////////////////to syncronise= setSyncProvider(getNtpTime);RTC.set(now()); 

extern RTC_DS1307 RTC;


time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
bool SyncNPT(byte DST);
