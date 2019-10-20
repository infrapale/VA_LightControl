#ifndef __AnalogKeys_h__
#define __AnalogKeys_h__
#include <Arduino.h>

#define MAX_NBR_VALUES  16
// BUFF_LEN and BUFF_MASK must match!!
#define BUFF_LEN        4
#define BUFF_MASK       0b00000011
#define KEY_TOL         16
#define BTN_DEB_CNT     6 
#define NOT_PRESSED_INDEX -1
#define ANALOG_TOLERANCE 10

class AnalogKeys
{
  public:
    AnalogKeys(byte analog_pin,byte nbr_keys, byte analog_resolution);
    void begin(void);
    void set_analog_values(unsigned int *analog_values);
    void set_key_values(char *key_values);
    void scan(void);
    char read(void);
    int rd_analog();
  private:
    int           get_key_index();  
    byte          number_of_keys;
    unsigned int  analog_values[MAX_NBR_VALUES];
    char          key_values[MAX_NBR_VALUES];
    char          key_buff[BUFF_LEN];
    byte          a_pin;
    volatile int  active_key_index;
    volatile int  cntr;
    volatile int  state;
    volatile int  nbr_values;
    volatile byte key_buff_wr_ptr;
    volatile byte key_buff_rd_ptr;



};

#endif
