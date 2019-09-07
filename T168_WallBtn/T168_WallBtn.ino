#define MH1_BTN
//#define MH2_BTN
//#define K_BTN
//#define TK_RELAY

#define MENU_DATA

#define NETWORKID     100  // The same on all nodes that talk to each other
#define NODEID        12    // The unique identifier of this node
#define BROADCAST     255
#define RECEIVER      BROADCAST    // The recipient of packets

//Match frequency to the hardware version of the rf69 on your Feather
#define FREQUENCY     RF69_434MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
//#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD   9600   //115200

#define RFM69_CS      10
#define RFM69_INT     2
#define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     9
#define RFM69_FREQ    434.0   //915.0

#define LED           13  // onboard blinky
#define MAX_BTN   6
#include <Arduino.h>
#include <Wire.h>
#include <RH_RF69.h>
// #include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69

#include <SPI.h>
#include <Astrid.h>
#include <VillaAstrid.h>
#include <SimpleTimer.h> 
#include <SmartLoop.h>
#include <Secret.h>

#ifdef K_BTN
#include <akbd.h>
akbd kbd(A0);
akbd qkbd(A1);
//led_blink leds(6,7,3,9,5,4);
#endif

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************

struct btn_struct {
   byte pin;
   byte cntr;   
   boolean pressed;
   char value;
};

int16_t packetnum = 0;  // packet counter, we increment per xmission
RH_RF69 rf69(RFM69_CS, RFM69_INT);
// RFM69 rf69 = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

SmartLoop Smart(1);
SimpleTimer timer;

//akbd4 kbd(0,1,2,3);
//char key_buff[MAX_BTN];
unit_type_entry Me ={"MH1T1","Terminal","T168","T1150","T168","17v01",'0'}; //len = 9,5,5,5,9,byte
time_type MyTime = {2017, 1,30,12,05,30}; 
btn_struct btn[MAX_BTN] ={
  {3,0,false,0},
  {4,0,false,0},
  {5,0,false,0},
  {6,0,false,0},
  {7,0,false,0},
  {8,0,false,0}
};

boolean msgReady;
boolean SerialFlag;
char key;
byte tx_delay_10ms;
byte btn_indx;
 
void setup() {
  byte i;
  while (!Serial); // wait until serial console is open, remove if not tethered to computer
  //Serial.begin(SERIAL_BAUD);
  for (i=0;i<MAX_BTN;i++){
     if( btn[i].pin > 0) {
         pinMode(btn[i].pin, INPUT_PULLUP);
         btn[i].cntr = 0;
     }
  }
  btn_indx = 0;
  #ifdef K_BTN
  kbd.begin();
  qkbd.begin();
  #endif

  Smart.begin(SERIAL_BAUD);
  Serial.println("Arduino RFM69HCW Transmitter");
  
  // Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);
  #ifdef K_BTN
  kbd.begin();
  qkbd.begin();
  #endif
  //==========================================================
  // Initialize rf69
  //==========================================================
  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init()) {
    Serial.println("RFM69 rf69 init failed");
    while (1);
  }
  Serial.println("RFM69 rf69 init OK!");
  
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RFM69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // from secret.h        1234567890123456
  //uint8_t rfm69_key[] ="Xyzabde123456789"; //exactly the same 16 characters/bytes on all nodes!

  rf69.setEncryptionKey(rfm69_key);
  
  pinMode(LED, OUTPUT);

  Serial.print("RFM69 rf69 @");  Serial.print((int)RFM69_FREQ,DEC);  Serial.println(" MHz");


  
  //==========================================================
  // Real time settings
  //==========================================================

  timer.setInterval(10, run_10ms);
  timer.setInterval(1000, run_1000ms);

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Main Loop
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void loop() {
   byte msg_indx;
   char rf69_packet[20] = "";
   byte i; 
   String r_addr;
   boolean done;
  
   timer.run();
   #if defined(MH1_BTN) || defined(MH2_BTN)
   key = rd_btn();   //kbd.read();
   #endif
   #ifdef K_BTN
   key = qkbd.read();
   if (key == 0) key = kbd.read();
    if (key) Serial.println(key);   
   #endif
  
  //if (key != 0) {
  //   Serial.print("key(hex)=");Serial.println(key,HEX);
  //}
  msg_indx = 0;
  done = false; i = 0;
  if (tx_delay_10ms == 0 ){
     while ( !done ) {
        if ( btn[i].value ) {
           Serial.print("index=");Serial.print(i);Serial.print(" : ");Serial.println(btn[i].value);
           switch(btn[i].value){
           #ifdef MH1_BTN
              case '1': radiate_msg("RGMH1"); break;
              case '2': radiate_msg("RMH14"); break;
              case '3': radiate_msg("RET_1"); break;
           #endif
           #ifdef MH2_BTN
              case '1': radiate_msg("RMH21"); break;
              case '2': radiate_msg("RMH22"); break;
              case '3': radiate_msg("RET_1"); break;             
           #endif
          } 
             btn[i].value= 0;
             done = true;
             tx_delay_10ms = 150;
        }
   
        else {
           i++;
           if (i >= MAX_BTN) done = true;
        } 
        
     } 
     #ifdef K_BTN
     switch(key){
        case '1': radiate_msg("RGWC_"); break;
        case '2': radiate_msg("RET_1"); break;
        case '3': radiate_msg("RPOLK"); break;   
        case '4': radiate_msg("RGMH2"); break;     
        case '5': radiate_msg("RPARV"); break;   
        case '6': radiate_msg("RGMH1"); break;   
        case '7': radiate_msg("RGKHH"); break;   
        case '8': radiate_msg("RGTUP"); break;   
        case '9': radiate_msg("RGKOK"); break;   
        case '*': radiate_msg("xxxxx"); break;   
        case '0': radiate_msg("xxxxx"); break;   
        case '#': radiate_msg("RGBRD"); break;   
     }  
     #endif

  }  
}
void radiate_msg( char *rf69_msg ) {
    char rf69_packet[20] = "";
    byte i;
    
    if (rf69_msg[0] != 0){
       rf69_packet[0] = SM_START;
       rf69_packet[1] = SM_BROADCAST;
       for (i=0; i<6; i++) rf69_packet[i+2]= rf69_msg[i];  //msg_table[msg_indx].msg[i];
       i++;
       if (i<20-3) {
          rf69_packet[i++] = SM_ASSIGN;
          rf69_packet[i++] = SM_TOGGLE;
          rf69_packet[i++] = SM_END;
          rf69_packet[i] =0;
       }
       rf69.send(rf69_packet, strlen(rf69_packet));
       rf69.waitPacketSent();
       Serial.println(rf69_packet);

       //delay(500);

    }
}

void run_10ms(void){
   Smart.HeartBeat10ms();
   if( Smart.Monitor()) msgReady = true;
   //kbd.scan();
   #if defined(MH1_BTN) || defined(MH2_BTN)
   scan_btn();
   #endif
   #ifdef K_BTN
   kbd.scan();
   qkbd.scan();
   #endif
   if ( tx_delay_10ms ) tx_delay_10ms--;
}

void run_1000ms(){
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


void scan_btn(void){
  byte i;
  for (i=0;i<MAX_BTN;i++){
     if (  digitalRead(btn[i].pin) == LOW ) {
        if ( btn[i].cntr == 0) {
           btn[i].cntr = 20;
        }
        else {
           btn[i].cntr--;
           if (( btn[i].cntr == 0) && (btn[i].value==0)) {
               btn[i].pressed = true;
           }
        }
     }
     else {   //pin = HIGH
        btn[i].cntr = 0;
     }
  }
}
char rd_btn(void){
   byte i=0;
   char btn_pressed=0;
   boolean done=false;

   while ( !done ){
      if (btn[i].pressed) {
         btn_pressed = i + '1';
         btn[i].pressed = false;
         btn[i].value = btn_pressed;
         done = true;        
      }
      if ( ++i == MAX_BTN ) done = true;
   }
   return(btn_pressed);
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
