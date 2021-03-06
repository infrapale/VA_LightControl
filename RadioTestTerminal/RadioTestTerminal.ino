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

#define RFM69_CS      9
#define RFM69_INT     6
#define RFM69_RST     11
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
#include <rfm69_support.h>
#include <TaHa.h>
// Tasks

TaHa radio_send_handle;
TaHa simulate_handle;

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
 
void setup() {
    byte i;
    
    delay(2000);
    while (!Serial); // wait until serial console is open, remove if not tethered to computer
    Serial.begin(SERIAL_BAUD);

    // clear code and zone buffers
    for(i=0;i<CODE_BUFF_LEN; i++){
        code_buff[i][0] = 0;
        zone_buff[i][0] = 0;
    }
    code_wr_indx = 0;
    code_rd_indx = 0;

    pinMode(LED, OUTPUT);     

    radio_init(RFM69_CS,RFM69_INT,RFM69_RST, RFM69_FREQ);
    radio_send_msg("T168_Wall_Button");

    // ------------------Real time settings---------------------
    simulate_handle.set_interval(10000,RUN_RECURRING, simulate_task);
    radio_send_handle.set_interval(100,RUN_RECURRING, radio_tx_hanndler);

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Main Loop
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void loop() {
  
    radio_send_handle.run();
    simulate_handle.run();
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


void simulate_task(void)
{
    static uint8_t msg_indx = 0;

    switch(msg_indx)
    {
        case 0: add_code("MH1","RMH11","T"); break;
        case 1: add_code("MH1","RMH12","T"); break;
        case 2: add_code("MH1","RMH13","T"); break;          
    }
    msg_indx++;
    if (msg_indx > 2) msg_indx = 0;
}

void mini_terminals(void){
    // if button is preseed add a command code to the ring buffer

    char btn;
    int btns;
    #if defined(MH1_BTN) || defined(MH2_BTN)
    btns = 3;
    #endif
    
    for(int i= 0; i < btns; i++){
        // btn = butt[i].Read();  //   rd_btn();
        //if(btn) break;
    }
    btn = 0;
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
