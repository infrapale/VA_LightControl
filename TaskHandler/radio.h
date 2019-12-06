#ifndef __RADIO_H__
#define __RADIO_H__
#include "Arduino.h"
#include <RH_RF69.h>

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
