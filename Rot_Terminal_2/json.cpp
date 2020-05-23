#include "Arduino.h"
void make_json_relay_array(char *buff, char category, char *zone, char addr, char command){
    byte i= 0;

    strcpy(&buff[i],"{\"C\":\"");
    i =  strlen(buff);
    buff[i++] = category;
    strcpy(&buff[i],"\",");
    i =  strlen(buff);
    strcpy(&buff[i],"\"Z\":\"");
    i =  strlen(buff);
    strcpy(&buff[i],zone);
    i =  strlen(buff); 
    strcpy(&buff[i],"\",");
    i =  strlen(buff);
    strcpy(&buff[i],"\"A\":\"");
    i =  strlen(buff);
    buff[i++] = addr;
    strcpy(&buff[i],"\",");
    i =  strlen(buff);
    strcpy(&buff[i],"\"C\":\"");
    i =  strlen(buff);
    buff[i++] = command;
    strcpy(&buff[i],"\"}");
          
}
