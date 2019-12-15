
#include "Arduino.h"
#include <SPI.h>
#include <rfm69_support.h>
#include <ArduinoJson.h>

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



StaticJsonDocument<160> load_json;

void setup() {
    delay(2000);
    Serial.begin(9600);
    //while (!SERIAL) ;  // Wait for serial terminal to open port before starting program
    Serial.println("RadioRelayJson");
    Serial.println("GitHub: infrpale/VA_LightControl/RadioRelayJson 2019");
    radio_init(RFM69_CS,RFM69_INT,RFM69_RST, RFM69_FREQ);
    radio_send_msg("RadioRelayJson");
    InitSoftCom();
    relay_dict_debug();
    
  }

void loop(void) {
    uint8_t i;
    if (radio_check_available_msg()) {
        Serial.print("! available!");
        // Should be a message for us now   
        char json_msg[RH_RF69_MAX_MESSAGE_LEN];
        uint8_t len;
        len = radio_read_msg(json_msg, RH_RF69_MAX_MESSAGE_LEN);
        if (len > 0) {;
            Serial.println();
            Serial.println(json_msg);
            
            DeserializationError error = deserializeJson(load_json, json_msg);
            if (error) {
                Serial.print(F("deserializeJson() failed: ")); Serial.println(error.c_str());
            }
            else {
                const char* zone = load_json["Z"];
                const char* sub_addr = load_json["S"];
                const char* value = load_json["V"];
                const char* remark = load_json["R"];
                Serial.println(zone);Serial.println(sub_addr);Serial.println(value);Serial.println(remark);
                uint8_t indx =  find_zone_name(zone, sub_addr);
                Serial.println(indx);
                if (indx > 0){
                    SendSoftcomRelayMsg(get_relay_unit(indx),get_relay_indx(indx),value[0]);
                }
            }
 
        } else {
            Serial.println("Receive failed");
        }
    }
    else{
        // Serial.print('.');
        delay(500);
    }

}
