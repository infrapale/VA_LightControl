#ifndef Pin_btn_h
#define Pin_btn_h
#include "Arduino.h"

#define BTN_DEB_CNT   20 
#define KEY_BUFF_LEN  4
class PinBtn{
public:
    PinBtn();
    void Init(byte pin_nbr,char val);
    void Scan(void);
    char Read(void);
private:
    char key_buff[KEY_BUFF_LEN];
    int key_buff_wr_ptr;
    int key_buff_rd_ptr;
    byte pin;
    char value;
    byte state;
    byte cntr;
};
#endif
