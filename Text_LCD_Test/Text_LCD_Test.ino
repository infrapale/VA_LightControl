/*********************************************************************
This is an example sketch for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include "TextLCD.h"

#define  ERROR_LED_PIN  13 //Led Pin: Typical Arduino Board
//#define  ERROR_LED_PIN  2 //Led Pin: samd21 xplained board

#define ERROR_LED_LIGHTUP_STATE  LOW // the state that makes the led light up on your board, either low or high

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
#define SERIAL          Serial

#ifndef __display__
#define __display__
//Adafruit_PCD8544 LCD_display = Adafruit_PCD8544(PCD_DC, PCD_CS, PCD_RST);
#endif

TextLcd lcd;




void setup()   {
  SERIAL.begin(115200);
  while (!SERIAL) ;  // Wait for serial terminal to open port before starting program
  SERIAL.println("TextLCD_Test");
  
  lcd.Clear();
  lcd.Write(0,"eka rivi");
  lcd.Write(1,"rivi 1");
  lcd.Write(2,"rivi 2");
  lcd.Write(3,"rivi 3");
  lcd.Show();
  
}

                                
void loop() {
      // Optional commands, can comment/uncomment below
    SERIAL.print("."); //print out dots in terminal, we only do this when the RTOS is in the idle state

}



 
