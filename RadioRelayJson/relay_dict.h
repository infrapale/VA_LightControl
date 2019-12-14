#ifndef __RELAY_DICT_H__
#define __RELAY_DICT_H__
#include "Arduino.h"


struct relay_dict_struct {
    char zone[4];
    char name[6];
    char relay_unit;
    char relay_indx;   
};


void relay_dict_debug(void);
uint8_t find_zone_name(const char *zone, const char *name);
char get_relay_unit(uint8_t indx);
char get_relay_indx(uint8_t indx);
#endif
