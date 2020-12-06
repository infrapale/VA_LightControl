
#include "relay_dict.h"

//#define RADIO_ZONE_MH1
#define RADIO_ZONE_MH2
//#define RADIO_ZONE_TK1


relay_dict_struct relay_dict[]={
    {"DMY","DUMMY",'0','0'},
#ifdef RADIO_ZONE_MH1   
    {"MH1","RMH11",'1','1'},   
    {"MH1","RMH12",'1','2'},
    {"MH1","RMH13",'1','3'},
    {"MH1","RKOK1",'1','4'},
    {"MH1","RKOK2",'2','1'},
    {"MH1","RKOK3",'2','2'},
    {"MH1","RKOK4",'2','3'},
    {"MH1","RKOK5",'2','4'},
    {"MH1","*.OFF",'*','0'},
#endif   
#ifdef RADIO_ZONE_MH2   
    {"MH2","RMH21",'1','1'},   
    {"MH2","RMH22",'1','2'},
    {"MH2","R____",'1','3'},
    {"MH2","RET_1",'1','4'},
    {"MH2","RWC_2",'2','1'},
    {"MH2","R____",'2','2'},
    {"MH2","RPSH1",'2','3'},
    {"MH2","RKHH2",'2','4'},
    {"MH2","*.OFF",'*','0'},
#endif   
#ifdef RADIO_ZONE_TK1   
    {"TK1","RTK_1",'1','1'},   
    {"TK1","RWC_1",'2','3'},
    {"TK1","RTUP1",'1','3'},
    {"TK1","RTUP2",'1','4'},
    {"TK1","RPARV",'1','2'},
    {"TK1","RKHH1",'1','5'},
    {"TK1","RSAUN",'1','5'},
    {"TK1","RPIHA",'1','7'},
    {"TK1","RTERA",'1','8'},
    {"TK1","RPOLK",'2','5'},
    {"TK1","*.OFF",'*','0'},
#endif            
    {"NOP","_END_",'0','0'}        
};


void relay_dict_debug(void){
    Serial.println(sizeof(relay_dict));
    Serial.println(sizeof(relay_dict[0]));
    Serial.println(int(sizeof(relay_dict)/sizeof(relay_dict[0])));
}

uint8_t find_zone_name(const char *zone, const char *name){
  // returns zero if no relay was found
  int dict_len = int(sizeof(relay_dict)/sizeof(relay_dict[0]));
  int i = 0;
  boolean relay_found = false;
  while (!relay_found && i < dict_len){
     //Serial.print(relay_dict[i].zone);Serial.print("-");Serial.println(relay_dict[i].name);
     if ((strcmp(zone,relay_dict[i].zone) == 0) && (strcmp(name,relay_dict[i].name) == 0)) {
         //Serial.print("Found: "); Serial.print(zone); Serial.print("-");Serial.println(name);
         relay_found = true;
     }
     else i++;
  }
  if(!relay_found) i = 0;  
  return i;
}

char get_relay_unit(uint8_t indx){
   return(relay_dict[indx].relay_unit);
}
char get_relay_indx(uint8_t indx){
   return(relay_dict[indx].relay_indx);
}
