


struct relay_dict_struct {
    char zone[4];
    char name[6];
    char relay_unit;
    char relay_indx;   
};

relay_dict_struct relay_dict[]={
    {"MH1","RMH11",'1','1'},   
    {"MH1","RMH12",'1','2'},
    {"MH1","RK__1",'1','3'},
    {"MH1","RK__2",'1','4'},
    {"MH1","RK__3",'2','1'},
    {"MH1","RPOY1",'2','2'},
    {"MH1","RPOY2",'2','3'},
    {"MH1","RKAAP",'3','4'}        
};

void relay_dict_debug(void){
    Serial.println(sizeof(relay_dict));
    Serial.println(sizeof(relay_dict[0]));
    
}
uint8_t find_zone_name(char *zone, char *name){
  uint8_t dict_len = int(sizeof(relay_dict)/sizeof(relay_dict[0])
  uint8_t i = 0;
  zone
  boolean zone_found = false;
  while (!zone_found){
     if (strcmp(zone,relay_dict[i].zone) == 0){
      
         found = true;
     }
  }

  
  
}
