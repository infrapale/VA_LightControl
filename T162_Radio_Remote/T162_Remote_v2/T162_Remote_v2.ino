//#define MH1_RELAY
//#define MH2_RELAY
//#define TK_RELAY

#define MENU_DATA


//*********************************************************************************************
#define SERIAL_BAUD   9600   //115200

#define RFM69_CS      10
#define RFM69_INT     2
#define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     9
#define RFM69_FREQ      434.0   //915.0
#define RFM69_TX_IVAL_100ms  20

#define LED_YELLOW    5
#define LED_RED       LED_YELLOW+1
#define LED           13  // onboard blinky

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "rfm.h"
#include <RH_RF69.h>
#include <Secret.h>

#include <Astrid.h>
#include <VillaAstrid.h>
#include <SimpleTimer.h> 
#include <SmartLoop.h>
#include <akbd4.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************


int16_t packetnum = 0;  // packet counter, we increment per xmission

RH_RF69 rf69(RFM69_CS, RFM69_IRQN);

SmartLoop Smart(1);
SimpleTimer timer;
akbd4 kbd(0,1,2,3);

unit_type_entry Me ={"MH1T1","Terminal","T159","T148","T159","17v01",'0'}; //len = 9,5,5,5,9,byte
time_type MyTime = {2016, 1,30,12,05,30}; 
boolean msgReady;
boolean SerialFlag;
char key;
byte tx_delay_10ms;
byte yellow_cntr;

void setup() {
    while (!Serial); // wait until serial console is open, remove if not tethered to computer
    //Serial.begin(SERIAL_BAUD);
    Smart.begin(SERIAL_BAUD);
    Serial.println("T162_Remote_v2");
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    digitalWrite(LED_YELLOW,HIGH);
    digitalWrite(LED_RED, HIGH);
  
    radio_init(RFM69_CS,RFM69_IRQN,RFM69_RST, RFM69_FREQ);
    radio_send_msg("T162_Remote");

    timer.setInterval(10, run_10ms);
    timer.setInterval(1000, run_1000ms);
}


void loop() {
  byte msg_indx;
  char radio_packet[20] = "";
  byte i; 
  String r_addr;
  
  
  //delay(1000);  // Wait 1 second between transmits, could also 'sleep' here!
  timer.run();
  key = kbd.read();
  if (key != 0) {
     Serial.print("key(hex)=");Serial.println(key,HEX);
     msg_indx = 0;
     switch(key){
     case '7': radiate_msg("RLH_1"); break;
     case '4': radiate_msg("RPOLK"); break;
     case '1': radiate_msg("RTERA"); break;
     case '5': 
        radiate_msg("RGBRD"); 
        //radiate_msg("RKOK3"); 
        //radiate_msg("RKOK4"); 
        break;
 
     case '2': 
        radiate_msg("RGTUP"); 
        //radiate_msg("RTUP1"); 
        //radiate_msg("RTUP2"); 
        break;
     case '*': radiate_msg("RGKHH"); break;
     case '3': 
        radiate_msg("RGKOK"); 
        //radiate_msg("RKOK1"); 
        //radiate_msg("RKOK2"); 
        break;
     case '0': radiate_msg("RPARV"); break;
     case '8': radiate_msg("RGWC_"); break;
     case '#': //MH1
        radiate_msg("RGMH1"); 
        //radiate_msg("RMH11"); 
        //radiate_msg("RMH12"); 
        //radiate_msg("RMH13"); 
        //radiate_msg("RMH14"); 
        break;
     case '9': radiate_msg("RET_1"); break;
     case '6': 
        radiate_msg("RGMH2"); 
        //radiate_msg("RMH21"); 
        //radiate_msg("RMH22"); 
        //radiate_msg("RMH23"); 
        break;
     case 'C': radiate_msg("RPIHA"); break;
     case 'B': radiate_msg("RTK_1"); break;
     case 'A': radiate_msg("AUTO1"); break;
    } 
  }  
}
void radiate_msg( char *relay_addr ) {
    digitalWrite(LED_RED, LOW); 
    String relay_json = JsonRelayString"VA", relay_addr, "T", "" )
    char rf69_packet[RADIO_MSG_LEN] = "";
    relay_json.toCharArray(rf69_packet, RADIO_MSG_LEN);
    radio_send_msg(rf69_packet);
    Serial.println(radio_packet);
    delay(500);
    digitalWrite(LED_RED, HIGH); 
}



void run_10ms(void){
   Smart.HeartBeat10ms();
   if( Smart.Monitor()) msgReady = true;
   kbd.scan();
   if ( yellow_cntr  == 0 )
      digitalWrite(LED_YELLOW, HIGH);  
   else
      yellow_cntr--;
}

void run_1000ms(){
   int aval;
   //aval = analogRead(A0);
   //Serial.print("A0="); Serial.println(analogRead(A0));
   //Serial.print("A1="); Serial.println(analogRead(A1));
   //Serial.print("A2="); Serial.println(analogRead(A2));
   //Serial.print("A3="); Serial.println(analogRead(A3));
   
   digitalWrite(LED_YELLOW, LOW); yellow_cntr= 10; 
   if (++MyTime.second > 59 ){
         //Smart.Send(SM_BROADCAST,"TK_R1","4=1");

      MyTime.second = 0;
      if (++MyTime.minute > 59 ){
         MyTime.minute = 0;
         if (++MyTime.hour > 23){
            MyTime.hour = 0;
         }
      }   
   }
}



void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i=0; i<loops; i++)
  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
