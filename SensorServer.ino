#include "DHT11lib.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <SSIDPASSWORD.h>
#include <ArduinoOTA.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>
#include "NPTtimeSync.h"
#include <ArduinoJson.h>
#include "ET_penmam.h"
#include "Eeprom_ESP.h"
#include <ESP8266OneWire.h>
#include <DallasTemperature.h>
#include <Arduino-Ping-master\ESP8266ping.h>
#define TELNET

#ifdef TELNET
WiFiServer Tserver(23);
WiFiClient Tclient;

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

#define DHT11L

#ifdef DHT11L
//#include <SparkFun_RHT03_Particle_Library-master\firmware\SparkFunRHT03.h>
//#include <Arduino-DHT22-master\DHT22.h>
//#include "DHT11lib.h"
#include <DHT-sensor-library-master\DHT.h>
DHT DHT(10,DHT22);
#endif
//const char* ssid = "........";
//const char* password = "........";
#define OTA
ESP8266WebServer server(80);
#define BUFFDIM 2024
const int led = 16; //.....13 for UNo..... 16 for ModeMCU

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

#define MAX_WEATHER_READ 12
Weather weather[MAX_WEATHER_READ]; byte iw = 0;
#define ET0
#define SUNPANELFACTOR 0.36
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
Geo sta={ 44.12474,8.25445,23, 30. / 180. * PI, 12. / 180. * PI };
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
		if (weather[i].wind > 0) {
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
					SPS("_"); SPS(weather[i].time - timep); SPS(solar_panels_watts); SPS("_");
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
class Sensor {
public:
	

	byte nsensors = 0, sensorT[10], pin1[10], pin2[10],kk=0;
	float value[30];
	unsigned long time_sensor = 0, time_sensor1 = 0;
	String  name[10];
	String  param[20];
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
		if (type == 5)if (start_read_OneWire(pin, temp, 0) < n_par) SPL("not found Temp sensor");

	} //setup sensors (type,byte pin);
	  //--------------------------------------------------------------------------
#define MIL_JSON_ANS 60000
#define MAX_JSON_STRING 2500

	byte APIweatherV(String streamId,String nomi[], byte Nval, float val[]) {
		WiFiClient client;
		char json[2600];
		
	
		if (client.connected()) client.stop();
		
		
		SPL("connecting to WU");
	
		const int httpPort = 80;
		if (!client.connect("api.wunderground.com", 80))
	//	if (!client.connect(IPAddress(192,168,1,77), 80))
		{
			client.stop();

			SP("connection failed ");// SPL(jsonserver);
			pulseLed(2000, 0, 1);
			return 0;
		}
	    SP("mem.h."); SPL(ESP.getFreeHeap());
		//	SP("Connect: "); SP(jsonserver);
		String url = "GET ";
		//String url = "/api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json";
		//url += "?pw=";
		//url += privateKey;
		url = url + streamId
			+" HTTP/1.1\r\n" +"Host: " + "api.wunderground.com" + "\r\n" +"Connection: close \r\n\r\n";
		//url += "&value=";
		//url += value;
		//	client.flush();
		Serial.print(" Requesting URL: ");
		Serial.print(url);
			client.print(url);
		// This will send the request to the server
		//client.print(String("GET ") + streamId + " HTTP/1.1\r\n" +
		//	"Host: " + "api.wunderground.com" + "\r\n" +
		//	"Connection: close\r\n\r\n");

		int i = 0, json_try = 0; bool ISJSON = false; byte ii = 0;
		Serial.println("Waiting Json");
		long time_step = millis() + MIL_JSON_ANS;
		char c, cp, cpp; bool obs = false;
		delay(500);
		while (millis() < time_step)
		{
			// Read all the lines of the reply from server and print them to Serialbol 
			while (millis() < time_step - MIL_JSON_ANS / 2 && !obs)
			{
			
				int nby = client.available();
				if (nby) {
#ifdef VERIFY_WU_ANSWER
					Serial.print(client.read());
#else
					//	obs = client.findUntil(nomi[0].c_str(), "}}}");
					obs = client.findUntil("current_observation", "}}}");
#endif
					//				if (obs)break;
					//				else { SPL_D("error"); client.stop(); return 0; }
				}
				else
				{
					Serial.print(nby); Serial.print('-'); pulseLed(nby+40, 50, 1);
				}
			}
			
			if (!obs) {
				SPL("error");
				while (client.available()) Serial.print(client.read());
				pulseLed(1000, 0, 1);
				client.stop(); return 0;
			}
	
			while (millis() < time_step&&c != '}'&&cp != '}'&&cpp != '}')

				while (obs&&client.available() && i < MAX_JSON_STRING) {
					cpp = cp;
					cp = c;
					c = client.read();
				
					//	buff[ii++] = c;
					//if (cpp == 'r'&&cp=='e'&&cpp=='s'){
					if (c == '{')
						ISJSON = true;
					
					//	json[i++] = cpp; json[i++] = cp;}
					if (ISJSON) { json[i++] = c; 
					//Serial.print(c);
					}
				}
			//Serial.println("endwhile");
			if (ISJSON) {
				json[i - 1] = 0;
				client.stop();

				Serial.print("Connected ! "); //SPL(jsonserver);
									  //SP_D(json);
				Serial.print(" Json read!"); Serial.println(i);
				Serial.print("m.b.h."); Serial.println(ESP.getFreeHeap());
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
					for (byte i = 1; i < Nval+1 ; i++) {
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
				//va_end(args);
			}

			else SPL("no json");

		}
#endif
		client.stop();
		SP("mem.h."); SPL(ESP.getFreeHeap());
		return 0;
	}
	long lvalue[20], sumET0 = 0;
#define WEATHERPOS 10		//eeprom pos for weather[i]
#define MYPOS 500			//eeprom pos for pool[24] 
#define SUMETPOS 6			//eeprom pos for total dayly ET0
#define WLEV0H 2			//"		"" for water level at 0:0 h
#define ERROR_P 0				//"		"	for errors
#define PINGTIMES 10
#define EELONGW(x, y) { eeprom_write_block(&y,(void *)x,4);}
#define EELONGR(x, y) {eeprom_read_block(&y,(void*)x,4); }
#define POOLFACTOR 12;		// surface of swimming pool area 96mq / area of expansion tank 8mq

//_____________________________________________________________________________________________________________________
	bool readSensors(long timeint) {//read and record sensors values each time interval sec
		
		if (millis() > time_sensor) {
//          0h first time of the day
			if (now() % SECS_PER_DAY < timeint) {
				sumET0 = 0; 
				pool.levelBegin(); //sumY = 0; sumXY = 0; sumX = 0; sumX2 = 0; n_sample = 0;
			}

			byte ntry = 0;
			while (!Ping.ping(IPAddress(192, 168, 1, 1))) {
				pulseLed(100, 100, 1); ntry++;
			}
			SP("pingOK"); SPL(ntry);
			byte k = 0;
			time_sensor = millis() + timeint * 1000;
			iw++; if (iw >= MAX_WEATHER_READ)iw = 0;
			for (byte i = 0; i < nsensors; i++) {
				if (sensorT[i] == 0)// Ultrasonic Distance sensor
				{
					int readings[PINGTIMES], minv = 0, maxv, it = 0;
					byte rept = 0;
					while(rept < 20&&it<PINGTIMES ) {
						readings[rept] = readDistance(pin1[i] / 16, pin1[i] % 16,weather[iw].temp);//distance 0..1 mm
						if (readings[rept] > 0) {
							minv += readings[rept]; it++;
						}
						rept++;
					}
					if (it > 0) {
						lvalue[k] = 0;
						maxv = minv*1.1 / it; minv = minv*0.9 / it; it = 0;
						for (byte rept = 0; rept < PINGTIMES; rept++)
							if (readings[rept] > minv&&readings[rept] < maxv) {
								lvalue[k] += readings[rept]; it++;
							}
						if (it > 0)lvalue[k] /= it;
					}
					else lvalue[k] = -10;
					lvalue[k] /= 10;
					if (now() % SECS_PER_DAY < timeint)EELONGW(WLEV0H, lvalue[k]);
					value[k] = lvalue[k]; k++;   //value are mm.
					weather[iw].rain1h = value[k - 1];
					
					n_sample++;
					long ora= hour() * 3600 + minute() * 60+second();
					pool.levelFit(ora/10, value[k-1]);
					if (pin2[i] > 1) {
						//float sl,si; pool.levelGet(0, 1, &sl, &si);	//get 0h water level from least sq. fit
						long lev; EELONGR(WLEV0H, lev);
						SPS("0h_lev"); SPS(lev);							//get 0h water level from EEprom saved val.
						value[k++] = weather[iw].rain1h - lev;
					}
	
				}
				else if (sensorT[i] == 1) //-------------------------DHT11other sensors
				{
#ifdef DHT11L       
	
					value[k++] = DHT.readTemperature();
					value[k++] = DHT.readHumidity();
		
#endif
				}
				else if (sensorT[i] == 2) {//------------------------Connect to Solar Panels KWh
					lvalue[k] = readSunKwh();

					byte count = 0;
					//------------------------------------------------repeat reading for wrong values up to 10 times____
					while (count < 10 &&lvalue[k]<=0||lvalue[k]>100000)
					{
						count++;
						pulseLed(2000, 100, 1);
						lvalue[k] = readSunKwh();
					}
					//	if (now() % SECS_PER_DAY < timeint)EELONGW(SUN0H, lvalue[k]);//----write Solar KWH at 0h
						value[k] = lvalue[k]; k++;
						weather[iw].water = value[k - 1];
				}
				else if (sensorT[i] == 3)//__________________ Weather data from Underground Station___________
				{
					float val[8];
					
					String Str="";
					Str+= OpName[WU_URL];
					SPL(OpName[0]);
					Str += name[i];
					Str+=".json";
					
					String nomi[10] = { "current_observation","             ","                ","            ","               ","             ","","","","" };
					for (byte ii = 1; ii < pin2[i] + 1; ii++)
						nomi[ii]= param[k + ii - 1];
					bool noData = true;
					SPL(Str);
					if (APIweatherV(Str, nomi, pin2[i], val)) noData = false;
					if (noData||val[0]<now()-10000){  //_________________no readings__________
						pulseLed(500, 200, 2);
						for (byte ii = 0; ii < pin2[i]; ii++)val[ii] = -1;
					}
					else
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
						byte iwm1 = iw - 1;
						if (iw == 0)iwm1 = MAX_WEATHER_READ-1;
						if (now() > weather[iwm1].time +timeint+ 100)sumET0 += value[k - 1] * 10000 * (now() - weather[iwm1].time) / 3600;
						else						sumET0 += value[k-1] * 10000 * timeint / 3600;
				//		SP("sET0"); SPL(sumET0);
						EELONGW(SUMETPOS, sumET0);
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
					for (byte j = 0; j < nres; j++)value[k ++ ] = temp[j];
				}
			}

			eeprom_write_byte((byte *)WEATHERPOS, iw);
			eeprom_write_block(&weather[iw], (void*)(WEATHERPOS + 1+iw*sizeof(Weather)), sizeof(Weather));
		return 0;
	}
	return 1;
}
#define SHORTFILE
	void recordSensors(long timeint) {
		byte k = 0;
#ifdef SHORTFILE
		if (time_sensor1 == 0)
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
			logfile.println();
		}
#endif
		
		if (millis() > time_sensor1) {
			logfile = SPIFFS.open("/logs.txt", "a+"); 
			logfile.seek(0, SeekEnd);
			time_sensor1 = millis() + timeint * 1000;
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
		logfile.seek(-dtime, SeekEnd);
		if (!logfile.find("t:"))return -1;
		
		int n=logfile.readBytesUntil(',', buf, 10);
		buf[n] = 0;
		bool found = true;
		while (atol(buf) < time ) {

			found = logfile.find("t:");// OpName[TIME_STR].c_str());
			if (found) {
				n = logfile.readBytesUntil(',', buf, 10);
				buf[n] = 0;
				SPL(buf);
			}
			else return -1;


		}
	
		//logfile.seek(-12, SeekCur);
		byte k = 0;
		n= logfile.readBytesUntil('\n', buf, 300);
		buf[n] = 0;
#ifdef SHORTFILE
		char * buffer = (char *)malloc(300);

		char * buff = strtok(buf, ":");
		buffer[0] = 0;
		for (byte i = 0; i < nsensors; i++) {
			buffer = strcat(buffer, buff);

			buffer = strcat(buffer, name[i].c_str());
			buffer = strcat(buffer, ":");
//			SPL(buffer);
			buff = strtok(NULL, ":");

			for (byte j = 0; j <pin2[i]; j++) {

				//while (buff!=NULL) {
//				SPL(buff);
				buffer = strcat(buffer, buff);

				buffer = strcat(buffer, param[k++].c_str());
				buffer = strcat(buffer, ":");
//				SPL(buffer);
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

private:
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
			Serial.print("no resp"); return 0;
		}
		i = 0;
		while (client.available()) {
			buff[i++] = client.read();
			Serial.print(buff[i - 1]);
			delay(100);

		}
		buff[i++] = 0;
		client.stop();
	//	SPL(i);
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
		char buff[20]; char * ii; int ival[4]; byte i = 1;
		IPAddress ip(192, 168, 1, 77);
		if (ServerCall(buff, "//", ip) > 1) {
			ival[0] = atoi(strtok(buff, " "));
			ii = strtok(NULL, " ");
			while (ii != NULL) { ival[i++] = atoi(ii); ii = strtok(NULL, " "); }
			return ival[2];
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
		distance = duration*(331 + 0.6*temp) / 200;
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
	
    File f = SPIFFS.open(server.arg(0), "r");
    if (!f) {
      server.send(200,"text/plain","Can't open SPIFFS file "+server.arg(0)+" !\r\n");          
    }
	else {
		char buf[BUFFDIM];
		long siz = f.size();
		SP("Download "); SP(server.arg(0)); SPL(siz);
		long bytes = atol(server.arg(1).c_str());
		if (bytes > 0) siz = bytes;
		if (siz > MAXSIZ) { f.seek(-MAXSIZ, SeekEnd); siz = MAXSIZ; }
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
void handlePr() {}
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
	MAX_WEATHER_SIZE,				//dimension of weather struct for averages
	LINEAR_REGRESSION_DELTA_X,		//stored linear frequency sec.
	NSPLIT,							//n. of linear regression subtotatls
	LATITUDE,
	LONGITUDE,
	ELEVATION,
	PANEL_ANGLE,
	PANEL_AZIMUT,
	SOLAR_PANEL_FACTOR


};
int opt[20] = { 360,360 };
void restart(byte ind){
	EEPROM.write(0, ind);
	byte b = EEPROM.read(1);
	b++;
	EEPROM.write(1, b);
	EEPROM.commit();
	ESP.restart();

}

void setup(void) {

	pinMode(led, OUTPUT);
	digitalWrite(led, 0);
	EEPROM.begin(2024);
	EELONGR(SUMETPOS, my.sumET0);
	Serial.begin(115200);
	if (!SPIFFS.begin()) {
		Serial.println("SPIFFS failed to mount !\r\n");
		pulseLed(10000, 100, 2);
		restart(2);
	}
#define READCONF




	String parametri[10];
	
	byte formati[10];

#ifdef READCONF
	char buf[100];

	File cfile = SPIFFS.open("/config.txt", "r");
						//first line contain comments
	cfile.find(13);
	buf[0] = '/';
	while (buf[0]=='/')buf[cfile.readBytesUntil(13, buf, 100)]=0;  //second option interger values
	byte n;
	SPL(buf);
	byte n_opt = atoi(strtok(buf, ","));
	for (byte i = 0; i < n_opt; i++)opt[i] = atoi(strtok(NULL, ",\n"));
	buf[0] = '/';
	while (buf[0] == '/')n=cfile.readBytesUntil(13, buf, 100);
	buf[n ] = 0;  //thrird line options names
	SPL(buf);

	byte n_optn = atoi(strtok(buf, ","));

	for (byte i = 0; i < n_optn; i++){
		 char * nam= strtok(NULL, ",");
		 OpName[i] = "";
		 OpName[i] += nam; SPL(OpName[i]);
	}
	byte k = 0;
	
	while (cfile.available()) {			//following line contain one type(1st v) of (3rd v)sensors :name{sensor1:    ,sensor2:     ,     ,} connected to (2nd v.)
		byte lbuf;
		buf[0] = '/';
		while (buf[0] == '/')lbuf = cfile.readBytesUntil(13, buf, 100);
		buf[lbuf] = 0;
		if (lbuf>6) {
			SPL(buf);
			char * point = strtok(buf, ",");
			if (point != NULL) {
				SPL(point);
				byte p1 = atoi(point);
				point = strtok(NULL, ",");
				if (point != NULL) {
					SPL(point);
					byte p2 = atoi(point);
					point = strtok(NULL, ",");
					if (point != NULL) {
						SPL(point);
						byte p3 = atoi(point);
						
						char  nome[20];
						strcpy(nome,strtok(NULL, ","));
						SP(p1); SP(' '); SP(p2); SP(' '); SP(p3); SP(' '); SP(nome); SP(' ');

						for (byte i = 0; i < p3 - 1; i++) { parametri[i] =strtok(NULL, ","); SP(parametri[i]); SP(' '); }
						parametri[p3 - 1] =strtok(NULL, "\n"); SP(parametri[p3 - 1]);
						my.beginSensor(p1, p2, p3, nome, parametri);
					}
				}
			}
		}
	}
	cfile.close();
#else
	//const char* parametri[10];
	parametri[0] = "Kwh";
	my.beginSensor(2, 0, 1, "SunPan", parametri);
	// ultrasonic sensor on GPIO 12
	parametri[0] = "Dist";
	my.beginSensor(0, 10 * 16 + 10, 1, "wlev", parametri);
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
	parametri[2] = "1hlev_var";
	parametri[3] = "dayETsum";
	parametri[4] = "DayLevVar";
	my.beginSensor(4, 0, 5, "ET0", parametri);
#endif
	delay(2000);
	WiFi.begin(OpName[SSID].c_str(), OpName[PSW].c_str());
	delay(500);
	if (opt[IP1] > 0) {
		SPS(opt[IP1]); SPS(opt[IP2]); SPS(opt[IP3]); SPS(opt[IP4]);
		WiFi.config(IPAddress(opt[IP1], opt[IP2], opt[IP3], opt[IP4]), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0), IPAddress(192, 168, 1, 1));
	}

	Serial.println("");
	
	byte count = 0;
	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print("."); pulseLed(100, 10, 1);;
		if (count++ >= 120) { pulseLed(10000, 0, 1); restart(0); }
	}

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(OpName[SSID]);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	Serial.println(WiFi.gatewayIP());
	Serial.println(WiFi.dnsIP());

	IPAddress GateIP(192, 168, 1, 1);
	byte ntry = 0;
	while (!Ping.ping(GateIP)) {
		pulseLed(100, 100, 1); ntry++;
}
	SP("pingOK"); SPL(ntry);
	if (SyncNPT(1))Serial.println("NPT time sync"); 
	if(now()<1000000000UL) {
		pulseLed(5000, 1000, 2); restart(1);
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
		  Serial.println("Cannot open logFile"); pulseLed(5000, 1000, 3); restart(3);
	  }
	  //	 while (logfile.available())Serial.print((char)logfile.read());
	  logfile.seek(0, SeekEnd);
  
  
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
  server.on("/", handleRoot);
 server.on("/download",handleDownload);
 server.on("/dir",handleDir);
 server.on("/del", handleDel);
 server.on("/edit", handleEdit);
 server.on("/pr", handlePr);
 server.on("/sensor", handleSensorsGet);
 server.on("/level", handleLevelTrend);
 server.on("/inline", [](){
	 
	 server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();

  Serial.println("HTTP server started");
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
	if(my.readSensorsFromFile(timedelay, buf) == -1) {
		server.send(200, "text/plain",reply+ " not found! \r\n");
		return;
	}
	reply += buf;
	SPL(reply);
	server.send(200, "text/plain",reply+"\r\n");

}
void handleSensorsContr() {

	byte nsensor = atoi(server.arg(0).c_str());
	bool state = my.onSensor(nsensor, server.arg(1).c_str()  );  //pump,on=   "pool.dist>50&&temp>0", off=   "pool.dist<10"
	if (state) digitalWrite(atoi(server.arg(2).c_str()),state);
}
void loop(void){
#ifdef TELNET
	getClients();
#endif
#ifdef OTA
	ArduinoOTA.handle();
#endif
	if(my.readSensors(opt[RecordInterval]))
				eeprom_write_block(&pool, (void*)MYPOS, sizeof(pool));
		
	my.recordSensors(opt[ RecordInterval ]);

  server.handleClient();
}
