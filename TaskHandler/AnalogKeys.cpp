#include "AnalogKeys.h"


AnalogKeys::AnalogKeys(byte analog_pin, byte nbr_keys, byte analog_resolution)
{
  a_pin= analog_pin;
  number_of_keys = nbr_keys;
  pinMode(a_pin, INPUT);
  for(byte i =0; i < MAX_NBR_VALUES; i++){
      analog_values[i] = 0;
      key_values[i] = 0;  
  }
  int step = 2^analog_resolution /(number_of_keys + 1); 
  for(byte i =0; i < MAX_NBR_VALUES; i++){
      analog_values[i] = (i+1)*step;
      key_values[i] = i+'0';  
  }
  //analog_values = { 43, 115, 190, 428, 465, 744, 836, 960}; 
  //key_values = {'0','*','8','7','5','4','2','1'};
  begin();
}

void AnalogKeys::begin(void)
{
    state=0;
    cntr = 0;
    key_buff_wr_ptr = 0;
    key_buff_rd_ptr = 0;
    for (int i=0;i< BUFF_LEN;i++) key_buff[i] = 0;
}

void AnalogKeys::set_analog_values(unsigned int *a_values){
    byte i = 0;
    while( a_values[i] && i < number_of_keys){
        analog_values[i] = a_values[i];
        i++;
    }
}    
void AnalogKeys::set_key_values(char *k_values){
    byte i = 0;
    while( k_values[i] && i < number_of_keys){
        key_values[i] = k_values[i];
        i++;
    }
} 
int AnalogKeys::rd_analog(void){
  return( analogRead(a_pin));
}

int AnalogKeys::get_key_index(){
    int a;
    int index = -1;
    a = analogRead(a_pin);
    if (a > ANALOG_TOLERANCE){
        int i = 0;
        while(i< number_of_keys && index == -1){
           int a_delta = abs(a-analog_values[i]);
           if (a_delta < ANALOG_TOLERANCE){
               index = i;
           }
           i++;
        }
    } 
    return index;
}

void AnalogKeys::scan(void) {

  // run every 10ms
  // Serial.println(state);
  switch(state){
      case 0:   // idle state
          active_key_index = get_key_index();
          if (active_key_index != NOT_PRESSED_INDEX ) {
              cntr = BTN_DEB_CNT;
              state = 1;
          }
          break;
      case 1:   // pressed, debounch         
          if (get_key_index() == active_key_index ) {
              if (cntr) cntr--;
              else state = 2;
          }
          else {
             state = 0;
          }
          break;
      case 2:   // pressed, OK
          if (get_key_index() == NOT_PRESSED_INDEX) {
              state = 3;
              cntr = BTN_DEB_CNT;
          }
          break;
     case 3:   // released, debounch
          if (get_key_index() == NOT_PRESSED_INDEX ) {
              if (cntr)  cntr--;
              else state = 4;
          }
          else {
             state = 0;
          }
          break;
     case 4:   // released, Store in buffer
          if((active_key_index != NOT_PRESSED_INDEX) && (active_key_index < number_of_keys)){
              key_buff[key_buff_wr_ptr]=key_values[active_key_index];
              // Serial.println(key_buff[key_buff_wr_ptr]);
              key_buff_wr_ptr = ++key_buff_wr_ptr & BUFF_MASK;
          }
          state = 0;
          break;
     
   }
}


char AnalogKeys::read(void){
   char btn_pressed=0;
   // Serial.print(key_buff_wr_ptr);Serial.print(">");Serial.println(key_buff_rd_ptr);
   if (key_buff[key_buff_rd_ptr]){
       btn_pressed = key_buff[key_buff_rd_ptr];
       key_buff[key_buff_rd_ptr] = 0;
       key_buff_rd_ptr = ++key_buff_rd_ptr & BUFF_MASK;  // ring buffer
       //Serial.println(btn_pressed);
   }
   return(btn_pressed);
}
