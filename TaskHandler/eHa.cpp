#include "eHa.h"


EHa::EHa(){
    event_buff_indx = 0;
    event_cntr = 0;
    for(uint8_t i = 0; i < EHA_MAX_EVENTS; i++) ebuff[i][0] = 0;
}

void EHa::add(char *data){
};
char *EHa::get_event(uint8_t last_event){
};
 
    
    private:
        uint8_t event_buff_ptr;
        uint8_t event_cntr;
        char ebuff[EHA_MAX_EVENTS][EHA_EVENT_DATA_LEN]
        
        //eha_callback task;

};
