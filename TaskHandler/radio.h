#ifndef __RADIO_H__
#define __RADIO_H__
#include "Arduino.h"
#include <RH_RF69.h>

#define RFM69_CS        8
#define RFM69_IRQN      3  // Pin 2 is IRQ 0!
#define RFM69_RST       4
#define RFM69_FREQ      434.0   //915.0

struct rfm69_struct {
    RH_RF69 *rf69;
    byte rfm_cs_pin; 
    byte rfm_irqn_pin; 
    byte rfm_rst_pin;
    float rfm_freq;
};
void radio_init(byte rfm_cs_pin, byte rfm_irqn_pin, byte rfm_rst_pin, float rfm_freq);
void radio_send_msg(char *rf69_msg );

  
#endif
