
#include "Arduino.h"
#include <TaHa.h>
#include <SPI.h>
#include <rfm69_support.h>
#include <ArduinoJson.h>
#include <avr/wdt.h>   /* Header for watchdog timers in AVR */

//#include "relay_dict.h"
//#include "relay_com.h"

//#define M0_RFM69    //for testing on M0-RFM69
#define MINI_RFM69
#ifdef  M0_RFM69
    #define RFM69_CS        8
    #define RFM69_INT       3
    #define RFM69_IRQN      0  // Pin 2 is IRQ 0!
    #define RFM69_RST       4
#endif

#ifdef  MINI_RFM69  
    #define RFM69_CS        10
    #define RFM69_INT       2
    #define RFM69_IRQN      0  // Pin 2 is IRQ 0!
    #define RFM69_RST       9
#endif

#define RFM69_FREQ      434.0   //915.0
#define MAX_TX_LEN      20
#define INP_BUFF_LEN    8
#define INP_BUFF_MASK   0b00000111
#define RADIO_MSG_LEN   62

char inp_buff[INP_BUFF_LEN][3];
uint8_t inp_wr_ptr;
uint8_t inp_rd_ptr;
int8_t cindx; 
char json_str[RADIO_MSG_LEN];

TaHa rd_serial_handle;
TaHa send_radio_handle;

void send_radio(void);

void setup() {
    wdt_disable();  /* Disable the watchdog and wait for more than 2 seconds */
    delay(4000);
    wdt_enable(WDTO_2S);  /* Enable the watchdog with a timeout of 2 seconds */

    Serial.begin(9600);
    //while (!SERIAL) ;  // Wait for serial terminal to open port before starting program
    Serial.println("TK_radio_unit");

    radio_init(RFM69_CS,RFM69_INT,RFM69_RST, RFM69_FREQ);
    radio_send_msg("TK_radio_unit");
    inp_wr_ptr = 0;
    inp_rd_ptr = 0;
    cindx =0; 
    for(uint8_t i= 0;i<3;i++) inp_buff[inp_wr_ptr][i]= 0;
    rd_serial_handle.set_interval(1,RUN_RECURRING, read_serial);
    send_radio_handle.set_interval(500,RUN_RECURRING, send_radio);

}

void loop() {
    rd_serial_handle.run();
    send_radio_handle.run();
    wdt_reset();
}

void read_serial(){
    boolean do_continue = true;
    if (Serial.available() >0) {
        //Serial.println("loop in if");
        char c = Serial.read();  
        //Serial.print(c);
        //Serial.println(cindx);
        switch(cindx){
            case 0: if (c!='<') do_continue = false; break;
            case 1: if (c!='#') do_continue = false; break;
            case 2: if (c!='I') do_continue = false; break;
            case 3: inp_buff[inp_wr_ptr][0] = c; break;
            case 4: inp_buff[inp_wr_ptr][1] = c; break;
            case 5: if (c!='=') do_continue = false; break;
            case 6: inp_buff[inp_wr_ptr][2] = c; break;
            case 7: if (c!='>') do_continue = false; break;
            case 8: if (c!='\r') do_continue = false; break;
            case 9: 
                if (c=='\n') {
                    Serial.print("All received: ");
                    for(uint8_t i = 0;i<3;i++)  Serial.print(inp_buff[inp_wr_ptr][i]); 
                    Serial.println();
                    inp_wr_ptr = (inp_wr_ptr++ & INP_BUFF_MASK);          
                } else inp_buff[inp_rd_ptr][0] =0;
                do_continue = false;
                break;
        }
        cindx++;
        if (cindx > MAX_TX_LEN -2) do_continue = false;              
        if(!do_continue) cindx = 0; 
    }
}

void send_radio(void){
    const char zone[8];
    const char event[10];
    const char value[10];
    boolean verified = false;
    
    if (inp_buff[inp_rd_ptr][0] != 0){
        for(uint8_t i = 0;i<3;i++)  Serial.print(inp_buff[inp_rd_ptr][i]);
        Serial.println();

        switch (inp_buff[inp_rd_ptr][0]) {   // Unit Address
        case '1':
            switch(inp_buff[inp_rd_ptr][1]) {  // index
            case '1':  
                strcpy(zone, "TK"); 
                strcpy(event, "Teras"); 
                verified = true;
                break;
            case '2':  
                strcpy(zone, "TK");
                strcpy(event, "Piha1"); 
                verified = true;
                break;
            case '3':  
                strcpy(zone, "TK");
                strcpy(event, "Piha2"); 
                verified = true;
                break;
            case '4':  
                strcpy(zone, "TK");
                strcpy(event, "Etuovi"); 
                verified = true;
                break;
            case '5':  
                strcpy(zone, "TK");
                strcpy(event, "Control"); 
                verified = true;
                break;
            }
            break;
        }
        
        Serial.print("zone=");Serial.println(zone);
        if(verified){
            switch (inp_buff[inp_rd_ptr][2]){
            case 'L':
                strcpy(value,"PIR_Low"); break;
            case 'H':
                strcpy(value,"PIR_High"); break;
            default:
                verified = false; break;                 
            }
        }
        Serial.println(zone);Serial.println(event);Serial.println(value);
        if (verified) {
            JsonInputString(json_str,zone,event,value,""); 
            Serial.println(json_str);
            radio_send_msg(json_str);
        }
        inp_buff[inp_rd_ptr][0] = 0;
        inp_rd_ptr = (inp_rd_ptr++ & INP_BUFF_MASK);
    }
}

void JsonInputString(char *result_str, const char *zone, const char *addr, const char *value, const char *remark ){
    result_str[0] = 0;
    Serial.println(zone);
    strcat(result_str,"{\"Z\":\"");
    strcat(result_str,zone);
    strcat(result_str,"\",");
    strcat(result_str,"\"E\":\"");
    strcat(result_str,addr);  
    strcat(result_str,"\",");
    strcat(result_str,"\"V\":\"");
    strcat(result_str,value);
    strcat(result_str,"\",");
    strcat(result_str,"\"R\":\"");
    strcat(result_str,remark);
    strcat(result_str,"\"}");
}
