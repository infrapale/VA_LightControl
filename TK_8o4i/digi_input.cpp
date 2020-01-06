#include "digi_input.h"

DigiInp::DigiInp(){
   pin = 0;
   inp_id ='x';
   state = 0;
   cntr = 0;
   inp_buff_wr_ptr = 0;
   inp_buff_rd_ptr = 0;
}

void DigiInp::Init(byte pin_nbr,char id){
    pin = pin_nbr;
    inp_id = id;
    state = 0;
    cntr = 0;
    inp_buff_wr_ptr = 0;
    inp_buff_rd_ptr = 0;
    buff_mask = Buff_mask(INP_BUFF_LEN);
    pinMode(pin, INPUT_PULLUP);
};

byte DigiInp::Buff_mask(byte buff_len){
    byte mask = 0;
    byte i=0;
    switch(buff_len){
        case 1:  mask = 0b00000000; break;
        case 2:  mask = 0b00000001; break;
        case 4:  mask = 0b00000011; break;
        case 8:  mask = 0b00000111; break;
        case 16: mask = 0b00001111; break;
        default:
            Serial.println("Incorrect buffer length @ DigiInp");
            mask = 0b00000000; 
            break; 
    }
    return mask;
}

void DigiInp::Scan(void){

  // run every 10ms
  switch(state){
      case 0:   // idle state
          if (digitalRead(pin) == LOW ) {
              cntr = INP_DEB_CNT;
              state = 1;
          }
          break;
      case 1:   // pressed, debounch         
          if (digitalRead(pin) == LOW ) {
              if (cntr) cntr--;
              else {
                  state = 2;
                  inp_buff[inp_buff_wr_ptr]='L';
                  inp_buff_wr_ptr = ++inp_buff_wr_ptr & buff_mask;
              }
          }
          else {
             state = 0;
          }
          break;
      case 2:   // pressed, OK
          if (digitalRead(pin) == HIGH ) {
              state = 3;
              cntr = INP_DEB_CNT;
          }
          break;
     case 3:   // released, debounch
          if (digitalRead(pin) == HIGH ) {
              if (cntr)  cntr--;
              else {
                  state = 0;
                  inp_buff[inp_buff_wr_ptr]='H';
                  inp_buff_wr_ptr = ++inp_buff_wr_ptr & buff_mask;                
              }
          }
          else {
             state = 0;
          }
          break;
    }
     //Serial.print("Key: "); Serial.print(i);Serial.print("> ");
     //Serial.print(btn[i].state);
     //Serial.println();
}
char DigiInp::Read(void){
   char btn_pressed=0;
   // Serial.print(inp_buff_wr_ptr);Serial.print(">");Serial.println(inp_buff_rd_ptr);
   if (inp_buff[inp_buff_rd_ptr]){
       btn_pressed = inp_buff[inp_buff_rd_ptr];
       inp_buff[inp_buff_rd_ptr] = 0;
       inp_buff_rd_ptr = ++inp_buff_rd_ptr & buff_mask;  // ring buffer
       //Serial.println(btn_pressed);
   }
   return(btn_pressed);
}
char DigiInp::GetId(void){
    return(inp_id);
}
