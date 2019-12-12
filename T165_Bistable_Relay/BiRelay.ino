
#define SWITCH_TIME_x10ms 5

struct relay_status_struct {
   boolean is_on;
   byte cntr_10ms;
};

relay_status_struct relay_status[NBR_RELAYS]; 

void Init(void){
   byte i;
   for(i=0;i<NBR_RELAYS;i++){
      relay_status[i].is_on = false; 
      relay_status[i].cntr_10ms = 0; 
   }
};

void turn_on(byte relay_indx) {
      relay_status[relay_indx].is_on = true; 
      relay_status[relay_indx].cntr_10ms = SWITCH_TIME_x10ms; 
};
void turn_off(byte relay_indx) {
      relay_status[relay_indx].is_on = false; 
      relay_status[relay_indx].cntr_10ms = SWITCH_TIME_x10ms; 
};

void toggle(byte relay_indx) {
      
      relay_status[relay_indx].is_on = ! relay_status[relay_indx].is_on; 
      relay_status[relay_indx].cntr_10ms = SWITCH_TIME_x10ms; 
};

void do_every_10ms(void){
   byte i;

   for(i=0;i<NBR_RELAYS;i++){
      if (relay_status[i].cntr_10ms > 0){
         relay_status[i].cntr_10ms--;
         if (relay_status[i].cntr_10ms > 0){
            if (relay_status[i].is_on) digitalWrite(relay_off_on[i][1],HIGH);
            else digitalWrite(relay_off_on[i][0],HIGH); 
         } 
         else {
            digitalWrite(relay_off_on[i][0],LOW);
            digitalWrite(relay_off_on[i][1],LOW);
         }
      }
   } 
};
