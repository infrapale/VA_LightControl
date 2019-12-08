//#define MH1_RELAY
//#define MH2_RELAY
//#define TK_RELAY

#define MENU_DATA

#define NETWORKID     100  // The same on all nodes that talk to each other
#define NODEID        2    // The unique identifier of this node
#define BROADCAST     255
#define RECEIVER      BROADCAST    // The recipient of packets

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD   9600   //115200

#define RFM69_CS      10
#define RFM69_IRQ     2
#define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     9
#define LED_YELLOW    5
#define LED_RED       LED_YELLOW+1
#define LED           13  // onboard blinky

#include <Arduino.h>
#include <Wire.h>
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <Astrid.h>
#include <VillaAstrid.h>
#include <SimpleTimer.h> 
#include <SmartLoop.h>
#include <akbd4.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************


int16_t packetnum = 0;  // packet counter, we increment per xmission

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);
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
  Serial.println("Arduino RFM69HCW Transmitter");
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_YELLOW,HIGH);
  digitalWrite(LED_RED, HIGH);
  
  // Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);

  // Initialize radio
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower();    // Only for RFM69HCW & HW!
  }
  radio.setPowerLevel(5); // power output ranges from 0 (5dBm) to 31 (20dBm)
  
  radio.encrypt(ENCRYPTKEY);
  kbd.set_aval(0,185);kbd.set_aval(1,416);kbd.set_aval(2,616);kbd.set_aval(3,847);
  pinMode(LED, OUTPUT);
  Serial.print("\nTransmitting at ");
  Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");
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
void radiate_msg( char *radio_msg ) {
    char radio_packet[20] = "";
    byte i;
    digitalWrite(LED_RED, LOW); 
    if (radio_msg[0] != 0){
       radio_packet[0] = SM_START;
       radio_packet[1] = SM_BROADCAST;
       for (i=0; i<6; i++) radio_packet[i+2]= radio_msg[i];  //msg_table[msg_indx].msg[i];
       i++;
       if (i<20-3) {
          radio_packet[i++] = SM_ASSIGN;
          radio_packet[i++] = SM_TOGGLE;
          radio_packet[i++] = SM_END;
          radio_packet[i] =0;
       }
       radio.send(RECEIVER, radio_packet, strlen(radio_packet),false);
       radio.receiveDone(); //put radio in RX mode
       Serial.println(radio_packet);

       delay(500);
       digitalWrite(LED_RED, HIGH); 

    }
}

byte get_node_id( char *msg ){
   byte i;

   
  
}
  // itoa(packetnum++, radiopacket+13, 10);
  /*
  Serial.print("Sending "); Serial.println(radiopacket);
    
  if (radio.sendWithRetry(RECEIVER, radiopacket, strlen(radiopacket))) { //target node Id, message as string or byte array, message length
    Serial.println("OK");
    Blink(LED, 50, 3); //blink LED 3 times, 50ms between blinks
  }
  else Serial.println("Send Error");

  radio.receiveDone(); //put radio in RX mode
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
  */


void SendRadioPacket( byte rec_nbr, char *radio_msg){
  //if (radio.sendWithRetry(RECEIVER, radio_msg, strlen(radio_msg))) { //target node Id, message as string or byte array, message length
  radio.send(RECEIVER, radio_msg, strlen(radio_msg),false);
  radio.receiveDone(); //put radio in RX mode


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
