#include "PinBtn.h"

PinBtn::PinBtn(){
   pin = 0;
   value ='x';
   state = 0;
   cntr = 0;
   key_buff_wr_ptr = 0;
   key_buff_rd_ptr = 0;
}

void PinBtn::Init(byte pin_nbr,char val){
    pin = pin_nbr;
    value = val;
    state = 0;
    cntr = 0;
    key_buff_wr_ptr = 0;
    key_buff_rd_ptr = 0;

    pinMode(pin, INPUT_PULLUP);
};

void PinBtn::Scan(void){

  // run every 10ms
  switch(state){
      case 0:   // idle state
          if (digitalRead(pin) == LOW ) {
              cntr = BTN_DEB_CNT;
              state = 1;
          }
          break;
      case 1:   // pressed, debounch         
          if (digitalRead(pin) == LOW ) {
              if (cntr) cntr--;
              else state = 2;
          }
          else {
             state = 0;
          }
          break;
      case 2:   // pressed, OK
          if (digitalRead(pin) == HIGH ) {
              state = 3;
              cntr = BTN_DEB_CNT;
          }
          break;
     case 3:   // released, debounch
          if (digitalRead(pin) == HIGH ) {
              if (cntr)  cntr--;
              else state = 4;
          }
          else {
             state = 0;
          }
          break;
     case 4:   // released, Store in buffer
          key_buff[key_buff_wr_ptr]=value;
          //Serial.println(key_buff[key_buff_wr_ptr]);
          key_buff_wr_ptr = ++key_buff_wr_ptr & 0x03;
          state = 0;
          break;
   }
     //Serial.print("Key: "); Serial.print(i);Serial.print("> ");
     //Serial.print(btn[i].state);
     //Serial.println();
}
char PinBtn::Read(void){
   char btn_pressed=0;
   // Serial.print(key_buff_wr_ptr);Serial.print(">");Serial.println(key_buff_rd_ptr);
   if (key_buff[key_buff_rd_ptr]){
       btn_pressed = key_buff[key_buff_rd_ptr];
       key_buff[key_buff_rd_ptr] = 0;
       key_buff_rd_ptr = ++key_buff_rd_ptr & 0x03;  // ring buffer
       //Serial.println(btn_pressed);
   }
   return(btn_pressed);
}
