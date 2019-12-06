
#include "radio.h"
#include <Secret.h>

//RH_RF69 rf69(RFM69_CS, RFM69_IRQN);
static rfm69_struct rfm69;


void radio_init(byte rfm_cs_pin, byte rfm_irqn_pin, byte rfm_rst_pin,float rfm_freq)
{   
    //static RH_RF69 _rf69(rfm_cs_pin, rfm_irqn_pin);
    rfm69.rf69 =  new RH_RF69(rfm_cs_pin, rfm_irqn_pin);
    rfm69.rfm_cs_pin = rfm_cs_pin;
    rfm69.rfm_irqn_pin = rfm_irqn_pin;
    rfm69.rfm_rst_pin = rfm_rst_pin;
    rfm69.rfm_freq = rfm_freq;
    pinMode(rfm69.rfm_rst_pin, OUTPUT);
    digitalWrite(rfm69.rfm_rst_pin, LOW);
    //rf69.RH_RF69(rfm_cs_pin, rfm_irqn_pin);

     // manual reset
     digitalWrite(rfm69.rfm_rst_pin, HIGH);
     delay(100);
     digitalWrite(rfm69.rfm_rst_pin, LOW);
     delay(100);
  
     if (!rfm69.rf69->init()) {
         Serial.println("RFM69 rf69 init failed");
         while (1);
     }
     Serial.println("RFM69 rf69 init OK!");
  
     // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
     // No encryption
     if (!rfm69.rf69->setFrequency(rfm69.rfm_freq)) {
          Serial.println("setFrequency failed");
     }

     // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
     // ishighpowermodule flag set like this:
     rfm69.rf69->setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

     // in secret.h          1234567890123456
     //uint8_t rfm69_key[] ="Xyzabde123456789"; //exactly the same 16 characters/bytes on all nodes!
     rfm69.rf69->setEncryptionKey(rfm69_key);
     Serial.print("RFM69 rf69 @");  Serial.print((int)rfm_freq,DEC);  Serial.println(" MHz");
    
} 

void radio_send_msg( char *rf69_msg ) {
    rfm69.rf69->waitPacketSent();
    //Serial.print("radiate_msg:");
    //Serial.println(rf69_msg);
    
    rfm69.rf69->send((uint8_t*)rf69_msg, strlen(rf69_msg));
}

boolean radio_check_available_msg(){
   return rf69.available() 
}

uint8_t rd_buf[RH_RF69_MAX_MESSAGE_LEN];

boolean radio_read_mesage(char *inp_buf){
   uint8_t len = sizeof(inp_buf);
   boolean result = false;
   if (radio_check_available_msg()) {
       if (rf69.recv(inp_buf, &len)) {
           if (len> 0){
               inp_buf[len] = 0;
               result = true;
               Serial.print("Received [");
               Serial.print(len);
               Serial.print("]: ");
               Serial.println((char*)buf);
               Serial.print("RSSI: ");
               Serial.println(rf69.lastRssi(), DEC);
           }
       }
   }
}
