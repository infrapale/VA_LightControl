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

void parse_broadcast_json( String *Json, json_broadcast_struct *d){

// {"C":"BC","P":"Param","V":"Value"



    d.category[0] = 0;
    d.param[0] = 0;
    d.value[0] = 0; 
  
}



/*
  {"menu": {
  "id": "file",
  "value": "File",
  "popup": {
    "menuitem": [
      {"value": "New", "onclick": "CreateNewDoc()"},
      {"value": "Open", "onclick": "OpenDoc()"},
      {"value": "Close", "onclick": "CloseDoc()"}
    ]
  }
}}
 */
