#define OTA
#define IOT			//......for IOT thingsSpeak data upload (---IOT--input TBD.)
#define FIX_IP			//......for Fixed IP connection as from config.txt
#define OS_API			//......for use of command interpreter and remote nodes control
//#define FTP				//......for FTP on SPIFFS file
#define TELNET          //......for TELNET debug connection on port 23
#define READCONF
#define DHT11L 10       //..... for  use DTH & RHT family sensors define input pin 
/////////////////////////////////////////////////////////////////////////////////////
#include "DHT11lib.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <FS.h>
//#include <SSIDPASSWORD.h>
#include <ArduinoOTA.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>
#include "NPTtimeSync1.h"
//#include <ArduinoJson.h>
#include "ET_penmam.h"
#include "Eeprom_ESP.h"
#include <ESP8266OneWire.h>
#include <DallasTemperature.h>
#include <Arduino-Ping-master\ESP8266ping.h>
#include <ThingSpeak.h>
#define VARIAB Lvalue                   //define internal variable accessed using function val(position);
long Lvalue[20];
#include "bitlashMio.h"                //bitlash interpreter called with espressione("expression formula")
#include "bitlashMio-functions.h"      //bitlash interprter functions including added now()->epochtime ; val(i)->internal VARIAB[i]

//#define FTP
#ifdef FTP
#include <ESP8266FtpServer.h>
FtpServer ftpSrv;
#endif
//#define TELNET
byte ntimes = 0;
byte eecode = 255;
#define START_INTERVAL 200000

#ifdef TELNET
WiFiServer Tserver(23);
WiFiClient Tclient;

const char * IotAPIkey = "KX8OBCOWA7MP8H56";
long IotChannel = 178477UL;
byte IOTflag[8] = { 1,0,1,10,11,0,0,0 };

byte n_out_pin=0;
bool noClient = true;
long TimeOUT;
#define ONTIME 2000000L
int getClients()
{
	if (noClient) {
		Tclient = Tserver.available();
		if (Tclient) {
			noClient = false;
			Serial.print("New client! ");
			Tclient.flush();
			Tclient.print('>');

			TimeOUT = millis();
			return 1;
		}

		return 0;
	}
	else  //noclient false
		if (!Tclient.connected() || millis()>TimeOUT + ONTIME) {
			Tclient.stop();
			noClient = true;
			return 0;
		}
	return 1;
}
#define SP(x) {if(Tclient)Tclient.print(x);Serial.print(x);}
#define SPL(x) {if(Tclient)Tclient.println(x);Serial.println(x);}
#define SPS(x) {if(Tclient){Tclient.print(' ');Tclient.print(x);}Serial.print(x);}
#define SP_D(x) {if(Tclient)Tclient.print(x);Serial.print(x);}
#define SPL_D(x) {if(Tclient)Tclient.println(x);Serial.println(x);}
#define SPS_D(x) {if(Tclient){Tclient.print(' ');Tclient.print(x);}Serial.print(x);}
#else
#define SP(x) Serial.print(x)
#define SPL(x) Serial.println(x)
#define SPS(x) Serial.print(' ');Serial.print(x)
#define SP_D(x) Serial.print(x)
#define SPL_D(x) Serial.println(x)
#define SPS_D(x) Serial.print(' ');Serial.print(x)
#endif

#ifdef DHT11L
//#include <SparkFun_RHT03_Particle_Library-master\firmware\SparkFunRHT03.h>
//#include <Arduino-DHT22-master\DHT22.h>
//#include "DHT11lib.h"
#include <DHT-sensor-library-master\DHT.h>
DHT DHT(DHT11L,DHT22);
#endif

ESP8266WebServer server(80);
#define BUFFDIM 2024
const int led = 16; //.....13 for UNo..... 16 for ModeMCU
int opt[20] = { 360,360,10	,0	,12	,192,168,1,30,10	,3600,	24, 441247,	82544,	23,		30,		12, 32 };//  default values
File logfile;

enum COP { WU_URL, SSID, PSW, ESP_STATION, TIME_STR };
String OpName[10] = {
	"/api/48dfa951428393ba/conditions/q/Italy/", //"/api/48dfa951428393ba/conditions/q/Italy/"
	"Vodafone-Out",
	"paolo-48",
	"poolESP"
	"t:            ",
	"               ",
	"                "
};
///-----------------------------class for linear regression--------------------------------------------------
class Level {
public:
#define N3600 360
#define N12 24
	byte n_sample = 0, SN[N12];
	long sumX, sumY, sumXY, sumX2;
	long SX[N12], SY[N12], SX2[N12], SXY[N12];
	void levelBegin() {
		sumX = 0;
		sumX2 = 0;
		sumXY = 0;
		sumY = 0;
		n_sample = 0;
	}
	void levelFit(long ora, float value) {
		sumX += ora;
		sumX2 += ora*ora;
		sumY += value;
		sumXY += value* ora;
		SY[ora / N3600] = sumY;
		SXY[ora / N3600] = sumXY;
		SX[ora / N3600] = sumX;
		SX2[ora / N3600] = sumX2;
		SN[ora / N3600] = n_sample++;
	}
	byte levelGet(byte ind1, byte ind2, float * slope, float * inter) {
		int ns = SN[ind2] - SN[ind1];
		if (ns > 0) {
			long s1 = SY[ind2] - SY[ind1];
			long s2 = SX[ind2] - SX[ind1];
			long s3 = SXY[ind2] - SXY[ind1];
			long s4 = SX2[ind2] - SX2[ind1];
			*slope = float(s1*(s2 / 10) - ns*(s3 / 10)) / float(s2*(s2 / 10) - ns*(s4 / 10));
			*inter = (s1 - *slope*s2) / ns;
			SP(ns); SPS(*slope); SPS(*inter); SPL();
		}
		return ns;
	}
};
Level pool;
//-------------------------------weather class---------------------------------------------------
struct Weather
{
	time_t time;        //"local_epoch"			local epoch time
	int temp;			//"temp_c"				temp ° celsius
	int humidity;		//"relative_humidity"	humidity %
	int rain1h;			//"precip_1h_metric"	rain last hour 0.1mm
	int rain;			//"precip_today_metric" rain today 0.1mm
	int wind;			//{"wind_kph"}			wind speed Km/h
						//		int atpres;			//"pressure_mb"			atm. pressure milliBar
	 int sunrad;		//solar radiation from WU station
	float penman;		//ETo penmam-monteith
	float water;		//solar KWh on solar panels 
	void write() {
		File wfile;
		if (SPIFFS.exists("/weather.log"))
			wfile = SPIFFS.open("/weather.log", "r+");
		else
			wfile = SPIFFS.open("/weather.log", "w+");

		if (!wfile) { Serial.println("Cannot open weather.log"); return; }
		wfile.seek(0, SeekEnd);
		wfile.seek(0, SeekEnd);
		wfile.print(time); wfile.print(',');
		wfile.print(temp); wfile.print(',');
		wfile.print(humidity); wfile.print(',');
		wfile.print(rain1h); wfile.print(',');
		wfile.print(wind); wfile.print(',');
		wfile.print(penman); wfile.print(',');
		wfile.print(water); wfile.print(',');
		wfile.print(sunrad); wfile.print(',');
		wfile.println(rain);
		wfile.close();
	}
	bool read(time_t timev) {
		char buf[20];
		byte n;
		time = 0;
		File wfile;
		if (SPIFFS.exists("/weather.log"))
			wfile = SPIFFS.open("/weather.log", "r+");
		else
			wfile = SPIFFS.open("/weather.log", "w+");

		if (!wfile) {
			Serial.println("Cannot open weather.log"); return 0;
		}
		wfile.seek(0, SeekEnd);
		long wfilepos = wfile.position();
		wfile.seek(0, SeekSet);
		while (time < timev&&wfile.available()) {
			if (wfile.find(10)) {
				n = wfile.readBytesUntil(',', buf, 20);
				buf[n] = 0;
				time = atol(buf);
				//		SPS_D(buf);
			}
			else {
				if (wfile.read()<0) { wfile.close(); return 0; }
			}

		}
		if (!wfile.available()) { wfile.close(); return false; }
		n = wfile.readBytesUntil(',', buf, 20);

		buf[n] = 0; //SPS_D(buf); 
		temp = atoi(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0; //SPS_D(buf); 
		humidity = atoi(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0; //SPS_D(buf); 
		rain1h = atoi(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0; //SPS_D(buf);
		wind = atoi(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0;// SPS_D(buf);
		penman = atof(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0;// SPS_D(buf);
		water = atof(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0;// SPS_D(buf);
		sunrad = atoi(buf);

		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0;// SPS_D(buf);
		rain = atoi(buf);


		wfile.seek(wfilepos, SeekSet);
		wfile.close();
		return true;
	}

};

#define MAX_WEATHER_READ 10
Weather weather[MAX_WEATHER_READ]; byte iw = 0;
#define ET0
#define SUNPANELFACTOR opt[17]/100. //0,36
//---------------------------------Geo/sun constant structure
#ifdef ET0
struct Geo
{
	float lat;//{"latitude"}
	float lon;//{"longitude"}
	int alt;//{"elevation"}
	float alfa ;//solar panels Elevation
	float beta ;//solar panels Azimuth
};
Geo sta;
//{ 44.12474,8.25445,23, 30. / 180. * PI, 12. / 180. * PI };
float slopeV[10]; byte ki = 0;

float rad_ratio = 0.8, prev_elevation = 0;
float ET0_calc(byte type) {
	Weather w_mean;
	Weather w_max, w_min;
	byte iwp = iw + 1;
	byte iwm = iw - 1;
	if (iw == 0)iwm = MAX_WEATHER_READ - 1;
	if (iw == MAX_WEATHER_READ-1)iwp = 0;

	if (weather[iwp].time == 0)return -1;
	w_mean.time = 0;
	w_mean.rain1h = 0;
	w_max.time = weather[iw].time;
	w_max.rain = weather[iw].rain;  // today rain
	w_min.rain = weather[iwp].rain;
	w_min.time = weather[iwp].time;
	w_mean.temp = 0;
	w_max.temp = -10;
	w_min.temp = 50;
	w_mean.humidity = 0;
	w_max.humidity = 0;
	w_min.humidity = 100;
	w_mean.wind = 0;
	w_mean.sunrad = 0;
	w_max.sunrad = 0;
	w_min.sunrad = 1200;
	w_mean.water = 0;// (weather[iw].water - weather[iwp].water) / (weather[iw].time - weather[iwp].time) * 3600;		//kwH produced last 1 h
	byte iel = 0;
	time_t timep=0;
	byte nwaterread = 0, n_sunrad = 0,n_temp=0,n_wind=0,n_rain1h=0;
	float waterp = weather[MAX_WEATHER_READ-1].water;
	for (byte i = 0; i < MAX_WEATHER_READ; i++) {
		//	SPS_D(weather[i].temp); SPS_D(weather[i].humidity); SP_D(" "); SPL_D(weather[i].wind);
		w_mean.time += weather[i].time/MAX_WEATHER_READ;
		if (weather[i].rain1h > 0)// distance level 
		{
			w_mean.rain1h += weather[i].rain1h;
			n_rain1h++;
		}
		if (weather[i].temp > 0) {
				w_mean.temp += weather[i].temp;
				if (w_max.temp < weather[i].temp)w_max.temp = weather[i].temp;
				if (w_min.temp > weather[i].temp)w_min.temp = weather[i].temp;
				n_temp++;
			}
		if (weather[i].humidity > 0) {
			w_mean.humidity += weather[i].humidity;

			if (w_max.humidity < weather[i].humidity)w_max.humidity = weather[i].humidity;
			if (w_min.humidity > weather[i].humidity)w_min.humidity = weather[i].humidity;
		}
		if (weather[i].wind >= 0) {
			w_mean.wind += weather[i].wind;
			n_wind++;
			if (w_max.wind < weather[i].wind)w_max.wind = weather[i].wind;
			if (w_min.wind > weather[i].wind)w_min.wind = weather[i].wind;
		}
			if (weather[i].sunrad > 0) {
				n_sunrad++;
				w_mean.sunrad += weather[i].sunrad;
				if (w_max.sunrad < weather[i].sunrad)w_max.sunrad = weather[i].sunrad;
				if (w_min.sunrad > weather[i].sunrad)w_min.sunrad = weather[i].sunrad;
			}//_________________________________________solar panels Kwh mean computed as------> w
			if (timep!=0&&weather[i].water != -1 && waterp != -1&&weather[i].water>waterp&&weather[i].time>timep+60) {
				float solar_panels_watts = (weather[i].water - waterp) / (weather[i].time - timep);
				if (solar_panels_watts < 2) {					//2*3600 ->7200 Watts max
					w_mean.water += solar_panels_watts;
				//	SPS("_"); SPS(weather[i].time - timep); SPS(solar_panels_watts); SPS("_");
					//			SP(weather[i].water); SP(" "); SPL(waterp);
					nwaterread++;
				}
			}
			waterp = weather[i].water;
			timep = weather[i].time;
#define SUN_START 300
	}
	if (n_temp == 0 || n_wind == 0 )return -1;
	w_mean.rain1h /= MAX_WEATHER_READ;
	w_mean.temp /= n_temp;
	w_mean.wind /= n_wind;
	w_mean.humidity /= MAX_WEATHER_READ;
	if (n_sunrad > 0)w_mean.sunrad /= n_sunrad; 
	if(nwaterread>0)
		w_mean.water = w_mean.water/nwaterread*3600;//*MAX_WATER_READ; back to -------->KwH
	else w_mean.water = 0;
	long sumProd = 0, sumSqr = 0;
	 SP(" t "); SPL(w_mean.time);
	for (byte i = 0; i < MAX_WEATHER_READ; i++) 
	if(weather[i].rain1h>0){
		//SP(sumProd); SP(' '); SP(sumSqr); SP("Dtime"); SP(long(weather[i].time) - long(w_mean.time)); SP(" D "); SPL(weather[i].rain1h - w_mean.rain1h);
		sumProd +=(long(weather[i].time) - long(w_mean.time))*(weather[i].rain1h - w_mean.rain1h);
		sumSqr += (long(weather[i].time) - long(w_mean.time))*(long(weather[i].time) - long(w_mean.time));
	}
	float sunset_time = sunrise__sunset_localtime(sta.lon, sta.lat, 2, 1);
	float sunrise_time = sunrise__sunset_localtime(sta.lon, sta.lat,2, 0);
	SP("sunrise"); SPS(sunrise_time); SPS("sunset"); SPL(sunset_time);
	int doy = (month(w_max.time) - 1) * 30 + day(w_max.time);

#define MID_DAY (sunset_time!=0?(sunset_time+sunrise_time)/2:13.5) //mid day sun hour
	// used local time weather.time converted to GMT added latitude time angle ...... better to use GMT time!!
	//                time_angle as local time_angle referred to south
	float time_angle = (hour(weather[iw].time) + minute(weather[iw].time) / 60. - MID_DAY + 12*sta.lon / 180.-0.5);// -0.5h time shift for 1h average data
	SP("t_a"); SPS(hour(weather[iw].time)); SPL(time_angle);
	float visibility = 50;       //               _______________best visibility km
	float sun_elevation = asin(sin(radians(sta.lat))*sin(sol_dec(doy)) + cos(radians(sta.lat))*cos(sol_dec(doy))*cos(time_angle*3.1415 / 12));
	float sun_azimut = acos((sin(sun_elevation)*sin(sta.lat*PI / 180) - sin(sol_dec(doy))) / (cos(sun_elevation)*cos(sta.lat*PI / 180.)));
	if (time_angle<0) 	if(	sun_azimut>0)sun_azimut *= -1.;
	else 	if (sun_azimut<0)sun_azimut *= -1.;
	prev_elevation = sun_elevation;
	float time_factor = cos(sun_azimut)*cos(sta.beta) + sin(sun_azimut)*sin(sta.beta);
	float sun_panel_inclination_factor = (cos(sta.alfa)*sin(sun_elevation) + sin(sta.alfa)*cos(sun_elevation)*time_factor);
	SPS("sun_el "); SP(sun_elevation); SPS("Az"); SPS(sun_azimut); SPS("P_f"); SPS(sun_panel_inclination_factor);
	
	SP(" corrF"); SPL(sin(sun_elevation) / sun_panel_inclination_factor);
	float expected_sunrad = 0;
	if (sun_elevation > 0)
		expected_sunrad = 1352.*exp(-(39 / visibility + 0.85)*atmos_pres(sta.alt) / atmos_pres(0) / (0.9 + 9.4 * sin(sun_elevation)));
	/*	SPS_D(sol_dec(doy)); SPS_D(MID_DAY); SPS_D(time_angle); SPS_D(sun_elevation); SPS_D("es"); SPL_D(expected_sunrad);
		static int nfactor;
		if (expected_sunrad > 10)
			if (nfactor == 0)
				if (time_angle > 0)  // if is a restart and afternoon recompute previous_factor from records
					for (byte ii = 1; ii < 6; ii++) {
						iwo = iw - ii; if (iwo < 0)iwo = MAX_WEATHER_READ - iwo;
						time_angle = (hour(weather[iwo].time) + 2. + minute(weather[iwo].time) / 60. - MID_DAY + sta.lon / 360.);
						sun_elevation = asin(sin(radians(sta.lat))*sin(sol_dec(doy)) + cos(radians(sta.lat))*cos(sol_dec(doy))*cos(time_angle*3.1415 / 12));
						float expecte_sunrad = 0;
						if (sun_elevation > 0)
							expecte_sunrad = 1352.*exp(-(39 / visibility + 0.85)*atmos_pres(sta.alt) / atmos_pres(0) / (0.9 + 9.4 * sin(sun_elevation)));

						previous_factor = (previous_factor*nfactor + weather[iwo].sunrad) / (nfactor + expecte_sunrad);
						nfactor = nfactor + expecte_sunrad;
					}

		if (savona_result ||
			(weather[iw].sunrad > expected_sunrad || weather[iw].sunrad < expected_sunrad*0.05)) //error reading sunrad assume =previous reading
		{
			SPS_D("apply cal. sunrad");
			if (previous_factor < 1.) {
				weather[iw].sunrad = expected_sunrad*previous_factor;
			}
			else weather[iw].sunrad = expected_sunrad;
		}
		else			//----------------------compute previous_factor as average of sunrad/expected_sunrad
			if (expected_sunrad > 10) {
				previous_factor = (previous_factor*nfactor + weather[iw].sunrad) / (nfactor + expected_sunrad);
				nfactor = nfactor + expected_sunrad;
			}
			else nfactor = 0;
			SP("p_f"); SP(previous_factor);
			//compute day cumulative sun radiation
			float day_sunrad = 0;
			for (byte j = 1; j < 24 / FREQ_WEATHER; j++) {
				int i = iw - j; if (i < 0)i = MAX_WEATHER_READ + 1 + i;
				iwo = i + 1; if (i > MAX_WEATHER_READ)iwo = 0;
				//		if (j == 0)iwo = iw - 24 / FREQ_WEATHER + 1; if (iwo < 0)iwo = MAX_WEATHER_READ + iwo + 1;
				if (weather[i].sunrad >= 0 && weather[i].sunrad < 1100 && weather[iwo].sunrad >= 0 && weather[iwo].sunrad < 1100) {
					long dtime;
					if (j == 0)
						//		 dtime = -(weather[i].time - weather[iwo].time-SECS_PER_DAY);
						dtime = (-hour(weather[iwo].time) * 3600 + minute(weather[iwo].time) * 60) + (hour(weather[i].time) * 3600 + minute(weather[iwo].time) * 60);
					else
						dtime = -(weather[i].time - weather[iwo].time);

					if (dtime > 40000)dtime = 12000;                              // missing data---max step 3.5 hours
					day_sunrad += (weather[iwo].sunrad + weather[i].sunrad) / 2 * (dtime / 1000);
					SPS_D(i); SPS_D(iwo); SPS_D(dtime); SPS_D(weather[i].time); SPS_D(weather[i].sunrad); SP_D(" "); SPL_D(day_sunrad);
				}
			}
			if (day_sunrad > 28000.) {
				byte	ix = iw - 1;
				if (ix < 0)ix = MAX_WEATHER_READ;
				while (weather[ix].penman > 6.) {
					ix--; if (ix < 0)ix = MAX_WEATHER_READ;
				}
				weather[iw].penman = weather[ix].penman;
			}
			else
			{
				w_mean.sunrad = int(day_sunrad);
			*/
			SP("t"); SPS(w_max.temp); SPS(w_mean.temp); SPS_D(w_min.temp);
				SP_D("h"); SPS(w_max.humidity); SPS(w_min.humidity);
				SP_D("w"); SPS_D(w_mean.wind);
				SP_D("s"); SPS_D(w_mean.sunrad); 
				SP_D("Sp"); SPL(w_mean.water); 
		
		//__________________penman____________________________________________
	int sunHours = 10;
	float rad_clearsky = clear_sky_rad(
		sta.alt,
		et_rad(sta.lat, sta.lon,
			sunset_hour_angle(sta.lat, sol_dec(doy)),
			inv_rel_dist_earth_sun(doy)));
	float Sun,ET;

	if (type == 0 && w_mean.sunrad< 1200) {										// calculate from WU station sunrad
		Sun = w_mean.sunrad / 1000;
		if (expected_sunrad>100)rad_ratio = Sun / expected_sunrad;
		SPS("rad_r"); SPS(rad_ratio);
		 ET=ETo_hourly(float(w_mean.sunrad), w_mean.temp, w_mean.wind / 3.6,
			mean_es(w_min.temp, w_max.temp),
			ea_from_tmin(w_min.temp),
			delta_sat_vap_pres(w_mean.temp),
			psy_const(atmos_pres(sta.alt)),
			rad_ratio);
		 SP("ET-0="); SPL(ET);
		 if (ET>0)return ET;
		 else return 0;
	}
	else if (type == 1 && w_mean.water < 5000) {

		Sun = w_mean.water*SUNPANELFACTOR;
		if (sun_panel_inclination_factor>0.01)
			Sun = w_mean.water * SUNPANELFACTOR *sin(sun_elevation) / sun_panel_inclination_factor;	//account for angle and size
		
			
		SPS("Sun"); SPS(Sun);
		if (expected_sunrad> 100)rad_ratio = Sun / expected_sunrad;
		SPS("rad_r"); SPS(rad_ratio);
		float rad_out = 4.9E-09*pow(w_mean.temp+275., 4.)*(0.34 - 0.14*sqrt(ea_from_tmin(w_min.temp)))*(1.35*rad_ratio - 0.35);
		SPS(" r_O "); SPS(rad_out);
		ET= ETo_hourly(Sun, w_mean.temp, w_mean.wind / 3.6,
			mean_es(w_min.temp, w_max.temp),
			ea_from_tmin(w_min.temp),
			delta_sat_vap_pres(w_mean.temp),
			psy_const(atmos_pres(sta.alt)),
			rad_ratio);
		 SP("ET1="); SPL(ET);
		if (ET>0)return ET;
		else return 0;
	}
	else
		if (type == 3) {
			ki++; if (ki == 10)ki = 0; slopeV[ki] = (sumProd  * 3600.) / sumSqr;
			return slopeV[ki];
		}
		else
			if (type == 4){float tot = 0;
			for (byte iki = 0; iki < 10; iki++)tot += slopeV[iki]; return tot / 10.;
		}
	else
	{																			//calculate from Solar panel WATTS
		Sun = sol_rad_from_sun_hours(
			daylight_hours(doy),
			sunHours,
			et_rad(sta.lat, sta.lon,
				sunset_hour_angle(sta.lat, sol_dec(doy)),
				inv_rel_dist_earth_sun(doy)));
	}
	SP(rad_clearsky); SP(' '); SP(Sun);

	float net_radi = net_rad(net_in_sol_rad(Sun),
		net_out_lw_rad(
			w_min.temp, w_max.temp,
			Sun,
			rad_clearsky,
			ea_from_tmin(w_min.temp)));
	SP(' '); SP(net_radi);
    weather[iw].penman = penman_monteith_ETo(net_radi, w_mean.temp, w_mean.wind / 3.6,
		mean_es(w_min.temp, w_max.temp),
		ea_from_tmin(w_min.temp),
		delta_sat_vap_pres(w_mean.temp),
		psy_const(atmos_pres(sta.alt)),
		0.10);
	return weather[iw].penman;
}


#endif
void pulseLed(int dur, int pause, int times) {
	for (byte i = 0; i < times; i++) { digitalWrite(led, 0); delay(dur); digitalWrite(led, 1); delay(pause); }
}
volatile unsigned long npulses = 0, oldmillis = 0, pulsmils;
void setupinterrupt(byte pin, byte mode) {
	pinMode(pin, INPUT_PULLUP);

	//  digitalWrite(pin,HIGH);
	attachInterrupt(digitalPinToInterrupt(pin), count, mode);
}
#define MINPULSDUR 100
void count() {
	noInterrupts();
	
	unsigned long dif= millis() - oldmillis;
	if (dif > MINPULSDUR) {  //____________________________short pulses not considered____________________________ 
		npulses++;
		oldmillis = millis();
		pulsmils = dif;
	}
	interrupts();
}

//_________________________________________________________________________________________________________________________________________class SENSOR
class Sensor {
public:
	

	byte nsensors = 0, sensorT[10], pin1[10], pin2[10],kk=0;
	float value[30];
	unsigned long time_sensor = 0, time_sensor1 = 0;
	String  name[15];
	String  param[30];
//	byte format[30] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	float sumY, sumXY,sumX,sumX2;
	float SX[24], SY[24], SX2[24], SXY[24];
	byte n_sample, SN[24];
	void beginSensor(byte type, byte pin, byte n_par,char  nome[],  String  par[]) {
		int temp[2];
		sensorT[nsensors] = type;
		pin1[nsensors] = pin;
		pin2[nsensors] = n_par;
		name[nsensors] = nome;
		if (type == 1) DHT.begin();
		for (byte i = 0; i < n_par; i++)param[kk++] = par[i];
		//if (time_sensor == 0)time_sensor = time;
		nsensors++;
		SP(sensorT[nsensors - 1]); SP(' ');
		SP(pin1[nsensors - 1]); SP(' ');
		SP(name[nsensors - 1]); SP(' ');
		for (byte i = 0; i < n_par; i++)SP(param[kk-1-i]);
		SPL();
		if (type == 5) { if (start_read_OneWire(pin, temp, 0) < n_par) SPL("not found Temp sensor"); }
		else if (type == 8) setupinterrupt(pin, CHANGE);

	} //setup sensors (type,byte pin);
	  //--------------------------------------------------------------------------
#define MIL_JSON_ANS 60000
#define MAX_JSON_STRING 2500
	//char json[2600];
	byte APIweatherV(String streamId, String nomi[], byte Nval, float val[]) {
		WiFiClient client;

		char json[2500];
		//if (client.connected()) client.stop();


		SPL("connecting to WU");

		const int httpPort = 80;
		if (!client.connect("api.wunderground.com", 80))
		{
			client.stop();
			SP("connection failed ");// SPL(jsonserver);
			pulseLed(2000, 0, 1);
			return 0;
		}
		//   SP("mem.h."); SPL(ESP.getFreeHeap());
		String url = "GET ";
		//	streamId= "/api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json";
		url = url + streamId
			+ " HTTP/1.1\r\n" + "Host: " + "api.wunderground.com" + "\r\n" + "Connection: close \r\n\r\n";
		client.print(url);

		int i = 0, json_try = 0; bool ISJSON = false; byte ii = 0;
		//	Serial.println("Waiting Json");
		long time_step = millis() + MIL_JSON_ANS;
		char c, cp, cpp; bool obs = false;
		delay(500);
		while (millis() < time_step)
		{
			// Read all the lines of the reply from server and print them to Serialbol 
			while (millis() < time_step -MIL_JSON_ANS / 2 && !obs)
			{

				int nby = client.available();
				if (nby>100) {
#ifdef VERIFY_WU_ANSWER
					Serial.print(client.read());
#else
					obs = client.find("current_observation");
#endif

				}
				else
				{
					SP(nby); SP('-');
					pulseLed(nby + 40, 50, 1);
				}
			}

			if (!obs) {
				SPL("error");
				while (client.available()) SP(client.read());
				pulseLed(1000, 0, 1);
				client.stop(); return 0;

			}
			//_________________timestep is connection timeout_______________________________ }}} ____end of jsonstring
			while (millis() < time_step&&c != '}'&&cp != '}'&&cpp != '}')

				while (obs&&client.available() && i < MAX_JSON_STRING) {
					cpp = cp;
					cp = c;
					c = client.read();
					//					if (c == '{')
					ISJSON = true;
					if (ISJSON) {
						json[i++] = c;
						//	SP(c);

					}

				}
			if (ISJSON) {
				json[i - 1] = 0;
				client.stop();
				//SP("Connected ! "); SPL(url);
				SP(" Json read!"); SPL(i);
				Serial.print("m.b.h."); Serial.println(ESP.getFreeHeap());
#define JSONLIB
#ifndef JSONLIB
				DynamicJsonBuffer jsonBuffer;

				JsonObject& root = jsonBuffer.parseObject(json);
				Serial.print("m.a.h."); Serial.println(ESP.getFreeHeap());
				// Test if parsing succeeds.
				if (!root.success()) {
					SPL("Weather parseObject() failed");
					return 0;

				}
				else {

					SPL("Weather Parsing...");
					for (byte i = 1; i < Nval + 1; i++) {
						if (nomi[i] == "local_epoch")
						{
							time_t time = root["local_epoch"];
							SPL(time);
							val[i - 1] = (float)time;
						}
						else
						{
							float valore = root[nomi[i]];

							//			const char * nul= root[nomi[i]];
							//			if (nul == "--")valore = -1;
							val[i - 1] = valore;
							SP(nomi[i]); SPL(valore);
							//	SP_D(valore);
						}

					}
					return 1;

				}
#else				//va_end(args);
				byte ret = JsonDecode(Nval + 1, json, nomi, val);
				for (byte i = 1; i <= Nval; i++) {
					SP(nomi[i]);
					val[i - 1] = val[i];
					SPL(val[i]);

				}
				return ret;
#endif

			}

			else SPL("no json");

		}

		client.stop();
	//	SP("mem.h."); SPL(ESP.getFreeHeap());
		return 0;

	}
/*	byte prova() {
		WiFiClient client;
		const int httpPort = 80;
		if (!client.connect("api.wunderground.com", 80))
		{
			client.stop();
			SP("connection failed ");// SPL(jsonserver);
			pulseLed(2000, 0, 1);
			return 0;
		}
	//	/api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json HTTP/1.1
	//	Host: api.wunderground.com
	//	Connection: close 
		String url = "GET ";
		String streamId = "/api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json";
		//url += "?pw=";
		//url += privateKey;
		url = url + streamId
			+ " HTTP/1.1\r\n" + "Host: " + "api.wunderground.com" + "\r\n" + "Connection: close \r\n\r\n";
		//url += "&value=";
		//url += value;
		//	client.flush();
		//	Serial.print(" Requesting URL: ");
		//	Serial.print(url);
		client.print(url);
	//	client.print("GET /api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json  HTTP/1.1 \r\n Connection: close \r\n \r\n");
		ulong millismax = millis() + 10000;
		while (!client.available() && millis() < millismax)delay(10);
		if (!client.available())SPL("no rep");
		while (client.available())SP(char(client.read()));
		client.stop();
	}
*/
//#define MYDECODE
#ifdef MYDECODE
float getvalue(char * buff, String nome) {
	char* buffin;
	char * p2;
	char * p1;
		nome = char(34) + nome + char(34);
		SPL(nome);
		buffin = strtok_r(buff, ",",&p1);
		SPL(buffin);
		while (buffin != NULL&&strtok_r(buffin, ":", &p2) != nome.c_str()) {
			SPL(buffin); buffin = strtok_r(NULL, ",", &p1);
		}
		if (buffin == NULL)return -1;
		else {
			float val;
			char * pointer = strtok_r(NULL, ",", &p2);
			if (strchr(pointer, '.') == NULL) val = (float)atol(pointer);
			else  val = atof(pointer);
			return val;
		}
	}
	byte JsonDecode(byte Nval, char json[], String nomi[], float val[]) {
		for (byte i = 0; i < Nval; i++)
			val[i] = getvalue(json, nomi[i]);
	}
#else

#define MYDECOD

#ifdef MYDECOD
	byte JsonDecode(byte k, char buff[], String nome[], float val[]) {
		byte ret = 0;
		char* buffin;
		char * p2;
		char * p1;
		byte i = 0;
		//for (i = 0; i<k; i++)
		//	nome[i] = char(34) + nome[i] + char(34);
		//SPL(nome);
		buffin = strtok_r(buff, ",", &p1);

//		SPL(buffin);
		char * title = " ";
		int comp = -1;
		while (buffin != NULL)
		{
			buffin = strtok_r(NULL, ",", &p1);
			if (buffin != NULL) {
				// SPL(buffin);
				title = strchr(strtok_r(buffin, ":", &p2), '"');

				if (title != NULL) {
					// SP(strlen(title)); SPL(title);
					i = 1;  //------------------------------first nome not used-------------
					while (comp != 0 && i<k) {
						String nomev = char(34)+nome[i] + char(34);
						comp = strcmp(title,nomev.c_str()); i++;
					}
					if (comp == 0) {
						Serial.print("T "); Serial.println(title);
						// float val;
						char valore[20];
						char * pointer = strtok_r(NULL, ",", &p2);
						char *point1 = strchr(pointer, '"');			//if value in between " " extract it
						if (point1 != NULL) {
							byte kk = 1;
							while (point1[kk] != '"') { valore[kk - 1] = point1[kk++]; }
							valore[kk - 1] = 0;
						}
						else strcpy(valore, pointer);
					
						if (strchr(valore, '.') == NULL) {
							long v = atol(valore); Serial.println(v);
							val[i - 1] = (float)v;
						}
						else  val[i - 1] = atof(valore);
						ret = 1;
						Serial.println(val[i - 1]);
						comp = -1;
					}
				}
				else comp = -1;
			}
		}
		return ret;

	}
#else

	byte JsonDecode(byte Nval,char * json, String nomi[], float val[]) {
		DynamicJsonBuffer  jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(json);
		//Serial.print("m.a.h."); Serial.println(ESP.getFreeHeap());
		// Test if parsing succeeds.
		if (!root.success()) {
			SPL("Weather parseObject() failed");
			SPL(json);
			return 0;
		}
		else {
			SPL("Weather Parsing...");
			for (byte i = 1; i < Nval + 1; i++) {
				if (nomi[i] == "local_epoch")
				{
					time_t time = root["local_epoch"];
					SPL(time);
					val[i - 1] = (float)time;
				}
				else
				{
					float valore = root[nomi[i]];
					//			const char * nul= root[nomi[i]];
					//			if (nul == "--")valore = -1;
					val[i - 1] = valore;
					SP(nomi[i]); SPL(valore);
					//	SP_D(valore);
				}
			}
			return 1;
		}
	}
#endif
#endif
	long lvalue[20], sumET0 = 0;
	// EEPROM 900--remote commands--------1000--rules---------
#define WEATHERPOS 10		//eeprom pos for weather[i]
#define MYPOS 500			//eeprom pos for pool[24] 
#define SUMETPOS 6			//eeprom pos for total dayly ET0
#define WLEV0H 2			//"		"" for water level at 0:0 h
#define ERROR_P 0				//"		"	for errors
#define PINGTIMES 10
#define EELONGW(x, y) { eeprom_write_block(&y,(void *)x,4);}
#define EELONGR(x, y) {eeprom_read_block(&y,(void*)x,4); }
#define POOLFACTOR opt[POOL_FACTOR]//12	// surface of swimming pool area 96mq / area of expansion tank 8mq


//_____________________________________________________________________________________________________________________
	bool readSensors(long timeint) {//read and record sensors values each time interval sec
		
		if (millis() > time_sensor) {
			//          0h first time of the day
			if (now() % SECS_PER_DAY < timeint) {
				sumET0 = 0;
				pool.levelBegin(); //sumY = 0; sumXY = 0; sumX = 0; sumX2 = 0; n_sample = 0;
			}

			byte ntry = 0;
			SP("ping ");
			while (!Ping.ping(IPAddress(192, 168, 1, 1))) {
				pulseLed(100, 100, 1); ntry++;
				if (ntry > 250)return 0;
			}
			SP("OK"); SPL(ntry);
			byte k = 0;
			time_sensor = millis() + timeint * 1000;
			iw++; if (iw >= MAX_WEATHER_READ)iw = 0;
			
			for (byte i = 0; i < nsensors; i++) {
				if (sensorT[i] == 0)           //---------------------- Ultrasonic Distance sensor
				{
					int readings[PINGTIMES], minv = 0, maxv, it = 0;
					byte rept = 0;
					SPL((opt[2] > PINGTIMES ? PINGTIMES : opt[2]));
					while ((rept<20)&&(it < (opt[2]>PINGTIMES ? PINGTIMES : opt[2]))) {
						readings[it] = readDistance(pin1[i] / 16, pin1[i] % 16, weather[iw].temp);//distance 0..1 mm
						if (readings[it] > 0) {
							minv += readings[it]; it++;
						}
						rept++;
						SP(rept); SP("  "); SPL(it);
						
					}
					if (it > 0) {
						lvalue[k] = 0;
						SPS(maxv); SPS(minv);
						maxv = minv*1.1 / it; minv = minv*0.9 / it; it = 0;
						for (byte rept = 0; rept < (opt[2]>PINGTIMES ? PINGTIMES : opt[2]); rept++)
							if (readings[rept] > minv&&readings[rept] < maxv) {
								lvalue[k] += readings[rept]; it++;
							}
						if (it > 0)lvalue[k] /= it;
					}
					else lvalue[k] = -(opt[2]>PINGTIMES ? PINGTIMES : opt[2]);
					lvalue[k] /= (opt[2]>PINGTIMES ? PINGTIMES : opt[2]);
					SP("d_"); SPL(lvalue[k]);
					value[k] = lvalue[k];
#ifdef IOT1
					ThingSpeak.setField(3, value[k]);
#endif
					if (now() % SECS_PER_DAY < timeint)EELONGW(WLEV0H, lvalue[k]);
					k++;   //value are mm.
					weather[iw].rain1h = value[k - 1];

					n_sample++;
					long ora = hour() * 3600 + minute() * 60 + second();
					pool.levelFit(ora / 10, value[k - 1]);
					if (pin2[i] > 1) {
						//float sl,si; pool.levelGet(0, 1, &sl, &si);	//get 0h water level from least sq. fit
						long lev; EELONGR(WLEV0H, lev);
						SPS("0h_lev"); SPS(lev);							//get 0h water level from EEprom saved val.
						value[k++] = weather[iw].rain1h - lev;
					}

				}
				else if (sensorT[i] == 1)    //-------------------------DHT11other sensors
				{
#ifdef DHT11L       

					value[k++] = DHT.readTemperature();
					value[k++] = DHT.readHumidity();

#endif
				}
				else if (sensorT[i] == 2) { //------------------------Connect to Solar Panels KWh
					lvalue[k] = readSunKwh();

					byte count = 0;
					//------------------------------------------------repeat reading for wrong values up to 10 times____
					while (count < 10 && lvalue[k] <= 0)
					{
						count++;
						pulseLed(1000, 100, 3);

						lvalue[k] = readSunKwh();
					}
					//	if (now() % SECS_PER_DAY < timeint)EELONGW(SUN0H, lvalue[k]);//----write Solar KWH at 0h
					value[k] = lvalue[k];
#ifdef IOT1
					ThingSpeak.setField(1, lvalue[k]);
#endif		
					k++;
					weather[iw].water = value[k - 1];
				}
				else if (sensorT[i] == 3)//__________________ Weather data from Underground Station___________
				{
					float val[8];

					String Str = "";
					Str += OpName[WU_URL];
					SPL(OpName[0]);
					Str += name[i];
					Str += ".json";

					String nomi[10] = { "current_observation","             ","                ","            ","               ","             ","","","","" };
					for (byte ii = 1; ii < pin2[i] + 1; ii++)
						nomi[ii] = param[k + ii - 1];
					bool noData = true;

					if (APIweatherV(Str, nomi, pin2[i], val)) noData = false;
					if (noData || val[0] < now() - 10000) {  //_________________no readings__________
						pulseLed(1000, 200, 2);
						for (byte ii = 0; ii < pin2[i]; ii++)val[ii] = -1;
					}

					for (byte ii = 0; ii < pin2[i]; ii++)value[k++] = val[ii];
					if (nomi[2] == "solarradiation") {
						weather[iw].sunrad = val[1];
					}
					else {
						weather[iw].time = now();
						weather[iw].temp = val[1];
						weather[iw].humidity = val[2];
						weather[iw].rain = val[3];
						weather[iw].wind = val[4];
					}

				}//-------------------------------------ET0 calculation--------------------------------------------------
				else if (sensorT[i] == 4) {
					value[k++] = ET0_calc(0);				//ET from solar Radiation WU
					if (pin2[i] > 1)
						value[k++] = ET0_calc(1);				//ET from  solar panel KWh
#ifdef IOT1
					if (value[k - 1] >= 0)ThingSpeak.setField(3, value[k - 1]);
					else
						ThingSpeak.setField(2, value[k - 2]);
#endif		
					
					if (value[k - 1] > 0 || value[k - 2] > 0) {
						byte iwm1 = iw - 1;
						if (iw == 0)iwm1 = MAX_WEATHER_READ - 1;
						float ETval = value[k - 1];
						if (ETval <= 0.&&value[k - 2] > 0) ETval = value[k - 2];
						if (now() > weather[iwm1].time + timeint + 100)
							sumET0 += ETval * 10000 * (now() - weather[iwm1].time) / 3600;
						else						sumET0 += ETval * 10000 * timeint / 3600;
						//		Reset SumET0 in case of errors!!
						if (sumET0 > 60000. || sumET0 < 0)sumET0 = 0;
						EELONGW(SUMETPOS, sumET0);
					}
					//	if (pin2[i] > 2)				
					//		value[k++] = ET0_calc(3);				// pool level variation [mm]/h least square slope last 12 value						
					if (pin2[i] > 2) {
						value[k++] = sumET0 / 10000.;				//daily ET sum
						SPL(value[k - 1]);
					}

				}
				else if (sensorT[i] == 5) {
					int temp[10];
					byte nres = start_read_OneWire(pin1[i], temp, 1);
					for (byte j = 0; j < nres; j++)value[k++] = temp[j];
				}
				else if (sensorT[i] == 6) { value[k++] = analogRead(A0); }  //-------------analog read --------
				else if (sensorT[i] == 7) {  //-------------digital read --------
					long pinv = 0;
					for (byte i = 0; i < pin2[i]; i++) pinv+=pinv*2+digitalRead(i);
					lvalue[k++] = pinv;
				}
				else if (sensorT[i] == 8) {   //-------------pulse read --------
					value[k++] = npulses;
					if(pin2[i]>1)value[k++] = 1000. / pulsmils;
				}
			}
#ifdef OS_API
			//sensor data are passed to rule interpreter variable (<1 are 100000 multiplied)
			for (byte i = 0; i < k; i++) { Lvalue[i] = value[i]; if (abs(lvalue[i]) == 0)Lvalue[i] = value[i] * 100000; }
#endif	
			eeprom_write_byte((byte *)WEATHERPOS, iw);
			eeprom_write_block(&weather[iw], (void*)(WEATHERPOS + 1 + iw * sizeof(Weather)), sizeof(Weather));
#ifdef IOT
			//byte iotInd[8] = { 1,0,1,10,11,0,0,0 }; 
			for (byte iot = 0; iot < 8; iot++)
				ThingSpeak.setField(iot, value[IOTflag[iot]]);
			if (ThingSpeak.writeFields(IotChannel, IotAPIkey)) { SPL_D("written ThinkSpeak"); }
			else{ SPL_D("TinkSpeak failed");
		}
#endif
		return 1;
	}
	return 0;
}
#define SHORTFILE
	void recordSensors(long timeint) {
		byte k = 0;
#ifdef SHORTFILE
		if (time_sensor1 == START_INTERVAL)
		{
			
			for (byte i = 0; i < nsensors; i++) {
				if (i > 0)logfile.print(',');
				logfile.print(name[i]);
				SP(name[i]); SP('-');
				logfile.print(",");
				logfile.print(pin2[i]);
				for (byte j = 0; j < pin2[i]; j++) {
					logfile.print(',');
					logfile.print(param[k]);
					SP(param[k]);
					k++;
				}
			}
			logfile.print(" err_"); logfile.print(eecode);
			logfile.print("t."); logfile.print(ntimes);
			logfile.println();
			time_sensor1 = 0;
		}
#endif
		
	//if (millis() > time_sensor1)
	
		{
			logfile = SPIFFS.open("/logs.txt", "a+"); 
			logfile.seek(0, SeekEnd);
			time_sensor1 = millis() + timeint;
			logfile.print("t:");
			logfile.print(now());
			 k = 0;
			for (byte i = 0; i < nsensors; i++) {
				logfile.print(',');
#ifndef SHORTFILE
				logfile.print(name[i]);
				SP(name[i]); SP('-');
#endif
				logfile.print(':');
				logfile.print('{');
				for (byte j = 0; j < pin2[i]; j++) {
#ifndef SHORTFILE
					logfile.print(param[k]);
					SP(param[k]);
#endif
					logfile.print(':');
					SP(value[k]);

#ifdef SHORTFILE
					if(param[k][0]<='Z'||value[k]==0)
						logfile.print(int(value[k++]));
					else
#endif
						logfile.print(value[k++]);
					if(j!=pin2[i]-1)logfile.print(',');
				}
				SPL();
				logfile.print('}');
			}

		}
		
		logfile.println(',');
		logfile.close();
	}//read and record sensors values each time interval sec
	int readSensorsFromFile(long dtime,char buf []) {
		logfile = SPIFFS.open("/logs.txt", "r+");
		//logfile.seek(0, SeekSet);
		time_t time = now() - dtime;
		logfile.seek(-dtime/2, SeekEnd);
		SPL(logfile.read());
		//logfile.find("\n");
		logfile.setTimeout(500);
		if (!logfile.find("t:"))return -1;
		SP("+");
		int n=logfile.readBytesUntil(',', buf, 100);
		SP(n);
		buf[n] = 0;
		SPS(buf);
		bool found = true;
		while (atol(buf) < time &&logfile.available()) {

			found = logfile.find("t:");// OpName[TIME_STR].c_str());
			//found = logfile.findUntil("t:","\^d");
			SP("_");
			if (found) {
				n = logfile.readBytesUntil(',', buf, 100);
				buf[n] = 0;
				SPL(buf);
			}
			else return -1;


		}
		if (!logfile.available())return -1;
		//logfile.seek(-12, SeekCur);
		byte k = 0;
		SP("<");
		logfile.setTimeout(100);
		n = logfile.readBytesUntil('\n', buf, 300);
		buf[n] = 0;
		SPL(buf);
#ifdef SHORTFILE
		char * buffer = (char *)malloc(300);
		buffer[0] = 0;
		char * buff="";
		if (buf[0] != ':') 
			buff = strtok(buf, ":");
		for (byte i = 0; i < nsensors; i++) {
			buffer = strcat(buffer, buff);

			buffer = strcat(buffer, name[i].c_str());
			buffer = strcat(buffer, ":");
			SPL(buffer);
			if (buf[0] == ':'&&i==0)
				buff = strtok(buf, ":");
			else 
				buff = strtok(NULL, ":");

			for (byte j = 0; j <pin2[i]; j++) {

				//while (buff!=NULL) {
				SPL(buff);
				buffer = strcat(buffer, buff);

				buffer = strcat(buffer, param[k++].c_str());
				buffer = strcat(buffer, ":");
				SPL(buffer);
				if(j!=pin2[i]-1||i!=nsensors-1)
					buff = strtok(NULL, ":");
				else
					buff = strtok(NULL, ",");
			}
			if(i==nsensors-1)
			 {
				//-----------end record	
				 
				buffer = strcat(buffer, buff);
				//	buffer[strlen(buffer)] = 0;
			}
		}

		strcpy(buf , buffer);
#endif

		SPL(buf);
		 //logfile.seek(posi,SeekSet);
		 logfile.close();
		 return n;
	} //read sensors from file
	bool onSensor(byte n, const char* command) {

	}// command: < number && > number || < number

//private:
	byte ServerCall(char buff[], char command[], IPAddress jserver) {
		// buff			return string
		// command		string 
		// jserver		Ip address of server
		// return		0= no connection; n number of buff char

		const int httpPort = 80;
		WiFiClient client;
		if (!client.connect(jserver, 80))
		{
			client.stop();

			SP("connection failed "); SPL(jserver);
			return 0;
		}
		client.println(command);
		int i = 0;
		while (!client.available() && i<255) {
			i++; delay(50); Serial.print('.');
		}
		//		SPS_D("resp=");
		if (i == 255) {
			Serial.print("no resp"); client.stop(); return 0;
		}
		i = 0;
		while (client.available()) {
			buff[i++] = client.read();
			//SP(buff[i - 1]);
			delay(100);

		}

		buff[i] = 0;
		client.stop();
		SPL(buff);
		return i;
	}
	byte addr[8];
	byte start_read_OneWire(byte pin, int temp[], byte mode) {//mode=0 start,mode=1 read
		pinMode(pin, INPUT_PULLUP);
		static OneWire ds(pin);
		static  DallasTemperature sensors(&ds);
		static DeviceAddress dsAdd[4];
		static byte nDSsens = 0;
		SPL(mode);
		if (mode == 0) {
			sensors.begin();
			nDSsens = sensors.getDeviceCount(); 
			SP("DS_"); SPL(nDSsens);
			ds.reset_search();
			for (byte i = 0; i < nDSsens; i++)if (!ds.search(dsAdd[i]))SPL("OneWire not found");
		}
		else {
			sensors.requestTemperatures();
			for (byte i = 0; i < nDSsens; i++) {
				temp[i] = sensors.getTempC(dsAdd[i]) * 10;
				SPS(i); SPS(" T "); SPS(temp[i]);
			}
		}
		return nDSsens;
	}
	
	
	long readSunKwh() {
		char buff[30]; char * ii; int ival[4]; byte i = 1;
		IPAddress ip(192, 168, 1, 77);
		if (ServerCall(buff, "//", ip) > 3) {
			ival[0] = atoi(strtok(buff, " "));
			ii = strtok(NULL, " ");
			while (ii != NULL) { ival[i++] = atoi(ii); ii = strtok(NULL, " "); }
			if(i>2)
				return ival[2];
			else return -2;
		}
		else return -1;
	}
	int readDistance(int  trigPin, int echoPin,int temp) {

		unsigned long duration = 0, distance;
		delay(50);
		pinMode(trigPin, OUTPUT);
		SP("Send sound. pin   ");
		SP(trigPin);
	
		
		//-send trig out
		digitalWrite(trigPin, LOW);
		digitalWrite(trigPin, HIGH);
		delayMicroseconds(10);
		digitalWrite(trigPin, LOW);
	//	Serial.print("   .....Reading echo from ");
	//	Serial.print(echoPin);
		//-wait echo in
		pinMode(echoPin, INPUT);
		//wait echo for 0.
		if (digitalRead(echoPin) == HIGH) {
			SPL("error"); return -1;
		}
		//Serial.print(digitalRead(echoPin));
	   //	Serial.print( " ...   ");
		unsigned long startMill = millis();
		{ long kk = 0;
		while (digitalRead(echoPin) == LOW &&millis() < 1000UL + startMill) { yield(); kk++; }
			//Serial.print(kk);
			if (millis() >= startMill + 1000UL)
			{
				SPL("no echo"); return -2;
			}
		
			startMill = micros();
			while (digitalRead(echoPin) == HIGH) { delayMicroseconds(1); }

			if (micros() >= startMill + 1000000L) {
				SPL("too long eco"); return -3;
			}


			duration = micros() - startMill;
		}
		//distance = duration * 17 / 100 ;
		if(temp>0&&temp<500)
		   distance = duration*(331 + 0.6*temp) / 200;
		else
			distance = duration*(332) / 200;
		SP("Duration: ");
		SP(duration);
		SP(" micros   ");
		SP(distance);
		SPL(" cm");

		return distance;

	}
	

};

class Relai {
	byte pin; boolean status;
	char* name;
	void begin(byte pin) { pinMode(pin, OUTPUT); }
	void set(bool status) { digitalWrite(pin, status); }
};
Sensor my;


void handleRoot() {
//  digitalWrite(led, 1);
  server.send(200, "text/plain", "->http://<esp_ipaddr>/download(or dir)?file=/filename");
 // digitalWrite(led, 0);
}
void handleDel() {
	String str = "";
	 {
		String str = server.arg(0);
		if (SPIFFS.exists(server.arg(0))) {
			SPIFFS.remove(server.arg(0)); str+= " file deleted";
		}
		else str += " file not found";
		server.send(200, "text/plain", str+"\r\n");
	}
}
void handleDir(){
  String str="";
    {
	   
	  String dum = "/";//  (char*)server.arg(0);
    //memmove(dum+1,dum,strlen(dum));dum[0]='/';
    //strcat(dum,(const char*)server.arg(0));
	  Serial.println(dum);
    Dir dir = SPIFFS.openDir(dum);

	//server.send(200, "text/plain", "directory\n");
	//server.send(200, "text/plain", dum);

int i = 0;
String resp="";
while (dir.next()) {
	resp += dir.fileName();
	resp += dir.fileSize();
	resp += "\r\n";
	Serial.print(i++);
	//Serial.println(str);

}
   
     //   siz -= sizeof(buf) - 1;
        server.send(200, "text/plain", resp);
 
   // File f = dir.openFile("r");
   // server.send(100,"text/plain",f.size());

if (i == 0)server.send(200, "text/plain", "no files\r\n");

    }
  }
#define MAXSIZ 10000L
void handleDownload(){
 {
    String str = "";
	SP("Download "); SP(server.arg(0));
    File f = SPIFFS.open(server.arg(0), "r+");
    if (!f) {
      server.send(200,"text/plain","Can't open SPIFFS file "+server.arg(0)+" !\r\n");          
    }
	else {
		char buf[BUFFDIM];
		long siz = f.size();
		//long bytes = atol(server.arg(1).c_str());
		//if (bytes > 0) siz = bytes;
		if (siz > MAXSIZ) { f.seek(-MAXSIZ, SeekEnd); siz = MAXSIZ; }
		SP("size"); SPL(siz);
		while (siz > 0) {
			size_t len = siz;
			if (len > BUFFDIM)len = BUFFDIM;
			f.read((uint8_t *)buf, len);
			buf[len] = 0;
			str += buf;
			siz -= sizeof(buf) - 1;

		}
		f.close();
		
		server.send(200, "text/plain", str + "\r\n"); //text/plain
	}
  }
} 
void handleRestart() { ESP.restart(); }
void handleEdit()
{
	String str = "Edit old:";

	File f = SPIFFS.open(server.arg(0), "r+");
	if (!f) {
		server.send(200, "text/plain", "Can't open SPIFFS file " + server.arg(0) + " !\r\n");
	}
	else {

		int len = 200;
		char buf[2][200];
		long fend = f.size();
		SP("Edit "); SP(server.arg(0)); SPL(fend);
		long line = atol(server.arg(1).c_str());
		long fwrite;
		//	if (bytes > 0) siz = bytes;
		//	if (siz > MAXSIZ) { f.seek(-MAXSIZ, SeekEnd); siz = MAXSIZ; }
		int linecount = 0, recl;
		while (linecount++ < line) {
			fwrite = f.position();

			recl = f.readBytesUntil(13, buf[0], 200);
			if (recl < 0) {
				server.send(200, "text/plain", "Line" + server.arg(1) + "not found !\r\n");
				return;
			}

		}
		

		long fread = f.position();
		buf[0][recl] = 0;
		String lines = buf[0];
		if (server.argName(2) == "list") {
			server.send(200, "text/plain", lines + "\r\n");
			return;
		}
		str += lines;
		if (fend - fread < len)len = fend - fread;
		f.read((uint8_t *)buf[0], len);
		if (server.argName(2) != "del") {

			lines.replace(server.arg(2), server.arg(3));

			f.seek(fwrite, SeekSet);
			f.print(lines + "\r");
			server.send(200, "text/plain", str + "\r\n     new: " + lines + "\r\n"); //text/plain
			fwrite = f.position();

		}
		else server.send(200, "text/plain", str + " deleted \r\n"); //text/plain

		byte i = 1;
		byte lenw = len;
		while (fread+len < fend) {
			SPS(fread); SPS(fwrite);
			
			fread += len;
			if (fend - fread < len)len = fend - fread;
			f.seek(fread, SeekSet);
			
			f.read((uint8_t *)buf[i % 2], len);
			i++;
			f.seek(fwrite, SeekSet);
			f.write((uint8_t *)buf[i % 2], lenw);
			fwrite += lenw;
			lenw = len;
		}

		f.seek(fwrite, SeekSet);
		f.write((uint8_t *)buf[i+1 % 2], len);
		f.close();

	}
}
//- See more at: http://www.esp8266.com/viewtopic.php?f=32&t=4570#sthash.RjMFgw08.dpuf
void handleNotFound(){
 // digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
 // digitalWrite(led, 0);
}
#ifdef MULTISSID
String found_SSID[5], found_psw[5];
bool connNetwork() {
	
	uint8_t n = 0;
	bool result = false;
	uint8_t nNetwork = scanNetwork(0);
	if (nNetwork == 0)
	{
		SPL("no net"); ESP.restart();
	}
	while (n < nNetwork && !result) {
		ssid= found_SSID[n].c_str();
		password = found_psw[n].c_str();
		n++;
	//	if (PASSWORD[strlen(PASSWORD) - 1] < '0')PASSWORD[strlen(PASSWORD) - 1] = 0;

		 SPL(password);
		 WiFi.begin(ssid, password);
		 result = true;
		 byte trial = 0;
		 // Wait for connection
		 while (WiFi.status() != WL_CONNECTED&&trial<100) {
			 delay(500);
			 trial++;
			 Serial.print(".");
		 }
		 if (trial == 100)result= false;

	}
	if (result){
		Serial.println("");
		Serial.print("Connected to ");
		Serial.println(ssid);
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
	}

	return result;
}

byte scanNetwork(byte flag)
{
	byte netCount = 0;

#ifndef WIFIMANAGER
	Serial.println("scan start");

	// WiFi.scanNetworks will return the number of networks found
	int n = WiFi.scanNetworks();
	Serial.println("scan done");
	if (n == 0)
	{
		Serial.println("no networks found");
		return 0;
	}
	else
	{
String Ssid[10];
String psw[10];
File file;
int PaswKnown = -1;
int Npas = 0;
if (!SPIFFS.exists("SSID_PASSWORD"))
file = SPIFFS.open("SSID_PASSWORD", "w");               //new password file
else
{    //-----------------read SSID and PASSWORD from file.

	Serial.println("Reading password file....");
	file = SPIFFS.open("SSID_PASSWORD", "r+");               //read passwords from file
	while (file.available())
	{
		char buff[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
		file.readBytesUntil(',', buff, 20);

		Ssid[Npas] = buff;
		char buf[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

		int n = file.readBytesUntil('\n', buf, 20);
		buf[n - 1] = 0;
		psw[Npas++] = buf;
		Serial.print(Ssid[Npas - 1]);
		Serial.print('\t'); Serial.print("<>"); Serial.println(psw[Npas - 1]);
	}
}
Serial.print(n);

Serial.println(" networks found");
for (int i = 0; i < n; ++i)
{
	// Print SSID and RSSI for each network found
	Serial.print(i);
	Serial.print(": ");
	int jpas = Npas - 1;
	while (WiFi.SSID(i) != Ssid[jpas] && jpas >= 0)jpas--;
	if (jpas >= 0)PaswKnown = i;
	Serial.print(WiFi.SSID(i));
	Serial.print(" (");
	Serial.print(WiFi.RSSI(i));
	Serial.print(")");
	Serial.print((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");

	if (jpas >= 0) {
		Serial.println(" passw. available"); netCount++;
		found_SSID[netCount - 1] = Ssid[jpas];
		found_psw[netCount - 1] = psw[jpas];

	}

	else Serial.println();
	delay(10);
}

if (PaswKnown < 0 || flag == 1)
{
	Serial.println("Select network n.");
	while (!Serial.available() && millis() < 30000) delay(10);
	if (millis() > 30000)return 0;
	byte ch = Serial.read();
	Ssid[Npas] = WiFi.SSID(ch - '0');
	Serial.print("Enter password for "); Serial.println(Ssid[Npas]);
	while (!Serial.available()) delay(10);
	psw[Npas] = Serial.readString();
	Serial.print(Ssid[Npas]); Serial.print(','); Serial.println(psw[Npas]);
	file.print(Ssid[Npas]); file.print(','); file.println(psw[Npas]);
	file.close();
	netCount++;
}
	}
#	endif
	return netCount;
}
#endif
enum    OP {

	Time_interval_read,
	Time_interval_write,
	ping_times,						//times of ultrasonic distance measuremnt repetitons
	n4,
	POOL_FACTOR,					//pool area/exp.tank ratio
	IP1,							//ip address 
	IP2,							//
	IP3,							//
	IP4,							//
	MAX_WEATHER_S,				//dimension of weather struct for averages
	LINEAR_REGRESSION_DELTA_X,		//stored linear frequency sec.
	NSPLIT,							//n. of linear regression subtotatls
	LATITUDE,
	LONGITUDE,
	ELEVATION,
	PANEL_ANGLE,
	PANEL_AZIMUT,
	SOLAR_PANEL_FACTOR


};

#ifdef OS_API
//////////////////////////////// BITLASH ROUTINES///////////////////////////////////////////////////////////
long stackvalue[10];
long sun_rad(void) { };  //compute sun radiation from kW produced
long et_0(void) {		//arg0=name   return ET0 value *1000
};
byte factor[30];
long gval_f(void) {
	if (!isstringarg(1))return my.value[getarg(1)] *pow( 10 , factor[getarg(1)]);
	else 
		for (byte i = 0; i < my.nsensors; i++)
		  
			if (strcmp((char *)getstringarg(1), my.param[i].c_str())) return my.value[i+1] * pow(10 , factor[i]);

}
long sval_f(void) {
	if (!isstringarg(1))my.value[getarg(1)]  = getarg(2)/pow(10, factor[getarg(1)]);
	else {
		for (byte i = 0; i < my.nsensors; i++) if (strcmp((char *)getstringarg(1), my.param[i].c_str()))  my.value[i+1] = getarg(2) / pow(10, factor[i]);
	}
}
long getdistance(void) {// arg1=pin  //return Ultrasonic measured distance in mm 
	return my.readDistance(getarg(1), getarg(1),0);
};
long DHT_sensor(void) { // no arg  //return DHT sensor temperature in °C *10 tem,humidity to stack vector
	stackvalue[0] = DHT.readTemperature();
	stackvalue[1] = DHT.readHumidity();
	return stackvalue[0];
};

long One_Wire(void) {   // arg0=n.of one wire sensor   //return DHT sensor temperature in °C *10 
	int temp[10];
	byte nres = my.start_read_OneWire(getarg(1), temp, 1);
	for (byte j = 0; j < nres; j++)stackvalue[j] = temp[j];
	return stackvalue[getarg(1)];
};
long WU_getdata(void) { //arg0=name of station  //return weather station data to stack vector};
}

long api_com(void)
{
	String buffer = "";
	String comm= (char *)getstringarg(2);
	byte sta = getarg(1);
	for (byte i = 3; i < getarg(0)+1; i++)
		if (isstringarg(i ))
			buffer += (char *)getstringarg(i );
		else
		{
			char buff[10];
			sprintf(buff, "=%d", getarg(i ));
			buffer += buff;
		}
	return API_command(comm, buffer, sta);

}
long  API_get(void) {  //1/string varName/2/byte factor /3/byte ic station IP4/4/string Url:command /5/Url:val/6/url:command/7/url:val...)
	WiFiClient client;

	char json[1000];
	const int httpPort = 80;
	if (!client.connect(IPAddress(192,168,1,getarg(3)), 80))
	{
		client.stop();
		SP("connection failed ");// SPL(jsonserver);
		pulseLed(2000, 0, 1);
		return 0;
	}
	String url = "GET ";
	url += getstringarg(4);
	for (byte i = 5; i < getarg(0) + 1; i+=2) {
		url += getstringarg(i);
		char bu[10];
		sprintf(bu, "=%d&", getarg(i + 1));
		url += bu;
	}
	url	+= " HTTP/1.1\r\n Connection: close \r\n\r\n";
	client.print(url);
	SPL(url);
	//////////////////////////////////////////////////////////////////////////////////////
	int i = 0; bool ISJSON = false; byte ii = 0;
	//	Serial.println("Waiting Json");
	char c; 
	delay(500);
		// Read all the lines of the reply from server and print them to Serialbol 
		
#define MAX_SEC 30
			byte inc = 0;
			while (!client.available() && inc < MAX_SEC * 2) {
				inc++; delay(500);
			}
			if (inc == MAX_SEC*2) {
				SPL("no answer");
				client.stop(); return 0;
			}
		//_________________timestep is connection timeout_______________________________ }}} ____end of jsonstring
			while (client.available() && i < MAX_JSON_STRING) {
	
				c = client.read();
				if(c=='{')ISJSON = true;
				if (ISJSON) {
					json[i++] = c;
					//	SP(c);
				}
			}
		if (ISJSON) {
			json[i - 1] = 0;
			client.stop();
			SP(" Json read!"); SPL(i);
			float valo[2];
			String nome[1] = { (char*)getstringarg(1) };
			byte ret = my.JsonDecode( 2, json,nome, valo);
			long retnum= valo[1] * pow(10 , getarg(2));
			return retnum;

		}

		else SPL("no json");

	

	client.stop();
	//SP("mem.h."); SPL(ESP.getFreeHeap());
	return 0;
}
///--------------end of BITLASH routines---------------------------------------------------------------
struct stations  {       //incoming commands......... 1 for each internal valve or output relay operated
	bool Flag = 0;                    //status 0 or 1 closed or opened
	unsigned long Stop_time = 0;      //epoch time to stop
	byte out_pin = 0;    //pin 1--8 internal pin 9--256 external / ->/8= IP4 %8= sid;from 0 to 32  
};
#define EE_CONF_POS 1600
#define MAX_RULE_CHAR 800
#define EE_POS_RULES 1000
#define EEPROM_ST     900
#define MAX_OUT_CHAN 8
#define PIN_OUT_FREQ 1000
#define FIRST_PIN_OUT 32
byte statn = 0;
stations st[MAX_OUT_CHAN];
byte rulen = 0, rulepos = 0;
unsigned long next_out_mill = 0;

void handleLogRule() { GetRule(EE_CONF_POS);}
void handleGetRule() { GetRule(EE_POS_RULES); }

void GetRule(int EEpos) {
	String outstr = "";
	if (server.argName(0) == "add") {                 //--------------append one rule-----------
		int rulep = EEpos;// EE_POS_RULES;
		rulen = EEPROM.read(EEpos);// EE_POS_RULES);
		rulep++;
		for (byte i = 0; i < rulen; i++)
		{
			rulep += EEPROM.read(rulep) + 1;
		}

		char  rule[80];
		strcpy(rule, server.arg(0).c_str());
		rule[strlen(rule) ]=0 ;
		SPL_D(rule);
		rulen++;
		EEPROM.write(EEpos, rulen);// EE_POS_RULES, rulen);
		EEPROM.write(rulep++, strlen(rule));
		eeprom_write_block(&rule, (void *)rulep, strlen(rule));
//		rulepos += strlen(rule)+1;
		outstr += "added"; outstr += rule;
		SP_D("n.rules "); SPL_D(rulen);
		EEPROM.commit();
	}
	else if (server.argName(0) == "del"|| server.argName(0) == "rep") {
																//-----------replace and delete commands!!!!!
		int rulep = EEpos;// EE_POS_RULES;
		int rulepp; byte k = 0; char buff[600];
		rulen = EEPROM.read(rulep++);
		int recordN = atoi(server.arg(0).c_str());
		SP_D(server.argName(0)); SPL_D(recordN);
		rulepp = rulep;
		if (recordN < 0) {										//--------rep=-1 means run the rule now (only this time!)
			strcpy(buff, server.arg(1).c_str());
			SP("Run rule:"); SP(buff); SP("=");
			long val = espressione(buff); SPL(val);
			String replay = "{run:OK},{result:";
			replay += answer+"}\r\n";
			server.send(202, "text/plain", replay);
			return;
		}
		for (byte i = 0; i <= recordN; i++)
		{
			rulepp = rulep;
			byte rulelen = EEPROM.read(rulep);
			rulep += rulelen + 1;
			SPL_D(rulelen);
		}
		if (server.argName(0) == "rep") {
			byte rulelen = strlen(server.arg(1).c_str());
			EEPROM.write(rulepp++, rulelen);
			Serial.print("new string:");
			for (byte i = 0; i < rulelen; i++)
			{
				if (rulepp >= rulep)buff[k++] = EEPROM.read(rulepp);	//-------------new rule is longer store in buff extra char
				EEPROM.write(rulepp++, server.arg(1)[i]);
				Serial.print(server.arg(1)[i]);
			}
			Serial.println();
		}
		if (k > 0) {																		
				byte k0 = 0;											//---------copy from buff to EEPROM and append remaining char to buff
				for (byte i = recordN + 1; i < rulen; i++)
				{
					byte ruleng = buff[k0++];// EEPROM.read(rulep);
					buff[k++]= EEPROM.read(rulepp);
					EEPROM.write(rulepp++, ruleng);
					for (byte j = 0; j < ruleng; j++) {
						buff[k++] = EEPROM.read(rulepp);
						EEPROM.write(rulepp++, buff[k0++]);
						Serial.print(buff[k0-1]);
					}
					Serial.println();
					//rulepp += ruleng + 1;
					rulep += ruleng + 1;
				}

				for (byte i = k0; i < k; i++) { EEPROM.write(rulepp++, buff[i]); }   // need to empty buff buffer 
			}
		else
		for (byte i = recordN+1; i < rulen; i++)                          //----------copy only following rules
		{
			byte ruleng = EEPROM.read(rulep);
			EEPROM.write(rulepp++, ruleng);
			for (byte j = 0; j < ruleng; j++)EEPROM.write(rulepp++, EEPROM.read(rulep + j + 1));
			//rulepp +=ruleng+1;
			rulep  += ruleng + 1;
		}
		outstr += "deleted";
	if(server.argName(0) == "del")
		EEPROM.write(EEpos, rulen - 1);// EE_POS_RULES, rulen - 1);
		EEPROM.commit();
	}
	else if(server.argName(0) == "list") {
		int rulep = EEpos;// EE_POS_RULES;
			
		rulen = EEPROM.read(EEpos);// EE_POS_RULES);
			rulep++;
			SPL_D(rulen);
			for (byte i = 0; i < rulen; i++)
			{
				byte ruleng = EEPROM.read(rulep);
				SPL_D(ruleng);
				for (byte j = 0; j < ruleng; j++) {
					char c = EEPROM.read(rulep + 1 + j);
					if (c != 13 && c != 10)  outstr += c;// SP_D(c); 
				}
				//SPL_D();
				outstr += "\r\n";
				rulep += EEPROM.read(rulep) + 1;
			}
			SPL_D(outstr);
	}
	server.send(202, "text/plain", outstr + "\r\n");

}
void handleRunOnce() // receive API command for Digital IO ( sid=station n.&en=1-open_0-close&t=duration_in_seconds)
{
	//if (statn == MAX_OUT_CHAN)return;
	String outstr = "{result=0}";
	//n_out_pin = 4;
	SPL(n_out_pin);
	for (byte i = 0; i < n_out_pin; i++) {
		SPL(st[i].out_pin);
		if (st[i].out_pin == atoi(server.arg(1).c_str())) {
			//st[statn].out_pin = atoi(server.arg(1).c_str());
			st[i].Flag = atoi(server.arg(2).c_str());
			st[i].Stop_time = now() + atoi(server.arg(3).c_str());
			eeprom_write_block(&st[i], (void *)(EEPROM_ST + statn * sizeof(stations)), sizeof(stations)); //command is stored in EEPROM
			SP_D(st[i].out_pin); if (st[i].Flag) { SPL_D(" start"); }
			else { SPL_D(" stop"); }
			outstr[8] = 1;
			break;

		}
	}
	server.send(202, "text/plain", outstr + "\r\n");

}
void applyRemoteToValve() {                              // --------apply remote API command to digital IO
	if (millis() < next_out_mill)return;

	next_out_mill = millis() + PIN_OUT_FREQ;
	for (byte i = 0; i < n_out_pin; i++)
	{
		if (st[i].Stop_time != 0 && now() > st[i].Stop_time) {
			st[i].Flag = 0;
			st[i].Stop_time = 0;
			eeprom_write_block(&st[i], (void *)(EEPROM_ST + i * sizeof(stations)), sizeof(stations));
			SP_D(st[i].out_pin); SPL_D(" closed");
		}
	//	if (st[i].out_pin < 8)
			digitalWrite( st[i].out_pin, st[i].Flag);  //FIRST_PIN_OUT first pin connected to valve or relay

	}
}
	//conditional Rules to be stored in EEPROM-------------------------------------------------------------------
/*void API_repeat(long timeDel) {

	for (byte k = 0; k < nTry; k++)
		if (millis() >timeTry[k] && ntrial[k]<10) {
			String streamId = storedStream[k];
			String command = storedCommand[k];
			byte ic = ipStored[k] - 20;
			timeTry[k] = millis() + timeDel;
			ntrial[k]++;
			if (API_command(streamId, command, ic)) {
				SP_D(command); SPL_D(ic);
				for (byte j = k + 1; j < nTry; j++) {
					storedStream[j - 1] = storedStream[j];
					storedCommand[j - 1] = storedCommand[j];
					timeTry[j - 1] = timeTry[j];
					ipStored[j - 1] = ipStored[j];
					ntrial[j - 1] = ntrial[j];
				}
				k--;
				nTry--;
			}
		}
	return;
}*/
byte API_command(String streamId, String command, byte ic) {

	String privateKey = "a6d82bced638de3def1e9bbb4983225c"; //MD5 hashed
	const int httpPort = 80;
	WiFiClient client;
	IPAddress jsonserver = IPAddress(192, 168, 1, ic + 20);
	SP(jsonserver);
	if (!client.connect(jsonserver, 80))
	{
		client.stop();

		SPL(" connection failed ");
		return 0;
	}
	//	SP("Connect: "); SP(jsonserver);
	String url = streamId;
	url += "?pw=";
	url += privateKey;
	url += command;
	SP_D(" Requesting URL: ");
	SPL_D(url);
	// This will send the request to the server
	client.print(String("GET ") + url + " HTTP/1.1\r\n" +
		"Host: " + "192.168.1.20" + "\r\n" +
		"Connection: close\r\n\r\n");
	byte count = 0; bool res = false;
	while (!client.available() && count<240) { delay(100); count++; }
	if (count == 240)return 0;
	while (client.available()) {
		char c = client.read();
		SP_D(c);
		if (c == ':'&&client.available()) {
			c = client.read();
			SP_D(c);
			if (c == '1')res = true;
		}
	}
	//if (res)if (client.read() != '1'&&client.read()!='1')res = false;
	client.stop();
	return res;
}

	byte nespr = 0; byte esp_old[10] ;                             //number of conditional espressions
	unsigned long ruleCheck=120000;
																   //char * express[10]; byte channel[10]; STORED IN EEPROM                     //conditional expressions and channel

	void Rule_Apply() {
	//	API_repeat(10000);
		if (millis() < ruleCheck)return;
		SP("Apply ");
		ruleCheck = millis() + 60000;                 //---------------apply rules aevery minute------------
		int rulepos = EE_POS_RULES;
			byte	  rulen = eeprom_read_byte((byte *)rulepos++);
			if (rulen > 250) {
				rulen = 0; eeprom_write_byte((byte *)(rulepos - 1), 0);
			}
			SP(rulen); SPL(" rules");
		for (byte i = 0; i < rulen; i++) {
			byte ruleLen = eeprom_read_byte((byte *)rulepos++);         // --------------read rules from EEprom
		//	byte channel = eeprom_read_byte((byte *)rulepos++);
			char rule[80];
			eeprom_read_block(&rule,(void *) rulepos, ruleLen);
			rule[ruleLen] = 0;
			rulepos += ruleLen;
			SP_D(rule);
			long esp = espressione(rule);
			SP_D("="); SPL_D(esp);
			//-------------INTERPRET THE RULE-----------
		/*	if (esp != esp_old[i]) {                   // ---------------- rule result is cahnged?-------------------
				if (channel < 48)
				{
					digitalWrite(channel, esp);        //---------------apply to digital IO
					esp_old[i] = esp;
				}
				else
					if (remoteSet(channel, esp, 0))    //---------------apply to remote Station
						esp_old[i] = esp;
			}
			*/
		}

	}

	byte remoteSet(byte chan, byte val, time_t timer) {
		byte ic = (chan - 48) / 16;
		byte ch = (chan - 48) % 16;
		char buff[30];
		sprintf(buff,"&sid=%d,&en=%d,&t=%d", ch, val, timer);
		API_command("/cm", buff, ic);
		return 0;
	}
#endif

void EEPROMk(byte ind){
	
	EEPROM.write(0, ind);
	byte b = ntimes;
	if (ind == eecode)b++;
	else b = 0;
	EEPROM.write(1, b);
	EEPROM.commit();
}
void restart(byte ind){
	EEPROMk(ind);
	ESP.restart();

}
void handleReset() {
	EEPROM.write(0, 0);
	EEPROM.write(1, 0);
#ifdef OS_API
	EEPROM.write(EE_POS_RULES, 0);
#endif
	EEPROM.write(EE_CONF_POS, 255);
	EEPROM.commit();
	restart(0);
}

int EEpoint = EE_CONF_POS;   //pointer to EEPROM configuration data;
byte readBytesFromEEprom(char buff[], int maxbuf) {
	byte buffLen=EEPROM.read(EEpoint++);
	for (byte i = 0; i < buffLen; i++)if (i < maxbuf)buff[i] = EEPROM.read(EEpoint++); else EEPROM.read(EEpoint++);
	buff[buffLen] = 0;
	return buffLen;
}
WiFiUDP Udp;

void setup(void) {
	// eecode :
	//		if			 exec
	//	0	past setup			
	//	2   no SPIFFS	
	//	3   OK SPIFFS	no read config.txt
	//	4	no conn		no fix IP
	//	5	no NPT
	//	6	no logfile
	
	pinMode(led, OUTPUT);
	digitalWrite(led, 0);
	EEPROM.begin(2024);
	EELONGR(SUMETPOS, my.sumET0);
	eecode = EEPROM.read(0);
	ntimes = EEPROM.read(1);
	if (EEPROM.read(EE_POS_RULES) > 40) EEPROM.write(EE_POS_RULES, 0);     // if wrong reset n. of rules to 0
	if(digitalRead(0)==0)EEPROM.write(EE_CONF_POS, 255);
	Serial.begin(115200);
	Serial.println(EEPROM.read(0), DEC);
#ifdef OS_API
	vinit();
	addBitlashFunction("apicom", (bitlash_function)api_com);		//send a URL command (string1,string2,n1,string3,n2.....)
	addBitlashFunction("et0", (bitlash_function)et_0);				//get current ET0      ?????????????????
	addBitlashFunction("distance", (bitlash_function)getdistance);	//get SR 04 sensor distance reading on (pin)
	addBitlashFunction("dht_s", (bitlash_function)DHT_sensor);		//get DHT011 sensor values temp.°C*10 
	addBitlashFunction("onew_s", (bitlash_function)One_Wire);		//get onewire sensors readings (sensor n.)
	addBitlashFunction("wu_data", (bitlash_function)WU_getdata);	//get weather underground station data  ???????????
	addBitlashFunction("apiget", (bitlash_function)API_get);		//get value of remote station var from Json API query
	addBitlashFunction("sval", (bitlash_function)sval_f);			//set value of my.sensor variable ( number or name)*factor
	addBitlashFunction("gval", (bitlash_function)gval_f);           //get value of my.sensor variable (number or name)*factor

#endif
	Serial.println(EEPROM.read(1), DEC);

	if (!SPIFFS.begin()) {
		Serial.println("SPIFFS failed to mount !\r\n");
		pulseLed(10000, 100, 2);
		restart(2);
	}


	EEPROMk( 3);
#ifdef OS_API
	for (byte i = 0; i<MAX_OUT_CHAN; i++)
		eeprom_read_block(&st[i], (void *)(EEPROM_ST + i * sizeof(stations)), sizeof(stations));
#endif
	String parametri[10];
	
	byte formati[10];
	//opt[8] = 31;
#define EElogRule
	#ifdef READCONF

	char buf[150];
	if((eecode == 3 && EEPROM.read(1)>10))EEPROM.write(EE_CONF_POS, 255);

		if( EEPROM.read(EE_CONF_POS) == 255 && SPIFFS.exists("/config.txt"))
		{
			File cfile = SPIFFS.open("/config.txt", "r");
			// -----------------read from file and write EEPROM-------------------------------
#ifdef EElogRule
			byte k = 0;
			SPL("Rewriting EEprom Config Memory.....")
				EEpoint = EE_CONF_POS + 1;
			while (cfile.available()) { byte n = cfile.readBytesUntil(13, buf, 150); buf[n] = 0; if (!(buf[1] == '/')) { k++; SPL(buf); EEPROM.write(EEpoint++, n); for (byte i = 0; i < n; i++)EEPROM.write(EEpoint++, buf[i]); } };
			EEPROM.write(EE_CONF_POS, k);
			EEPROM.commit();
		}
	//	__________________________________________read from EEPROM______________________________
		{
#else
		//cfile.find(13);
	//cfile.seek(0, SeekSet);
 
#endif

		//second line may  contain comments	
		byte n;

#ifndef EElogRule
		buf[0] = '/';
		Serial.println("Options:");
		while (buf[0] == '/')buf[cfile.readBytesUntil(13, buf, 100)] = 0;  // comment line or   line with option interger values
		Serial.println(buf);                      //decode options
#else
		EEpoint = EE_CONF_POS + 1;
		n = readBytesFromEEprom(buf, 100);// __________________first line with n.of opt and int opt[] values
		buf[n] = 0; SPL(buf);
#endif


		byte n_opt = atoi(strtok(buf, ","));
		for (byte i = 0; i < n_opt; i++) {
			opt[i] = atoi(strtok(NULL, ",\n"));
			Serial.println(opt[i]);
		}
		Serial.println("OpNames:");
#ifndef EElogRule
		buf[0] = '/';
		while (buf[0] == '/')n = cfile.readBytesUntil(13, buf, 100);
		buf[n] = 0;  //thrird line options names
		Serial.println(buf);
#else
		n = readBytesFromEEprom(buf, 100);  //______________________second line with n.of.Opnames  and char* OpNames[] string values
		buf[n] = 0; SPL(buf);
#endif
		byte n_optn = atoi(strtok(buf, ","));

		for (byte i = 0; i < n_optn; i++) {
			char * nam = strtok(NULL, ",");
			OpName[i] = "";
			OpName[i] += nam;
			Serial.println(OpName[i]);
		}
		byte k = 0;
#ifndef EElogRule
		while (cfile.available()>6) {			//following line contain one type(1st v) of (3rd v)sensors :name{sensor1:    ,sensor2:     ,     ,} connected to (2nd v.)
			byte lbuf;
			int fileleft = cfile.available();

			buf[0] = '/';
			
			while (buf[0] == '/') {
				Serial.print("Reading:"); lbuf = cfile.readBytesUntil(13, buf,fileleft< 100?fileleft:100); Serial.println(lbuf);
			}
#else
		//read all reacord n=total - 2 header records
		byte nrec = EEPROM.read(EE_CONF_POS);
		for (byte k=0;k<nrec-2;k++)
		{
			byte lbuf = readBytesFromEEprom(buf, 100);  //___________________________following lines contain logrule commands values
#endif
			buf[lbuf] = 0;
			if (lbuf > 6) {
				Serial.println(buf);
				char * point = strtok(buf, ",");
				if (point != NULL) {
					Serial.println(point);
					byte p1 = atoi(point);
					point = strtok(NULL, ",");
					if (point != NULL) {
						Serial.println(point);
						byte p2 = atoi(point);
						point = strtok(NULL, ",");
						if (point != NULL) {
							Serial.println(point);
							byte p3 = atoi(point);

							char  nome[20];
							strcpy(nome, strtok(NULL, ","));
							Serial.print(p1); Serial.print(' '); Serial.print(p2); Serial.print(' '); Serial.print(p3); Serial.print(' '); Serial.print(nome); Serial.print(' ');

							for (byte i = 0; i < p3 - 1; i++) { parametri[i] = strtok(NULL, ","); Serial.println(parametri[i]); Serial.print(' '); }
							parametri[p3 - 1] = strtok(NULL, ",\n"); Serial.print(parametri[p3 - 1]);
							char * remain = strtok(NULL, ",\n"); byte kk = my.nsensors;
							while (remain != NULL) { factor[kk++] = atoi(remain); remain = strtok(NULL, ",\n"); }
							if (p1 < 10)
								my.beginSensor(p1, p2, p3, nome, parametri);

							else if (p1 == 10) // output pin definition line  10,0,n_out_pin,X=nome pin x,Y=nome pin Y,...
							{
#ifdef OS_API

								n_out_pin = p3;
								Serial.println(n_out_pin);
								for (byte i = 0; i < p3; i++) {
									st[i].out_pin = atoi(parametri[i].c_str());
									/*	if (st[i].out_pin < 10) {
											char* command = "+pinmode(1,1)";
											command[10] = '0' + st[i].out_pin;
											espressione(command);
										}*/
									pinMode(st[i].out_pin, 1);
									Serial.print("out_pin"); Serial.println(st[i].out_pin);
								}
#endif
							}
#ifdef IOT
								//if not comment first line contain ThingsSpeak channel,APIkey,index[0-7](value sequence n. on logs file)

								//cfile.readBytesUntil(13, buf, 150);
							else if (p1 >= 20) {
								IotChannel = p1;
								IotAPIkey = nome;
								for(byte j=0;j<p3;j++)IOTflag[j]= atoi(parametri[j].c_str());
								SP_D("IOT:"); SPL_D(nome);
							}
								/*if (buf[0] != '/') {
							
									IotAPIkey = strtok(buf, ",");
									SPL_D(buf);
									IotChannel = atol(strtok(NULL, ","));
									byte kk = 0;
									IOTflag[kk++] = atoi(strtok(NULL, ","));
									char * strtr = strtok(NULL, ",\n");
									while (strtr != NULL) {
										IOTflag[kk++] = atoi(strtr);
										strtr = strtok(NULL, ",\n\r");
									}
								}*/
#endif
							

						}
					}
				}
			}
		}
		Serial.println("CONFIG DONE ");
#ifndef		EElogRule
		cfile.close();
#endif
	}
/*	else
	{
		Serial.print("C_nf ");
		//const char* parametri[10];
		parametri[0] = "Kwh";
		my.beginSensor(2, 0, 1, "SunPan", parametri);
		// ultrasonic sensor on GPIO 12
		parametri[0] = "Dist";
		parametri[1] = "dayLevC";

		my.beginSensor(0, 10 * 16 + 10, 2, "wlev", parametri);
		parametri[0] = "temp";
		parametri[1] = "humidity";
		//my.beginSensor(1, 10, 2, "hous",parametri);
		parametri[0] = "local_epoch";
		parametri[1] = "temp_c";
		parametri[2] = "relative_humidity";
		parametri[4] = "wind_kph";
		parametri[3] = "precip_today_metric";
		my.beginSensor(3, 0, 5, "pws:ISAVONAL1", parametri);
		parametri[0] = "local_epoch";
		parametri[1] = "solarradiation";
		my.beginSensor(3, 0, 2, "SAVONA", parametri);
		parametri[0] = "sunrad";
		parametri[1] = "sol_pan";
		parametri[2] = "dayETsum";
		my.beginSensor(4, 0, 3, "ET0", parametri);
	}*/
#endif
	for (byte i = 0; i < 20; i++)Serial.println(opt[i]);

	sta = { opt[LATITUDE] / 10000.   ,
			opt[LONGITUDE] / 10000.   ,
			opt[ELEVATION]             ,
			opt[PANEL_ANGLE] * PI / 180.,	
			opt[PANEL_AZIMUT] * PI / 180 };
	Serial.print("WIFI CONN:");
//	delay(5000);
//	WiFi.mode(WIFI_STA);
//	delay(5000);
	Serial.println("Connecting to:");
	Serial.println(OpName[SSID].c_str()); 
	Serial.print("PSW:");
	Serial.println(OpName[PSW].c_str());
	WiFi.begin( OpName[SSID].c_str(), OpName[PSW].c_str());
//	delay(1000);
//	SPL(" conn");
	byte count = 0;
	// Wait for connection .5 sec pulses
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print("."); pulseLed(100, 10, 1);;
		if (count++ >= 120) { pulseLed(10000, 0, 1); restart(4); }
	}


	
#ifdef FIX_IP
	
	if (opt[IP1] > 0) {
		if (!(eecode == 4&&ntimes>10)) {
			Serial.println("Fip "); Serial.println(opt[IP1]); Serial.println(opt[IP2]); Serial.println(opt[IP3]); Serial.println(opt[IP4]);

			WiFi.config(IPAddress(opt[IP1], opt[IP2], opt[IP3], opt[IP4]), IPAddress(192, 168, 1, 1),
				IPAddress(255, 255, 255, 0));//, IPAddress( 192,168,1,1 ));
		}
	}
#endif
	
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(OpName[SSID]);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	Serial.println(WiFi.gatewayIP());
	Serial.println(WiFi.dnsIP());
//	udp.begin(8888);
	server.onNotFound(handleNotFound);
	server.begin();
	Serial.println("HTTP server started");

	IPAddress GateIP(192, 168, 1, 1);
	byte ntry = 0;
	while (!Ping.ping(GateIP)) {
		pulseLed(100, 100, 1); ntry++;
		if (ntry > 100){pulseLed(10000, 0, 1); restart(4);
	}
}
	SP("pingOK"); SPL(ntry);

	setSyncInterval(3600);
	SPL("TimeSync...");
	if (SyncNPT(1))Serial.println("NPT time sync"); 
	if(now()<1000000000UL) {
		SPL("No time sync.....Restart!")
		pulseLed(5000, 1000, 2); restart(5);
	}
  DateTime ora = now();
  SP(ora.hour()); SP(":"); SPL(ora.minute());
  if (MDNS.begin(OpName[ESP_STATION].c_str())) {
    Serial.println("MDNS responder started");
  }
 	  if (SPIFFS.exists("/logs.txt"))
		  logfile = SPIFFS.open("/logs.txt", "r+");
	  else
		  logfile = SPIFFS.open("/logs.txt", "w+");

	  if (!logfile) {
		  Serial.println("Cannot open logFile"); pulseLed(5000, 1000, 3); restart(6);
	  }
	  //	 while (logfile.available())Serial.print((char)logfile.read());
	 // logfile.seek(0, SeekEnd);
	  logfile.close();
  
#ifdef OTA
  ArduinoOTA.onStart([]() {
	  Serial.println("Start_Ota");
  });
  ArduinoOTA.onEnd([]() {
	  Serial.println("\nEnd");
	  ESP.restart();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {//SP(".");
	  { Serial.print(" Progress:% "); Serial.println((progress / (total / 100))); }
  });
  ArduinoOTA.onError([](ota_error_t error) {
	  Serial.print("Error : "); Serial.println(error);
	  if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
	  else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
	  else if (error == OTA_CONNECT_ERROR)Serial.println("Connect Failed");
	  else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
	  else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.setHostname("EspSensors");

  ArduinoOTA.begin();
#endif
#ifdef IOT
  static WiFiClient IOTclient;
  static int status = WL_IDLE_STATUS;
 // unsigned long myChannelNumber = 31461;

  ThingSpeak.begin(IOTclient);
//bool IOTwrite(float value,byte field){ ThingSpeak.setField(field, value); }
//ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
#endif

#ifdef FTP
  ftpSrv.begin("esp8266", "esp8266");    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
#endif
//-------------------------OpenSprinkler API----------------------------
//#define OS_API
  
#ifdef OS_API
 // for(byte i=0;i<MAX_OUT_CHAN;i++)
 // eeprom_read_block(&st[i], (void *)(EEPROM_ST + i * sizeof(stations)), sizeof(stations));
  server.on("/cm", handleRunOnce);
  //server.on("/cr", handleProgramRun);
  server.on("/rule", handleGetRule);
  server.on("/logrule", handleLogRule);
#endif
  server.on("/rst", handleReset);
  server.on("/", handleRoot);
 server.on("/download",handleDownload);
 server.on("/dir",handleDir);
 server.on("/del", handleDel);
 server.on("/edit", handleEdit);
 server.on("/restart", handleRestart);
 server.on("/sensor", handleSensorsGet);
 server.on("/level", handleLevelTrend);
 server.on("/inline", [](){
	 
	 server.send(200, "text/plain", "this works as well");
  });

#ifdef TELNET
  Tserver.begin();
#endif
  eeprom_read_block(&pool, (void *)(MYPOS), sizeof(pool));
 



  
  //------------------------------read weather from EEPROM



  iw=eeprom_read_byte((byte *)WEATHERPOS);
  if (iw < MAX_WEATHER_READ) {
	  SP("EEread weather"); SPL(iw);
	  for (byte j = 0; j < MAX_WEATHER_READ; j++) {
		  eeprom_read_block(&weather[j], (void *)(WEATHERPOS + 1 + j * sizeof(Weather)), sizeof(Weather));
		  SP(weather[j].time); SP('\t'); SP(weather[j].temp); SP('\t');
		  SP(weather[j].humidity); SP('\t'); SP(weather[j].wind); SP('\t');
		  SP(weather[j].sunrad); SP('\t'); SP(weather[j].water); SP(' '); SPL(weather[j].rain1h);

	  }
  }
  else iw = 0;
  EEPROMk( 0);
  my.time_sensor = START_INTERVAL;
  my.time_sensor1 = START_INTERVAL;
}
int RecordInterval = 1;
void handleLevelTrend() {
	String rep = "";
	char buff[80];
	byte step= atoi(server.arg(0).c_str());
	for (byte i = 0; i < 24-step; i++) {
		float slope, inter;
		if(pool.levelGet(i, i + step, &slope, &inter)>0){
			sprintf(buff, "%d slope %d  intercepts %d %d \r\n", i, int(slope*3600), int(inter+slope*i*360),int( inter + slope*(i+step) * 360));
			rep += buff;
		}

	}
	server.send(200, "text/plain", rep);

}
void handleSensorsGet() {
	long timedelay = atol(server.arg(0).c_str());
	char buf[300];
	SPL(now()-timedelay);
	String reply = "";
	sprintf(buf, "%d:%d", hour( now() - timedelay),minute(now() - timedelay));
	reply += buf;
	SP(buf); SPS(reply);
	if(my.readSensorsFromFile(timedelay, buf) == -1) {
		SPL("not found");
		server.send(200, "text/plain",reply+ " not found! \r\n");
		return;
	}
	reply += buf;
	SP("<");
	SP(reply); SPL(">");
	server.send(200, "text/plain",reply+"\r\n");

}
void handleSensorsContr() {

	byte nsensor = atoi(server.arg(0).c_str());
	bool state = my.onSensor(nsensor, server.arg(1).c_str()  );  //pump,on=   "pool.dist>50&&temp>0", off=   "pool.dist<10"
	if (state) digitalWrite(atoi(server.arg(2).c_str()),state);
}
void loop(void) {
#ifdef FTP
	ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
#endif
#ifdef TELNET
	getClients();
#endif
#ifdef OTA
	ArduinoOTA.handle();
#endif
#ifdef PROVASITO
	static ulong mymillis = 0;
	if (millis() > mymillis) { my.prova(); mymillis = millis() + 60000; }
#endif
	if (my.readSensors(opt[RecordInterval])) {
		eeprom_write_block(&pool, (void*)MYPOS, sizeof(pool));
		my.recordSensors(opt[RecordInterval]);
	}
#ifdef OS_API
	Rule_Apply();
	applyRemoteToValve();
#endif
  server.handleClient();
}
