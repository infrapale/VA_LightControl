// T124  Analog Keyboard 2 ADC 
// 2015-11-08 
// Row1-4: 179 - 409 - 605 - 830
// Col1-4:  0-1-2-3

#include <Arduino.h>
#include <Wire.h> 
#include <SimpleTimer.h> 
#define seven_x_three
#include "AnalogKbd.h"

akbd kbd(1,2,KBD_4x4);
char key;
int aval;


void setup()
{
  Serial.begin(9600);  // start serial for output
  kbd.begin();
}

void loop()
{
   while(1){
      kbd.scan();
      aval = kbd.rd_analog(0);
      Serial.print("0: ");
      Serial.println(aval);
      aval = kbd.rd_analog(1);
      key = kbd.read();
      Serial.print("1: ");
      Serial.println(aval);
      delay(1000);
      if (key) {
          Serial.println(key,DEC);
      }
    
  } 
}
