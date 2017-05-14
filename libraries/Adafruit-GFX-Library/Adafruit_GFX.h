#ifndef _ADAFRUIT_GFX_H
#define _ADAFRUIT_GFX_H

#if ARDUINO >= 100
 #include "Arduino.h"
 #include "Print.h"
#else
 #include "WProgram.h"
#endif

#include "gfxfont.h"

class Adafruit_GFX : public Print {

 public:

  Adafruit_GFX(int16_t w, int16_t h); // Constructor

  // This MUST be defined by the subclass:
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

  // These MAY be overridden by the subclass to provide device-specific
  // optimized code.  Otherwise 'generic' versions are used.
  virtual void
    drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),
    drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
    drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
    drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
    fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
    fillScreen(uint16_t color),
    invertDisplay(boolean i);

  // These exist only with Adafruit_GFX (no subclass overrides)
  void
    drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
    drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
      uint16_t color),
    fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
    fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
      int16_t delta, uint16_t color),
    drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
      int16_t x2, int16_t y2, uint16_t color),
    fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
      int16_t x2, int16_t y2, uint16_t color),
    drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
      int16_t radius, uint16_t color),
    fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
      int16_t radius, uint16_t color),
    drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
      int16_t w, int16_t h, uint16_t color),
    drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
      int16_t w, int16_t h, uint16_t color, uint16_t bg),
    drawBitmap(int16_t x, int16_t y, uint8_t *bitmap,
      int16_t w, int16_t h, uint16_t color),
    drawBitmap(int16_t x, int16_t y, uint8_t *bitmap,
      int16_t w, int16_t h, uint16_t color, uint16_t bg),
    drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
      int16_t w, int16_t h, uint16_t color),
    drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
      uint16_t bg, uint8_t size),
    setCursor(int16_t x, int16_t y),
    setTextColor(uint16_t c),
    setTextColor(uint16_t c, uint16_t bg),
    setTextSize(uint8_t s),
    setTextWrap(boolean w),
    setRotation(uint8_t r),
    cp437(boolean x=true),
    setFont(const GFXfont *f = NULL),
    getTextBounds(char *string, int16_t x, int16_t y,
      int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h),
    getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y,
      int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

#if ARDUINO >= 100
  virtual size_t write(uint8_t);
#else
  virtual void   write(uint8_t);
#endif

  int16_t height(void) const;
  int16_t width(void) const;

  uint8_t getRotation(void) const;

  // get current cursor position (get rotation safe maximum values, using: width() for x, height() for y)
  int16_t getCursorX(void) const;
  int16_t getCursorY(void) const;
  //////////////////////////////////////////////////////////////////////
  int drawChar(unsigned int uniCode, int x, int y, int size)
  {
	  unsigned int width = 0;
	  unsigned int height = 0;
	  unsigned int flash_address = 0;
	  char gap = 0;
	  uniCode -= 32;

	  drawChar(x, y, uniCode + 32, textcolor, textbgcolor, textsize);
	  return 6 * textsize;
  }
  /***************************************************************************************
  ** Function name:           drawNumber unsigned with size
  ** Description:             draw a long integer
  ***************************************************************************************/
  int drawNumber(long long_num, int poX, int poY, int size)
  {
	  char tmp[12];
	  if (long_num < 0) sprintf(tmp, "%li", long_num);
	  else sprintf(tmp, "%lu", long_num);
	  return drawString(tmp, poX, poY, size);
  }

  /***************************************************************************************
  ** Function name:           drawString
  ** Description :            draw string
  ***************************************************************************************/
  int drawString(char *string, int poX, int poY, int size)
  {
	  int sumX = 0;

	  while (*string)
	  {
		  int xPlus = drawChar(*string, poX, poY, size);
		  sumX += xPlus;
		  *string++;
		  poX += xPlus;                            /* Move cursor right       */
	  }
	  return sumX;
  }

  /***************************************************************************************
  ** Function name:           drawCentreString
  ** Descriptions:            draw string centred on dX
  ***************************************************************************************/
  int drawCentreString(char *string, int dX, int poY, int size)
  {
	  int sumX = 0;
	  int len = 0;
	  char *pointer = string;
	  char ascii;

	  while (*pointer)
	  {
		  ascii = *pointer;

#ifdef LOAD_GLCD
		  if (size == 1)len += 6;
#endif
#ifdef LOAD_FONT2
		  if (size == 2)len += 1 + pgm_read_byte(widtbl_f16 + ascii - 32);
#endif
		  //if (size==3)len += 1+pgm_read_byte(widtbl_f48+ascii-32)/2;
#ifdef LOAD_FONT4
		  if (size == 4)len += pgm_read_byte(widtbl_f32 + ascii - 32) - 3;
#endif
		  //if (size==5) len += pgm_read_byte(widtbl_f48+ascii-32)-3;
#ifdef LOAD_FONT6
		  if (size == 6) len += pgm_read_byte(widtbl_f64 + ascii - 32) - 3;
#endif
#ifdef LOAD_FONT7
		  if (size == 7) len += pgm_read_byte(widtbl_f7s + ascii - 32) + 2;
#endif
		  *pointer++;
	  }
	  len = len*textsize;
	  int poX = dX - len / 2;

	  if (poX < 0) poX = 0;

	  while (*string)
	  {
		  int xPlus = drawChar(*string, poX, poY, size);
		  sumX += xPlus;
		  *string++;
		  poX += xPlus;                  /* Move cursor right            */
	  }

	  return sumX;
  }

  /***************************************************************************************
  ** Function name:           drawRightString
  ** Descriptions:            draw string right justified to dX
  ***************************************************************************************/
  int drawRightString(char *string, int dX, int poY, int size)
  {
	  int sumX = 0;
	  int len = 0;
	  char *pointer = string;
	  char ascii;

	  while (*pointer)
	  {
		  ascii = *pointer;

#ifdef LOAD_GLCD
		  if (size == 1)len += 6;
#endif
#ifdef LOAD_FONT2
		  if (size == 2)len += 1 + pgm_read_byte(widtbl_f16 + ascii - 32);
#endif
		  //if (size==3)len += 1+pgm_read_byte(widtbl_f48+ascii-32)/2;
#ifdef LOAD_FONT4
		  if (size == 4)len += pgm_read_byte(widtbl_f32 + ascii - 32) - 3;
#endif
		  //if (size==5) len += pgm_read_byte(widtbl_f48+ascii-32)-3;
#ifdef LOAD_FONT6
		  if (size == 6) len += pgm_read_byte(widtbl_f64 + ascii - 32) - 3;
#endif
#ifdef LOAD_FONT7
		  if (size == 7) len += pgm_read_byte(widtbl_f7s + ascii - 32) + 2;
#endif
		  *pointer++;
	  }

	  len = len*textsize;
	  int poX = dX - len;

	  if (poX < 0) poX = 0;

	  while (*string)
	  {
		  int xPlus = drawChar(*string, poX, poY, size);
		  sumX += xPlus;
		  *string++;
		  poX += xPlus;          /* Move cursor right            */
	  }

	  return sumX;
  }

  /***************************************************************************************
  ** Function name:           drawFloat
  ** Descriptions:            drawFloat
  ***************************************************************************************/
  int drawFloat(float floatNumber, int decimal, int poX, int poY, int size)
  {
	  unsigned long temp = 0;
	  float decy = 0.0;
	  float rounding = 0.5;

	  float eep = 0.000001;

	  int sumX = 0;
	  int xPlus = 0;

	  if (floatNumber - 0.0 < eep)       // floatNumber < 0
	  {
		  xPlus = drawChar('-', poX, poY, size);
		  floatNumber = -floatNumber;

		  poX += xPlus;
		  sumX += xPlus;
	  }

	  for (unsigned char i = 0; i<decimal; ++i)
	  {
		  rounding /= 10.0;
	  }

	  floatNumber += rounding;

	  temp = (long)floatNumber;


	  xPlus = drawNumber(temp, poX, poY, size);

	  poX += xPlus;
	  sumX += xPlus;

	  if (decimal>0)
	  {
		  xPlus = drawChar('.', poX, poY, size);
		  poX += xPlus;                            /* Move cursor right            */
		  sumX += xPlus;
	  }
	  else
	  {
		  return sumX;
	  }

	  decy = floatNumber - temp;
	  for (unsigned char i = 0; i<decimal; i++)
	  {
		  decy *= 10;                                /* for the next decimal         */
		  temp = decy;                               /* get the decimal              */
		  xPlus = drawNumber(temp, poX, poY, size);

		  poX += xPlus;                              /* Move cursor right            */
		  sumX += xPlus;
		  decy -= temp;
	  }
	  return sumX;
  }
  /////////////////////////////////////////////////////////////////////
 protected:
  const int16_t
    WIDTH, HEIGHT;   // This is the 'raw' display w/h - never changes
  int16_t
    _width, _height, // Display w/h as modified by current rotation
    cursor_x, cursor_y;
  uint16_t
    textcolor, textbgcolor;
  uint8_t
    textsize,
    rotation;
  boolean
    wrap,   // If set, 'wrap' text at right edge of display
    _cp437; // If set, use correct CP437 charset (default is off)
  GFXfont
    *gfxFont;
};

class Adafruit_GFX_Button {

 public:
  Adafruit_GFX_Button(void);
  void initButton(Adafruit_GFX *gfx, int16_t x, int16_t y,
   uint8_t w, uint8_t h, uint16_t outline, uint16_t fill,
   uint16_t textcolor, char *label, uint8_t textsize);
  void drawButton(boolean inverted = false);
  boolean contains(int16_t x, int16_t y);

  void press(boolean p); // change state of button
  boolean isPressed();   // the button state is
  boolean justPressed(); // the button state from unpressed to pressed
  boolean justReleased();// the button state from pressed to unpressed

 private:
  Adafruit_GFX *_gfx;
  int16_t _x, _y;
  uint16_t _w, _h;
  uint8_t _textsize;
  uint16_t _outlinecolor, _fillcolor, _textcolor;
  char _label[10];

  boolean currstate, laststate;
};

class GFXcanvas1 : public Adafruit_GFX {

 public:
  GFXcanvas1(uint16_t w, uint16_t h);
  ~GFXcanvas1(void);
  void     drawPixel(int16_t x, int16_t y, uint16_t color),
           fillScreen(uint16_t color);
  uint8_t *getBuffer(void);
 private:
  uint8_t *buffer;
};

class GFXcanvas16 : public Adafruit_GFX {
  GFXcanvas16(uint16_t w, uint16_t h);
  ~GFXcanvas16(void);
  void      drawPixel(int16_t x, int16_t y, uint16_t color),
            fillScreen(uint16_t color);
  uint16_t *getBuffer(void);
 private:
  uint16_t *buffer;
};

#endif // _ADAFRUIT_GFX_H
