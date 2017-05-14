ASensor library
-------------
Introduction
------------
ASensor (arduino sensor) library that contains wrappers for different sensors implemented in external libraries. Can be used as standalone or together with AWind library

Example:
--------
Sensors example demonstrates basic measurement node that measures and sends via radio temperature and humidity results
Installation: AFrame + https://github.com/AndreiDegtiarev/arduino-DHT + https://github.com/AndreiDegtiarev/RF24 libraries need to be installed

Wiring: DHT sensor should be connected to the pin 10
Radio sender: see rf24 library

Documentation
------------
API documentation: http://andreidegtiarev.github.io/ASensor/

Installation
------------
This library is installed together with AWind library. The content of awind_full.zip from https://github.com/AndreiDegtiarev/AWind/releases has to be unziped into arduinosketchfolder/libraries/ folder.
Optionally if you need specific sensors following libraries need to be installed:
* RF24:    https://github.com/AndreiDegtiarev/RF24
* DHT:     https://github.com/AndreiDegtiarev/arduino-DHT
* BMP085:  https://github.com/AndreiDegtiarev/Adafruit_Sensor + https://github.com/AndreiDegtiarev/Adafruit_BMP085_Unified
* DS18B20: https://github.com/AndreiDegtiarev/OneWire

License
------------
ASensor library can be redistributed and/or modified under the terms of the MIT license. Please see the included documents for further information.
