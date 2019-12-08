
#include "Arduino.h"
#include <SPI.h>
//#include <RH_RF69.h>
#include "rfm.h"
//#include <Secret.h>
#include <ArduinoJson.h>
// Adafruit M0+RFM60 Feather:
// #8 - used as the radio CS (chip select) pin
// #3 - used as the radio GPIO0 / IRQ (interrupt request) pin.
// #4 - used as the radio Reset pin

#define RADIO_MSG_LEN   60
#define RFM69_CS        8
#define RFM69_INT       3
#define RFM69_IRQN      0  // Pin 2 is IRQ 0!
#define RFM69_RST       4
#define RFM69_FREQ      434.0   //915.0
#define RFM69_TX_IVAL_100ms  20



//void Init_RFM69(byte rfm_rst_pin,float rfm_freq);
//void RadiateMsg(char *rf69_msg );
//RH_RF69 rf69(RFM69_CS, RFM69_IRQN);
StaticJsonDocument<160> load_json;

void setup() {
    delay(2000);
    Serial.begin(115200);
    //while (!SERIAL) ;  // Wait for serial terminal to open port before starting program
    Serial.println("RadioRelayJson");
    radio_init(RFM69_CS,RFM69_IRQN,RFM69_RST, RFM69_FREQ);
    radio_send_msg("RadioRelayJson");

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
                // Serial.println(zone);Serial.println(sub_addr);Serial.println(value);Serial.println(remark);

                
            }
 
        } else {
            Serial.println("Receive failed");
        }
    }
    else{
        //Serial.print('.');
        delay(500);
    }

}
