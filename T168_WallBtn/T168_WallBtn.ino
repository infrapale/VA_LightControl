#define MH1_BTN
//#define MH2_BTN
//#define K_BTN
//#define TK_RELAY

#define MENU_DATA

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
#define RFM69_TX_IVAL_100ms  20;
#define LED           13  // onboard blinky
#define MAX_BTN   6
#define CODE_LEN  6
#define CODE_BUFF_LEN 8
#include <Arduino.h>
#include <Wire.h>
#include <RH_RF69.h>
#include <SPI.h>
#include <VillaAstrid.h>
#include <SimpleTimer.h> 
#include <Secret.h>

#ifdef K_BTN
#include <akbd.h>
akbd kbd(A0);
akbd qkbd(A1);
//led_blink leds(6,7,3,9,5,4);
#endif
// Code buffer
char code_buff[CODE_BUFF_LEN][CODE_LEN];  // ring buffer
byte code_wr_indx;
byte code_rd_indx;
int rfm68_tx_ival_cntr;
#include "PinBtn.h"

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************


int16_t packetnum = 0;  // packet counter, we increment per xmission
RH_RF69 rf69(RFM69_CS, RFM69_INT);
SimpleTimer timer;
PinBtn butt[MAX_BTN];

void run_10ms(void);
void run_100ms(void);
void add_code(char *new_code);
void radiate_msg(char *rf69_msg );
void add_code(char *new_code);

byte tx_delay_10ms;
 
void setup() {
  byte i;
  while (!Serial); // wait until serial console is open, remove if not tethered to computer
  Serial.begin(SERIAL_BAUD);
  
  butt[0].Init(3,'1');
  butt[1].Init(4,'2');
  butt[2].Init(5,'3');
  butt[3].Init(6,'4');
  butt[4].Init(7,'5');
  butt[5].Init(8,'6');

  // clear code buffer
  for(i=0;i<CODE_BUFF_LEN; i++){
      code_buff[i][0] = 0;
  }
  code_wr_indx = 0;
  code_rd_indx = 0;
  rfm68_tx_ival_cntr =0;

  #ifdef K_BTN
  kbd.begin();
  qkbd.begin();
  #endif

  pinMode(LED, OUTPUT);     

  //==========================================================
  // Initialize rf69
  //==========================================================
  // Hard Reset the RFM module
  // Serial.println("Arduino RFM69HCW Transmitter");
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);
  
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

  // in secret.h          1234567890123456
  //uint8_t rfm69_key[] ="Xyzabde123456789"; //exactly the same 16 characters/bytes on all nodes!
  rf69.setEncryptionKey(rfm69_key);
  Serial.print("RFM69 rf69 @");  Serial.print((int)RFM69_FREQ,DEC);  Serial.println(" MHz");

  //==========================================================
  // Real time settings
  //==========================================================
  timer.setInterval(10, run_10ms);
  timer.setInterval(100, run_100ms);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Main Loop
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void loop() {
     
    timer.run();
    
    //-------------------------------------------------------
    // Read preseed buttons
    // if button is preseed add a command code to the ring buffer
    //-------------------------------------------------------
    #if defined(MH1_BTN) || defined(MH2_BTN)
         mini_terminals();
    #endif
    #ifdef K_BTN
         maxi_terminals();
    #endif
        
    #ifdef K_BTN
    btn = qkbd.read();
    if (btn == 0) btn = kbd.read();
    if (btn) Serial.println(btn);   
    #endif
    //-------------------------------------------------------
  
    //-------------------------------------------------------
    // Send message vir RFM69
    // Each message must be separated by an interval
    // Interval is set in RFM69_TX_IVAL_100ms as 100ms multiplier
    //-------------------------------------------------------
    if(rfm68_tx_ival_cntr == 0){
        if(code_buff[code_rd_indx][0] != 0){
            radiate_msg(code_buff[code_rd_indx]);
            code_buff[code_rd_indx][0] = 0;
            code_rd_indx = ++code_rd_indx & 0b00000111;   
            rfm68_tx_ival_cntr = RFM69_TX_IVAL_100ms;          
        }
    }
    //-------------------------------------------------------
    // infinite loop..
    //-------------------------------------------------------
}

void mini_terminals(void){
    // if button is preseed add a command code to the ring buffer

    char btn;
    int btns;
    #if defined(MH1_BTN) || defined(MH2_BTN)
    btns = 3;
    #endif
    
    for(int i= 0; i < btns; i++){
        btn = butt[i].Read();  //   rd_btn();
        if(btn) break;
    }
    if (btn != 0) {
        //Serial.print("button= ");Serial.println(btn);
        switch(btn){
            #ifdef MH1_BTN
            case '1': add_code("RGMH1"); break;
            case '2': add_code("RMH14"); break;
            case '3': add_code("RET_1"); break;
            #endif
            #ifdef MH2_BTN
            case '1': add_code("RMH21"); break;
            case '2': add_code("RMH22"); break;
            case '3': add_code("RET_1"); break;  
            #endif
        }           
       
    } 
}

void maxi_terminals(void){
    #ifdef K_BTN
    char btn = qkbd.read();
    if (btn == 0) btn = kbd.read();
    if (btn) Serial.println(btn);   
  
    switch(btn){
       case '1': add_code("RGWC_"); break;        
       case '2': add_code("RET_1"); break;
       case '3': add_code("RPOLK"); break;
       case '4': add_code("RGMH2"); break;     
       case '5': add_code("RPARV"); break;   
       case '6': add_code("RGMH1"); break;   
       case '7': add_code("RGKHH"); break;   
       case '8': add_code("RGTUP"); break;   
       case '9': add_code("RGKOK"); break;   
       case '*': add_code("xxxxx"); break;   
       case '0': add_code("xxxxx"); break;   
       case '#': add_code("RGBRD"); break;   
    }  
    #endif    
}

void add_code(char *new_code){
    int i;
    for(i = 0; i < CODE_LEN; i++) {
        code_buff[code_wr_indx][i] = new_code[i];
        if(new_code[i] == 0) break; 
    }
    for(; i < CODE_LEN; i++) code_buff[code_wr_indx][i] = 0;
    code_wr_indx = ++code_wr_indx & 0b00000111;   
}

#define RADIO_MSG_LEN 70
void radiate_msg( char *rf69_msg ) {
    String relay_json;
    char rf69_packet[RADIO_MSG_LEN] = "";
    byte i;

    relay_json = JsonRelayString("VA",rf69_msg,"T","");
    relay_json.toCharArray(rf69_packet, RADIO_MSG_LEN);
 
    rf69.send(rf69_packet, strlen(rf69_packet));
    rf69.waitPacketSent();
}

void run_10ms(void){
   //Smart.HeartBeat10ms();
   //if( Smart.Monitor()) msgReady = true;
   //kbd.scan();
   #if defined(MH1_BTN) || defined(MH2_BTN)
   for(int i= 0;i<MAX_BTN;i++){
       butt[i].Scan();
   }    
   //scan_btn();
   #endif
   #ifdef K_BTN
   kbd.scan();
   qkbd.scan();
   #endif
   if ( tx_delay_10ms ) tx_delay_10ms--;
}
void run_100ms(void){
    if(rfm68_tx_ival_cntr) rfm68_tx_ival_cntr--;

}
