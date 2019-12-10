


struct relay_dict_struct {
    char zone[4];
    char name[6];
    char relay_unit;
    char relay_indx;   
};

relay_dict_struct relay_dict[]={
#ifdef RADIO_ZONE_MH2   
    {"MH1","RMH14",'1','1'},   
    {"MH1","RMH12",'1','2'},
    {"MH1","RK__1",'1','3'},
    {"MH1","RK__2",'1','4'},
    {"MH1","RET_1",'2','1'},
    {"MH1","RPOY1",'2','2'},
    {"MH1","RPOY2",'2','3'},
    {"MH1","RKAAP",'3','4'},
#endif            
    {"NOP","_END_",'0','0'}        
};

void relay_dict_debug(void){
    // Serial.println(sizeof(relay_dict));
    // Serial.println(sizeof(relay_dict[0]));
    
}
uint8_t find_zone_name(char *zone, char *name){
  int dict_len = int(sizeof(relay_dict)/sizeof(relay_dict[0]));
  int i = 0;
  boolean relay_found = false;
  while (!relay_found && i < dict_len){
     if ((strcmp(zone,relay_dict[i].zone) == 0) && (strcmp(name,relay_dict[i].name) == 0)) {
         // Serial.print("Found: "); Serial.print(zone); Serial.print("-");Serial.println(name);
         relay_found = true;
     }
     else i++;
  }

  
  
}
