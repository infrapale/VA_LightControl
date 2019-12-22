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
#define MAX_BTN       6
#define CODE_LEN      6
#define ZONE_LEN      4
#define FUNC_LEN      4
#define CODE_BUFF_LEN 32
#define CODE_BUFF_LEN_MASK 0b00011111;

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <avr/wdt.h>   /* Header for watchdog timers in AVR */
#include <rfm69_support.h>
#include <TaHa.h>
#include <Pin_Button.h>
#include <avr/wdt.h>   /* Header for watchdog timers in AVR */

#ifdef K_BTN
#include <akbd.h>
akbd kbd(A0);
akbd qkbd(A1);
//led_blink leds(6,7,3,9,5,4);
#endif
// Tasks
TaHa scan_btn_handle;
TaHa radio_send_handle;
// Code buffer
char code_buff[CODE_BUFF_LEN][CODE_LEN];  // ring buffer
char zone_buff[CODE_BUFF_LEN][ZONE_LEN];  // ring buffer
char func_buff[CODE_BUFF_LEN][FUNC_LEN];
byte code_wr_indx;
byte code_rd_indx;

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************


int16_t packetnum = 0;  // packet counter, we increment per xmission
PinBtn butt[MAX_BTN];
 
void setup() {
    byte i;
    
    wdt_disable();  /* Disable the watchdog and wait for more than 2 seconds */
    delay(2000);
    while (!Serial); // wait until serial console is open, remove if not tethered to computer
    Serial.begin(SERIAL_BAUD);
    wdt_enable(WDTO_2S);
    butt[0].Init(3,'1');
    butt[1].Init(4,'2');
    butt[2].Init(5,'3');
    butt[3].Init(6,'4');
    butt[4].Init(7,'5');
    butt[5].Init(8,'6');

    // clear code and zone buffers
    for(i=0;i<CODE_BUFF_LEN; i++){
        code_buff[i][0] = 0;
        zone_buff[i][0] = 0;
    }
    code_wr_indx = 0;
    code_rd_indx = 0;

    #ifdef K_BTN
    kbd.begin();
    qkbd.begin();
    #endif

    pinMode(LED, OUTPUT);     

    radio_init(RFM69_CS,RFM69_INT,RFM69_RST, RFM69_FREQ);
    radio_send_msg("T168_Wall_Button");

    // ------------------Real time settings---------------------
    scan_btn_handle.set_interval(10,RUN_RECURRING, scan_btn);
    radio_send_handle.set_interval(100,RUN_RECURRING, radio_tx_hanndler);

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Main Loop
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void loop() {
  
    scan_btn_handle.run();
    radio_send_handle.run();
    wdt_reset();
    //-------------------------------------------------------
    // Read pressed buttons
    // if button is preseed add a command code to the ring buffer
    //-------------------------------------------------------
    #if defined(MH1_BTN) || defined(MH2_BTN)
         mini_terminals();
    #endif
    #ifdef K_BTN
         maxi_terminals();
    #endif
        
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
            case '1': add_code("MH1","RMH11","T"); break;
            case '2': add_code("MH1","RMH12","T"); break;
            case '3': add_code("MH1","RMH13","T"); break;
            #endif
            #ifdef MH2_BTN
            case '1': add_code("MH2","RMH21","T"); break;
            case '2': add_code("MH2","RMH22","T"); break;
            case '3': add_code("MH2","RET_1","T"); break;  
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
       case '1': add_code("MH2","RWC_2","T"); 
                 break;        
       case '2': add_code("MH2","RET_1","T"); 
                 break;
       case '3': add_code("TK1","RPOLK","T"); 
                 break;
       case '4': add_code("MH2","RMH21","T"); 
                 add_code("MH2","RMH22","T"); 
                 break;     
       case '5': add_code("TK1","RPARV","T"); 
                 break;   
       case '6': add_code("MH1","RMH11","T"); 
                 add_code("MH1","RMH12","T"); 
                 add_code("MH1","RMH13","T"); 
                 break;   
       case '7': add_code("MH2","RKHH2","T"); 
                 add_code("MH2","RPSH1","T");
                 break;   
       case '8': add_code("TK1","RTUP1","T"); 
                 add_code("TK1","RTUP2","T");
                 break;   
       case '9': add_code("MH1","RKOK1","T"); 
                 add_code("MH1","RKOK2","T"); 
                 add_code("MH1","RKOK3","T"); 
                 add_code("MH1","RKOK4","T"); 
                 add_code("MH1","RKOK5","T"); 
                  break;   
       case '*': add_code("MH1","xxxxx","T"); break;   
       case '0': add_code("MH1","*.OFF","0"); 
                 add_code("MH2","*.OFF","0");
                 add_code("TK1","*.OFF","0");
                 break;   
       case '#': add_code("MH1","RKOK3","T"); 
                 add_code("MH1","RKOK4","T"); 
                 add_code("MH1","RKOK5","T"); 
                 break;   
    }  
    #endif    
}


void add_code(const char *new_zone, const char *new_code, const char *new_func){
    int i;
    for(i = 0; i < CODE_LEN; i++) {
        if (new_code[i] != 0) { 
            code_buff[code_wr_indx][i] = new_code[i];
        } 
        else {
           code_buff[code_wr_indx][i] =0;
        }   
    }
    for(i = 0; i < ZONE_LEN; i++) {
        if (new_code[i] != 0) { 
            zone_buff[code_wr_indx][i] = new_zone[i];
        } 
        else {
           zone_buff[code_wr_indx][i] =0;
        }   
    }
    for(i = 0; i < FUNC_LEN; i++) {
        if (new_func[i] != 0) { 
            func_buff[code_wr_indx][i] = new_func[i];
        } 
        else {
           func_buff[code_wr_indx][i] =0;
        }   
    }
 

    code_wr_indx = ++code_wr_indx & CODE_BUFF_LEN_MASK;   
}

void radiate_msg( const char *zone, const char *relay_addr, char *func ) {
    String relay_json = JsonRelayString(zone, relay_addr, func, "" );
    char rf69_packet[RH_RF69_MAX_MESSAGE_LEN] = "";
    relay_json.toCharArray(rf69_packet, RH_RF69_MAX_MESSAGE_LEN);
    radio_send_msg(rf69_packet);
    Serial.println(rf69_packet);
}

void radio_tx_hanndler(void){
    if (code_buff[code_rd_indx][0] != 0){
        radiate_msg(zone_buff[code_rd_indx],code_buff[code_rd_indx],func_buff[code_rd_indx]);
        Serial.print(zone_buff[code_rd_indx]); Serial.println(code_buff[code_rd_indx]);
        code_buff[code_rd_indx][0] = 0;
        code_rd_indx = ++code_rd_indx & CODE_BUFF_LEN_MASK; 
        radio_send_handle.delay_task(2000);
    }
}

void scan_btn(void){
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
}
