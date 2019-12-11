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

#define LED_YELLOW    5
#define LED_RED       LED_YELLOW+1
#define LED           13  // onboard blinky

#define CODE_LEN      6
#define ZONE_LEN      4
#define CODE_BUFF_LEN 8


#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <rfm69_support.h>
#include <TaHa.h>
#include <akbd4.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************

akbd4 kbd(0,1,2,3);

char key;
byte yellow_cntr;

// Tasks
TaHa scan_btn_handle;
TaHa radio_send_handle;
TaHa task_1000ms_handle;
// Code buffer
char code_buff[CODE_BUFF_LEN][CODE_LEN];  // ring buffer
char zone_buff[CODE_BUFF_LEN][ZONE_LEN];  // ring buffer
byte code_wr_indx;
byte code_rd_indx;


void setup() {
    byte i;
    while (!Serial); // wait until serial console is open, remove if not tethered to computer
    Serial.begin(SERIAL_BAUD);
    // Smart.begin(SERIAL_BAUD);
    Serial.println("T162_Remote_v2");
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    digitalWrite(LED_YELLOW,HIGH);
    digitalWrite(LED_RED, HIGH);

    // clear code and zone buffers
    for(i=0;i<CODE_BUFF_LEN; i++){
        code_buff[i][0] = 0;
        zone_buff[i][0] = 0;
    }
    code_wr_indx = 0;
    code_rd_indx = 0;

    radio_init(RFM69_CS,RFM69_INT,RFM69_RST, RFM69_FREQ);
    radio_send_msg("T162_Remote");
    
    // ------------------Real time settings---------------------
    scan_btn_handle.set_interval(10,RUN_RECURRING, scan_btn);
    radio_send_handle.set_interval(100,RUN_RECURRING, radio_tx_hanndler);
    task_1000ms_handle.set_interval(1000,RUN_RECURRING, run_1000ms);
}


void loop() {
    byte msg_indx;
    char radio_packet[20] = "";
    byte i; 
    String r_addr;
  
    scan_btn_handle.run();
    radio_send_handle.run();
    task_1000ms_handle.run();

 
    key = kbd.read();
    if (key != 0) {
        Serial.print("key(hex)=");Serial.println(key,HEX);
        msg_indx = 0;
        switch(key){
        case '7': add_code("TK1","RLH_1"); break;
        case '4': add_code("TK1","RPOLK"); break;
        case '1': add_code("TK1","RTERA"); break;
        case '5': 
            add_code("MH1","RKOK3"); 
            add_code("MH1","RKOK4"); 
            break;
 
        case '2': 
            add_code("TK1","RTUP1"); 
            add_code("TK1","RTUP2"); 
            break;
        case '*': add_code("MH2","RKHH1"); break;
        case '3': 
            add_code("MH1","RKOK1"); 
            add_code("MH1","RKOK2"); 
            break;
        case '0': add_code("TK1","RPARV"); break;
        case '8': add_code("TK1","RGWC_"); break;
        case '#': //MH1
            add_code("MH1","RMH11"); 
            add_code("MH1","RMH12"); 
            add_code("MH1","RMH13"); 
            add_code("MH1","RMH14"); 
            break;
        case '9': add_code("MH2","RET_1"); break;
        case '6': 
            add_code("MH2","RMH21"); 
            add_code("MH2","RMH22"); 
            add_code("MH2","RMH23"); 
            break;
        case 'C': add_code("TK1","RPIHA"); break;
        case 'B': add_code("TK1","RTK_1"); break;
        case 'A': add_code("TK1","AUTO1"); break;
        } 
    }  
}


void add_code(char *new_zone, char *new_code){
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

    code_wr_indx = ++code_wr_indx & 0b00000111;   
}

void radiate_msg( char *zone, char *relay_addr ) {
    digitalWrite(LED_RED, LOW); 
    String relay_json = JsonRelayString(zone, relay_addr, "T", "" );
    char rf69_packet[RH_RF69_MAX_MESSAGE_LEN] = "";
    relay_json.toCharArray(rf69_packet, RH_RF69_MAX_MESSAGE_LEN);
    radio_send_msg(rf69_packet);
    Serial.println(rf69_packet);
    digitalWrite(LED_RED, HIGH); 
}

void radio_tx_hanndler(void){
    if (code_buff[code_rd_indx][0] != 0){
        radiate_msg(zone_buff[code_rd_indx],code_buff[code_rd_indx]);
        Serial.print(zone_buff[code_rd_indx]); Serial.println(code_buff[code_rd_indx]);
        code_buff[code_rd_indx][0] = 0;
        code_rd_indx = ++code_rd_indx & 0b00000111; 
        radio_send_handle.delay_task(2000);
    }
}

void scan_btn(void){
    kbd.scan();
    if ( yellow_cntr  == 0 )
        digitalWrite(LED_YELLOW, HIGH);  
    else
        yellow_cntr--;
}

void run_1000ms(void){
    for (uint8_t i = 0; i < 4; i++){  
        Serial.print(kbd.rd_analog(i));Serial.print('-');
    }
    Serial.println();   
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
