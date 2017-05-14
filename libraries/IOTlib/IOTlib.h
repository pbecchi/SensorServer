
class  IOTstr {
public:

	long Channel;
	const char* key;
	long values[8];
	byte fieldn[8];
	byte indval = 0;
	byte nchar = 0;
	IOTstr(char * mkey) { key = mkey; }
	byte updateThingSpeak(long value, byte field_n, byte code)
	{
		if (code == 0) {
			//void updateThingSpeak(long value, byte code) {
			//Serial.println("update");
			values[indval++] = value; nchar += 9; while (value >= 10) { value /= 10; nchar++; }
			fieldn[indval - 1] = field_n;
			return indval - 1;
		}
		else
		{
#if ARDUINO <100 	
			//Serial.println("connect....");
			byte IOTip[4] = { 184,106,153,149 };  //ip of api.thingspeak.com
			Client IOTclient(IOTip, 80);   //184,106,153,149
			if (IOTclient.connect()) {
#else
#ifndef ESP8266 
			EthernetClient IOTclient;
#else
			WiFiClient IOTclient;
#endif
			if (IOTclient.connect("api.thingspeak.com", 80)) {
#endif

				delay(1000);
				String url = "GET /update?api_key=";
				url += key;
				url += "&";
		//		SPL_D(url);
				IOTclient.print(url);// "GET /update?api_key=FPIY8XPZBC2YGSS7&");

				for (int ii = 0; ii < indval; ii++) {
		//			SP_D(fieldn[ii]); SP_D('>'); SPL_D(values[ii]);
					IOTclient.print("field"); IOTclient.print(fieldn[ii]); IOTclient.print("="); IOTclient.print(values[ii]);
					if (ii < indval - 1)IOTclient.print("&"); else IOTclient.println('\n');
				}

				//IOTclient.println(" HTTP/1.1\nHost: api.thingspeak.com\nConnection: close\n");//" HTTP/1.1\r\n" + "Host: " + "api.wunderground.com" + "\r\n" + "Connection: close \r\n\r\n"
				indval = 0; nchar = 0;
				delay(2000);
				byte inc = 0;
				//------------wait replay------------------------------------------------------------
				while (!IOTclient.available() && inc < 250) { delay(20); inc++; }
		//		SPL_D(inc);
		//		while (IOTclient.available()) { SP_D(IOTclient.read()); }

			}
		//	else { SPL_D("conn.failed"); }
			IOTclient.stop();
			return 0;

			}
		return 100;
		}
	};