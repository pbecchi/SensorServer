
#define SERIALprint Serial.print
#define SERIALprintln Serial.println
//#define SERIALflush Serial.flush
#define SERIALavailable Serial.available
#define SERIALread Serial.read
#define SERIALwrite Serial.write
//#define TELNET
//---------------------------------------------------------------------
#ifdef TELNET
#define SERIALprint client[NC].print
#define SERIALprintln client[NC].println
//#define SERIALflush Serial.flush
#define SERIALavailable client[NC].available
#define SERIALread client[NC].read
#define SERIALwrite client[NC].write
#endif
#include <ESP8266WiFi.h>
#include <SSIDPASSWORD.h>

//how many clients should be able to telnet to this ESP8266
#define NC 1
#define MAX_SRV_CLIENTS NC

#ifdef TELNET
#define TELNET1 TELNET
WiFiServer server(23);
WiFiClient client[MAX_SRV_CLIENTS];

#endif 
//--------------------------------added for AT24C32 EEPROM I2C

#define AT24C32
//#include <EEPROM.h>

#ifdef AT24C32
#define AT24C32_ADDRESS    0x57 
#define DS3231_ADDRESS 0x68
class EE { 
  public :
   byte read(int );
   void write(int ,byte );
};  
EE EEPROM;
#endif
