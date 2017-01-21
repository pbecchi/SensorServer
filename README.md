
# OpenSensor
 
The software is still in development, but most of basic functions are already available and mainly a simplification and a streamling work is necessary. Several test unit are now operationaland work without failure for several month .
If you want to test the SW now keep in mind that is not well documented and you may discover bugs and uncompleate features.
Please let me know your comments and suggestions that will be precious for may remainig coding effort.
 
## INSTALLATION


No special HW is required: an ESP8266 family device is sufficient (better with 4MB flash)!
Code use various Flags (#define is on first few lines) to activate or disactivate SW functions like:
	
	TELNET telnet server to debug the unit and verify operations
	OTA    Over the Air upload of files and SW
	FIX_IP Fixed or dYnamic IP
	IOT    Use of ThingsSpeak channels
	FTP		Ftp server to upload and download the files from SPIFFS

Keep in mind that you cannot run all the services at the same time since this is not supported by the ESP8266 libraries (for example if you want to use FTP you should delete TELNET & OTA .....)
	 
OpenSensor can only be compiled on ESP8266 using ESPArduinoCore libraries.
In addition all the libraries for the sensors must be available on /libraries/ folder (see first lines of code) .
To start using the SW you have to upload to SPIFFS the text file config.sys that contain default configuration of your setup:
see LOGRULE below for explanation of the setup values. This file will be read when you reset your unit and copied in EEPROM from where you can modify each record (LOGRULE) (a restart is needed). Since wrong values may affect your startup and create boot loops, you can restore your default pressing the flash button(pin0 low ) when you restart the unit.
LogRules (config.txt records) on first 2 lines define startup costants on following lines sensor data to be read and recorded.
In addition you may specify witch pin to use for direct control from RULES and the values that you want to send to ThingSpeak.
  

## Main Functions:

### Network
The units must be located in a WiFi network with an fixed IP 192.168.1.xx where xx is the network unique identification of the stations .  
 
### Sensor aquisition:
The type of sensors that OpenSensor supports can be extended very easily including their libraries and adding few lines of code, the ones available right now are:

    *ultrasonic distance sensor like HC SR04
    *temperature humidity sensor like RHT 03 or DHT0xx
    *one wire family sensors like ds18b20 temp probes
    *any other GPIO input reading
 
### Remote units and remote json data aquisition:
 
input data can be also optained by Web Query :e.g. data from another unit and Weather Undergroud Stations Data
 
### Local computations:

Computation are mainly performed by Bitlash command line that are executed in intervals.

####Routines available for special computation : 

-ETo (evotranspiration water lost during day)computations using data from local or remote weather stations: the required sun radiation is obtained from WU station (if "solarradiation" is available) or from kw produced by SOLAR PANEL (if available you should provide  panel location,inclination & azimuth to compute a correction factors)  
Required input values to be read every 6 minutes  (names must mach first column names: WU parameters names):

    -temp_c             measured    temp [centigrade],
    -relative_humidity  humidity[%],
    -wind_kph           wind[km/h],
    -solarradiation     sun radiation[w]
    -kwh                sunpower[kwh] (from solar panels)
    
Compute :

    1.hourly ET0         from solarradiation input
    2.hourly ET0         from solarpower (from Solar Panels knowing positions , angles and factor(solar radiation/Panelspower*100)
    3.dayly ET0           sum of day hourly values.
   
   
    
    
 
### Data recording:
 
Data can be logged locally on the internal flash memory (4MB) using a SPIFFS file: /logs.txt.This file can be read remotely by a web query.
You can also send data to Thingspeak channels that you can read from everywhere.
All these fuctions are controlled by LOGRULES with the format specified below.
Data is written in the flash memory in clear but compact format and can be retrieved by Web ULR commands.
 
### Local end remote GPIOs control:
 
Output commands can go to any available output pin and control any compatible device :like relay, leds, alarms ......or can be sent to another unit or to an Open Sprinkler station.
 
The number of available pin can be extended easily using PCF8574 multiplexer unit that you can connect with I2c.
 
### Web control:
 
Following WEB control URL commands are available:

    * /download?file=filename       send to client last 20000 byte of filename.
    * /dir                          list SPIFFS file 
    * /del?file=filename            delete filename from SPIFF 
    * /sensor?file=filename&t=delay retrieve the record logged delay minutes 
    * /restart                      restart  the unit
    * /rst                          reset the unit
    * /rule?............            has defined in following paragraf 
    * /logrule?..........
 
### Rules and Logrules:
 
 
The operations of OpenSensor are controlled byÂ  sequence of commands lines of t wo types:
 
LOGRULES : (default LOGRULE are read from /config.txt file and can be changed with above command)

#### 1st line :

        n.of terms
        timing interval for input reading (in seconds)
        timing interval for input recording(in seconds)
        ndis number of distance measurements for averaging
        n4   (not used now)
        pof  factor for water tank level measurement =1
        ip1  1st term of network IP
        ip2  2nd term of network IP
        ip3  3rd term of network IP
		ip4  4rd term of network IP
		s_w  (not used now)
        de_x (not used now)
        nslp (not used now)
        solar panel latitude int value = latitude*10000
        solar panel longitude int value =longitude*10000
        solar panel elevation int value mt.
        solar panel inclination  from horizzontal
        solar panel azimuth  from south (+ east)
        solar panel factor (to derive from electric KW produced sun KW available int*100.
 
###  2° line :

        n.of terms
        /api/wupersonalkey/conditions/q/Nation/  WeatherUnderground query string
        SSID									SSID of local WiFi network
		PSW                                     password  of local WiFi network
        Server Name                             mDNS name on local network
### 3° line on:

        p1 type of sensor:
            0= HC SR04 ultraso ic ;  value measured is a distance in mm*10;
            1=DHTxx or RHTxx sensor; valuee are measured temperature and humidity
            3=W.U. station data;     value are retrieved from named station, value are retrieved that match given descriptions.
            4=ET0 computation;       first value hour ET0 from WU station sunrad,second value  hour ET0 from Solar panels, 3rd value day ET0 = sum of hour ETo
            5=one wire sensor;       one value from each sensor connected to the One wire bus.
			6=analog input on A0;    value is actual milliVolts read. ( 0 to 1 Volt) 
			8=pulse reading on pin1; first value count, second value pulses/sec
    case p1<=9
        p2 additional term                       GPIO pin value (if applicable)
        p3 n.of data values read and recoreded   n. of values obtained and recorded
        name of the sensor						 name of the sensor (name of WU station)	
        name of p3 values						 names of values (name of the JSON weather underground parameters)
    case p1=10     (for RULE tup :define used GPIO in output)
        p2 not used
        p3 n.of GPIO
        name  not used
        number of GPIO used (p3 values)
    case p1>20
        p1=Thingspeak channel n.
        p3=n.of terms<=8
        paswword for writing on thingspeak channel
        sequence  of internal values to be uploaded t Thingspeak (sequential number of data to be read and recorded specified above  )
 
##  RULES:  

(specify a Bitlash c ommand to be executed)
 
Rules are written with Bitlash sintax and are executed at a time interval . In addition to basic Bitlash functions that perform computation and address local GPIO , I have added the possibility to access all logged datas with the function :	  
gval(i) return value of sensor variable where i is the sequence number or the name of the logged values.	
sval(i,val)  set value val of sensor variable where i is the sequence number or the name of the logged values

 other function added to Bitlash:
	 apicom(host  Pvalue,URL:string+value+string+value....) to allow user to send an URL command to a station in the same network following OpenSprinkler API syntax.
     apiget(host:value Varname,query:string,name:string,val,string,val......) get value of a remote station variables in with a Json API query

RULE are managed with following URL command:
 
http://ip1.ip2.ip3.ip4/rule?command=xxx&comand1=xxx&command2=xxx
http://ip1.ip2.ip3.ip4/logrule?command=xxx&comand1=xxx&command2=xxx

where commands are:

        add=ruleString         you add a new rule to be executed every minute
        rep=n&rule=ruleString  you replace RULE n from the list with the new ruleString
        rep=-1&rule=ruleString you execute a RULE immediately without addition to the list
        del=n                  you delete  rule number n from the list
        list=                  you list all RULES 

## ET0 computations

Biggest difficulty for ET0 calculation is to have necessary meteo data available.
ET0 computations require the knowledge of actual local temperature humidity and wind + actual solar radiation.
First 3 elements can be easily obtained by a local WU station while is more difficult to get  sun radiation from WU.
You should define a local WU station for reading temperature,humidity ,rain and wind while you can get sunradiation from a more distant one (like in my the example).
If you dont find any sunrad on Weather Underground than you can use the power produced from solar panels( knowing location inclination and direction of the panels).
This power corrected by the angles will be proportional to sunradiation: you will need to perform a calibration to trasform  electrical power to sun radiation.
All input values need to be read put into  weather[i]  structure. Those value are the imput data and will be averaged at the time of ET0  calculation. 
So, in general , hourly ET0 is computed either from WU sunrad and from Solar Panel power, the available values are integrated to obtain (at sunset) current dayly ET0.
  

## BITLASH commands

I have ported a subset of the Billroy BITLASH code to ESP8266 . This library allow a VERY POWERFULL method to control Open Sensors.
Bitlash commands are the best way to customize the run time operations of Open Sensor. For example you can:

	-perform a certain operation (close a relay for example..) on this or on another unit when certain conditions are met
	-change rain delay of an Open Sprinkler unit when there has been a certain amount of rain
	-compute a rain delay duration on the base of actual ET0
	-open a valve (or relay) on under certain time (>timestart && <timestart+duration) and external temperature conditions
Data used on computation are sensors measurements for this or any other unit in the network and commands (in term of setting GPIO High or Low) 	can be local or remote.
BITLASH Commands follow the sintax and the rules tha can be found on BITLASH USER MANUAL. They are created and  edited in  OpenSensor with  RULE url queries.
Commands are executed every 60 seconds (or only at the time you send the query) in the same order as the are listed and results may change a value of a variable or of a GPIO in any network unit.
Bitlash reconnise only  long integers and strings therefore if you have floating numbers you have to specify a esponent factor to convert the float to long int.
In addition to standard Bithlash functions a full set of user functions have been added:

	+now();
	+millis();
    +"apicom(string1,string2,n1,string3,n2.....)"			//send a URL command 
	+"et0"										            //get current ET0 not implemented yet
	+"distance(pin)"									        //get SR 04 sensor distance mm reading on (pin)
	+"dht_s(pin)"									            //get DHT011 sensor values temp.°C*10 
	+"onew_s(pin,n)"							    	    	//get pin onewire sensors readings (sensor n)
	+"wu_data"							        	    	    //get weather underground station data  not implemented
	+"apiget(varname,fact,ic,IP4,urlstring,urlval,urlstring,ulrval...)"									//get value of remote station var from Json API 
	
	+"sval"										//set value of my.sensor variable ( number or name)*factor
	+"gval"									    //get value of my.sensor variable (number or name)*factor

   



 
 ## EXAMPLE 

 As an example I include a following confix.txt file:

//  ,t_r,t_w,p-ty	,n4	,pof,ip1,ip2,ip3,ip4,s_w,de_x,nspl,	lat,	long,	elev,	pa_el,	pa_az,pa_fact 
18,360,360,10	,	,12	,192,168,001,031,10	,3600,	12, 441247,	82544,	23,		31,		12, 32
4,/api/48dfa951428393ba/conditions/q/Italy/,TP-LINK_C20B,paolo-48,poolESP
2,0,1,Sunpan,Kwh
//0,170,2,wlev,Dist,LevVar
//3,0,5,pws:ISAVONAL1,local_epoch,temp_c,relative_humidity,precip_today_metric,wind_kph
3,0,2,SAVONA,local_epoch,solarradiation
//4,0,3,ET0,sunrad,sol_pan,avg_var
5,2,1,DSTemp,Pool
10,0,3,prova,0,4,5
 
notes:

1.line: defines 360s log interval(*), IP=192.168.1.30(*) ,data for my solar panels .
2.Line : define WU api query first part,local WiFi data:SSID,PSW(*), nDNS name.
3 line : define recording of Solar Panel Kwh 
4 line : define distance water surface measurement
5 line : data to be recorded from WU station:pws:SAVONAL1 ................  (names must match json names)
6 line : similar data from WU station :SAVONA
7 line : hourly ET0 computation using meteo data from line 5 and SolarRadiation from line 6 and 3
8 line: one wire Temperature sensors
9 line : pin 0 4 and 5 as output to be addressed directely by Rules

