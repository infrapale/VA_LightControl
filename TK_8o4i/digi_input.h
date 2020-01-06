#ifndef __DIGI_INP_H__
#define __DIGI_INP_H__
#include "Arduino.h"

#define INP_DEB_CNT   20 
#define INP_BUFF_LEN  4
class DigiInp{
public:
    DigiInp();
    void Init(byte pin_nbr,char id);
    void Scan(void);
    char Read(void);
    char GetId(void);
private:
    char inp_buff[INP_BUFF_LEN];
    int inp_buff_wr_ptr;
    int inp_buff_rd_ptr;
    byte Buff_mask(byte buff_len);
    byte pin;
    char inp_id;
    byte state;
    byte cntr;
    byte buff_mask;
};
#endif
