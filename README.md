
#OpenSensor
 
The software is still in development, but most of basic functions are already available and
mainly a simplification and a streamling work is necessary. Several test unit are now operational
and work without failure for several month .
If you want to test the SW now keep in mind that is not well documented and you may discover bugs and
uncompleate features.
Please let me know your comments and suggestions that will be precious for may remainig coding effort.
 
##INSTALLATION

No special HW is required: an ESP8266 family device is sufficient (better with 4MB flash)!
Code use various Flags (#define is on first few lines) to activate or disactivate SW functions like:
	
	TELNET telnet server to debug the unit and verify operations
	OTA    Over the Air upload of files and SW
	FIX_IP Fixed or dYnamic IP
	IOT    Use of ThingsSpeak channels
	FTP		Ftp server to upload and download the files from SPIFFS

Keep in mind that you cannot run all the services at the same time since this is not supported by the ESP8266 libraries 
(for example if you want to use FTP you should delete TELNET & OTA .....)
	 
OpenSensor can only be compiled on ESP8266 using ESPArduinoCore libraries.
In addition all the libraries for the sensors must be available on /libraries/ folder (see first lines of code) .
To start using the SW you have to upload to SPIFFS the text file config.sys that contain default configuration of your setup:
see LOGRULE below for explanation of the setup values. This file will be read when you reset your unit and copied in EEPROM 
from where you can modify each record (LOGRULE) (a restart is needed). Since wrong values may affect your startup and create boot loops,
you can restore your default pressing the flash button(pin0 low ) when you restart the unit.
LogRules (config.txt records) on first 2 lines define startup costants on following lines sensor data to be read and recorded.
In addition you may specify witch pin to use for direct control from RULES and the values that you want to send to ThingSpeak.
  

##Main Functions:

###Network
The units must be located in a WiFi network with an fixed IP 192.168.1.xx where xx is the network unique identification of the stations .  
 
###Sensor aquisition:
The type of sensors that OpenSensor supports can be extended very easily including their libraries and adding few lines of code,
the ones available right now are:

    *ultrasonic distance sensor like HC SR04
    *temperature humidity sensor like RHT 03 or DHT0xx
    *one wire family sensors like ds18b20 temp probes
    *any other GPIO input reading
 
###Remote units and remote json data aquisition:
 
input data can be also optained by Web Query :e.g. data from another unit and Weather Undergroud Stations Data
 
###Local computations:

Computation are mainly performed by Bitlash command line that are executed in intervals.
Some special peace of SW has been written for special computation : 

    *ETo (evotranspiration water lost during day)computations using data from local or remote weather stations: the required sun radiation is obtained from WU station (if "solarradiation" is available) or from kw produced by SOLAR PANEL (if available provide input data for calculation to correct for panel location,inclination & azimuth)Â Â 
    *least square smooting routines to interpolate sensor inputs.
 
###Data recording:
 
Data can be logged locally on the internal flash memory (4MB) using a SPIFFS file: /logs.txt.This file can be read remotely by a web query.
You can also send data to Thingspeak channels that you can read from everywhere.
All these fuctions are controlled by LOGRULES with the format specified below.
Data is written in the flash memory in clear but compact format and can be retrieved by Web ULR commands.Â 
 
###Local end remote GPIOs control:
 
Output commands can go to any available output pin and control any compatible device :like relay, leds, alarms Â…whatsoever!
or can be sent to another unit or to an Open Sprinkler station.
 
The number of available pin can be extended easily using PCF8574 multiplexer unit that you can connect with I2c.
 
###Web control:
 
Following WEB control URL commands are available:

    * /download?file=filename       send to client last 20000 byte of filename.
    * /dir                          list SPIFFS file 
    * /del?file=filename            delete filename from SPIFF 
    * /sensor?file=filename&t=delay retrieve the record logged delay minutes 
    * /restart                      restart  the unit
    * /rst                          reset the unit
    * /rule?............            has defined in following paragraf 
    * /logrule?..........
 
###Rules and Logrules:
 
 
The operations of OpenSensor are controlled byÂ  sequence of commands lines of t wo types:
 
LOGRULES : (default LOGRULE are read from /config.txt file and can be changed with above command)

 1st line :

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
 
2° line :

        n.of terms
        /api/wupersonalkey/conditions/q/Nation/  WeatherUnderground query string
        SSID									SSID of local WiFi network
		PSW                                     password  of local WiFi network
        Server Name                             mDNS name on local network
3° line on:

        p1 type of sensor:
            0= HC SR04 ultraso ic ; 
            1=DHTxx or RHTxx sensor;
            3=W.U. station data;
            4=ET0 computation;
            5=one wire sensor;
    case p1<=9
        p2 additional term
        p3 n.of data values read and recoreded
        name of the sensor
        name of p3 values
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
 
RULES:  (specify a Bitlash c ommand to be executed)
 
Rules are written with Bitlash sintax and are executed at a time interval . In addition to basic Bitlash functions
that perform computation and address local GPIO , I have added the possibility to access all logged datas with the function
	gval(i) return value of sensor variable where i is the sequence number or the name of the logged values.
	sval(i,val)  set value val of sensor variable where i is the sequence number or the name of the logged values
 other function added to Bitlash:
	 apicom(host:value,URL:string+value+string+value....) to allow user to send an URL to a station in the same network following OpenSprinkler API syntax.
     apiget(host:value,query:string,name:string) get value of a remote station variables in with a Json API query ......to be implemented......

RULE are managed with following URL command:
 
http://ip1.ip2.ip3.ip4/rule?command=xxx&comand1=xxx&command2=xxx
http://ip1.ip2.ip3.ip4/logrule?command=xxx&comand1=xxx&command2=xxx


        case command add=ruleString         you add a new rule to be executed every minute
        case command rep=n&rule=ruleString  you replace RULE n from the list with the new ruleString
        case command rep=-1&rule=ruleString you execute a RULE immediately without addition to the list
        case command del=n                  you delete  rule number n from the list
        case command list=                  you list all RULES 

 
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

