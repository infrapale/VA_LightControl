#ifndef __JSON_H__
#define __JSON_H__
void make_json_relay_array(char *buff, char category, char *zone, char addr, char command);

struct json_broadcast_struct {
    char category[8]; 
    char param[8];
    char value[8];
};
#endif
