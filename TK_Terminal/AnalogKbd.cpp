#include "AnalogKbd.h"


akbd::akbd(byte col1,byte col2, byte kbd_mode)
{
  _pin_nbr[0] = col1;
  _pin_nbr[1] = col2;
  pinMode(_pin_nbr[0], INPUT);
  pinMode(_pin_nbr[1], INPUT);  
  switch ( kbd_mode){
  
    case KBD_4x4:
       nbr_values = 8;
       key_val[0] = 43;     
       key_val[1] = 115; 
       key_val[2] = 190; 
       key_val[3] = 428; 
       key_val[4] = 465; 
       key_val[5] = 746; 
       key_val[6] = 836; 
       key_val[7] = 960; 
       

       key_char[0][0] ='0'; key_char[0][1] ='D';
       key_char[1][0] ='*'; key_char[1][1] ='#';
       key_char[2][0] ='8'; key_char[2][1] ='C';
       key_char[3][0] ='7'; key_char[3][1] ='9';
       key_char[4][0] ='5'; key_char[4][1] ='B';
       key_char[5][0] ='4'; key_char[5][1] ='6';
       key_char[6][0] ='2'; key_char[6][1] ='A';
       key_char[7][0] ='1'; key_char[7][1] ='3';

       break;
    case KBD_4x3:
       nbr_values = 6;
       key_val[0] = 739;  //1 7 
       key_val[1] = 458;  //4 *
       key_val[2] = 421;  //2 8
       key_val[3] = 183;  //5 0  
       key_val[4] = 108;  //3 9
       key_val[5] = 35;   //6 #

       key_char[0][0] ='1'; key_char[0][1] ='7';
       key_char[1][0] ='4'; key_char[1][1] ='*';
       key_char[2][0] ='2'; key_char[2][1] ='8';
       key_char[3][0] ='5'; key_char[3][1] ='0';
       key_char[4][0] ='3'; key_char[4][1] ='9';
       key_char[5][0] ='6'; key_char[5][1] ='#';

       break;
  }

}

void akbd::begin(void)
{
  kbdState=0;
}





/* Keyboard layout 
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
*/
void akbd::set_aval(byte key_indx, unsigned int key_value){
    key_val[key_indx] = key_value;
}    

//int kbdPin = 15; 
int akbd::rd_analog(byte cIndx){
  return( analogRead(_pin_nbr[cIndx]));
}

void akbd::scan(void) {
  int kbdVal = 0; 
  int d;
  int row=0;
  char k=0;
  byte col=0;
  boolean done=false;
  
  while (! done){
    kbdVal = analogRead(_pin_nbr[col]);
    if (kbdVal > KEY_TOL ) done = true;
    else col++;
    if (col>= NBR_COL) done = true;
  }
  if (kbdVal > KEY_TOL ){
    done = false;
    while (! done ){
      d = kbdVal-key_val[row];
      d = abs(d);
      if (d <= KEY_TOL) {
        k = key_char[row][col];
        done = true;
      }
      else row++;
      if (row>=nbr_values) done = true;
    }
       
    switch( kbdState ){
      case 0:  // no key pressed
        if (k) {
           lastKey = k;
           kbdDebCntr=KBD_DEB;
           kbdState= 1;
        }
        break;
      case 1:  // key pressed but debounce still active
         if (k == lastKey) {
           if (kbdDebCntr) kbdDebCntr--;
           else kbdState= 2;
         } else kbdState = 0;
         break;
      case 2:  // key pressed and accepted, not read
          if (k != lastKey) kbdState = 0;
          break;
      case 3:  // key read
           if (k != lastKey) kbdState = 0;
           break;
      }          
      if (kbdState == 0){
         lastKey = k;
         kbdDebCntr=KBD_DEB;      
      }
   } else kbdState = 0;   // no key pressed
}




char akbd::read(void){
  if (kbdState == 2){ 
     kbdState = 3;
     return(lastKey);
  } else return(0);
}
