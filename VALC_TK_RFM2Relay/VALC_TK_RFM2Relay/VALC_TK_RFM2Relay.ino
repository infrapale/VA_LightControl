#include "Arduino.h"
#include <SPI.h>
#include <rfm69_support.h>
#include <ArduinoJson.h>
#include <avr/wdt.h>   /* Header for watchdog timers in AVR */

#include "relay_dict.h"
#include "relay_com.h"


// Adafruit M0+RFM60 Feather:
// #8 - used as the radio CS (chip select) pin
// #3 - used as the radio GPIO0 / IRQ (interrupt request) pin.
// #4 - used as the radio Reset pin

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
#define SERIAL_RX_BUF_LEN  16


StaticJsonDocument<160> load_json;
uint8_t serial_rx_state = 0;
uint8_t serial_rx_idx   = 0;
char  serial_rx_buf[SERIAL_RX_BUF_LEN];


void setup() {
    wdt_disable();  /* Disable the watchdog and wait for more than 2 seconds */
    delay(2000);
    wdt_enable(WDTO_2S);  /* Enable the watchdog with a timeout of 2 seconds */

    Serial.begin(9600);
    //while (!SERIAL) ;  // Wait for serial terminal to open port before starting program
    Serial.println("VALC_TK_RFM2Relay");
    Serial.println("GitHub: infrpale/VA_LightControl");
    radio_init(RFM69_CS,RFM69_INT,RFM69_RST, RFM69_FREQ);
    radio_send_msg("VALC_TK_RFM2Relay");
    InitSoftCom();
    relay_dict_debug();
    
  }

void loop(void) {
    uint8_t i;
    char r_unit;
    char c;
    char io_addr[6];
    char pir_value[2];
    wdt_reset();
    
    if (radio_check_available_msg()) {
        // Serial.print("! available!");
        // Should be a message for us now   
        char json_msg[RH_RF69_MAX_MESSAGE_LEN];
        uint8_t len;
        len = radio_read_msg(json_msg, RH_RF69_MAX_MESSAGE_LEN);
        if (len > 0) {;
            // Serial.println();
            // Serial.println(json_msg);
            
            DeserializationError error = deserializeJson(load_json, json_msg);
            if (error) {
                // Serial.print(F("deserializeJson() failed: ")); 
                // Serial.println(error.c_str());
            }
            else {
                const char* zone = load_json["Z"];
                const char* sub_addr = load_json["S"];
                const char* value = load_json["V"];
                const char* remark = load_json["R"];
                // Serial.println(zone);Serial.println(sub_addr);Serial.println(value);Serial.println(remark);
                uint8_t indx =  find_zone_name(zone, sub_addr);
                // Serial.println(indx);
                if (indx > 0){
                    r_unit = get_relay_unit(indx);
                    if (((r_unit > '0') && (r_unit < '9')) || (r_unit == '*')){
                        SendSoftcomRelayMsg(get_relay_unit(indx),get_relay_indx(indx),value[0]);
                    }
                } 
            }
 
        } else {
            // Serial.println("Receive failed");
        }
    }
    else{
        // Serial.print('.');
    }
    //uint8_t serial_rx_state = 0;
    //uint8_t serial_rx_idx   = 0;
    //uint8_t serial_rx_buf[SERIAL_RX_BUF_LEN];
    if (Serial.available() > 0){
        c= Serial.read();
        switch (serial_rx_state){
            case 0:
                if (c == '<') {
                    serial_rx_idx = 0;
                    serial_rx_buf[serial_rx_idx++] = c;
                    serial_rx_state = 1;
                }
                break;
            case 1:
                if (serial_rx_idx < SERIAL_RX_BUF_LEN -2) {
                    serial_rx_buf[serial_rx_idx++] = c;
                }
                if (c == '>') {
                    serial_rx_buf[serial_rx_idx] = 0x00;
                    serial_rx_state = 2;
                }
                break;
            case 2:
                serial_rx_state = 0;
                serial_rx_idx = 0;
                strcpy(io_addr,"PIR");
                io_addr[3] = serial_rx_buf[4];
                io_addr[4] = serial_rx_buf[5];
                io_addr[5] = 0x00;
                pir_value[0] = serial_rx_buf[6];
                pir_value[1] = 0x00;
                 
                radiate_msg( "TK", io_addr, pir_value );
                serial_rx_state = 0;
                break;
        }      
    }
}

void radiate_msg( const char *zone, const char *io_addr, char *io_value ) {
    String relay_json = JsonRelayString(zone, io_addr, io_value, "" );
    char rf69_packet[RH_RF69_MAX_MESSAGE_LEN] = "";
    relay_json.toCharArray(rf69_packet, RH_RF69_MAX_MESSAGE_LEN);
    radio_send_msg(rf69_packet);
    //Serial.println(rf69_packet);
}
