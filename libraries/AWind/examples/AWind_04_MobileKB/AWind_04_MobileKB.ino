#include <UTFT.h>
#include <UTouch.h>

#include "WindowsManager.h"
#include "Log.h"
#include "DefaultDecorators.h"
#include "wnd_info.h"
#ifdef ESP8266
#define F
#include <SPI.h>
UTFT myGLCD(ILI9341_S5P, 15, 5, 2);
UTouch  myTouch(4, 4, 32, 3, 5);

#else

// Setup TFT display + touch (see UTFT and UTouch library documentation)
#ifdef _VARIANT_ARDUINO_DUE_X_   //DUE +tft shield
UTFT    myGLCD(CTE32,25,26,27,28);
UTouch  myTouch(6,5,32,3,2);
#else
UTFT myGLCD(ITDB32S,38,39,40,41); //mega
UTouch myTouch(6,5,4,3,2);
#endif
#endif
//Windows manager: container for GUI elements 
WindowsManager<wnd_info> windowsManager(&myGLCD,&myTouch);
//Initialize apperance. Create your own DefaultDecorators class if you would like different application look
DefaultDecorators dfltDecorators(DefaultDecorators::all);

void setup()
{
	out.begin(57600);
	out<<(F("Setup"))<<endln;

	//initialize display
	myGLCD.InitLCD();
	myGLCD.clrScr();
	//initialize touch
	myTouch.InitTouch();
	myTouch.setPrecision(PREC_MEDIUM);

        DefaultDecorators::InitAll();
	//initialize window manager
	windowsManager.Initialize();
	AHelper::LogFreeRam();
	out<<F("End setup")<<endln;
}

void loop()
{
	windowsManager.loop();
}

