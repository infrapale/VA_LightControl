#ifndef __AnalogKeys_h__
#define __AnalogKeys_h__
#include <Arduino.h>

#define MAX_NBR_VALUES 8
#define KEY_TOL 16
#define KBD_DEB 4

class AnalogKeys
{
  public:
    AnalogKeys(byte analog_pin,byte nbr_keys);
    void begin(void);
    void set_analog_values(unsigned int *analog_values);
    void set_key_values(char *key_values);
    void scan(void);
    char read(void);
    int rd_analog(byte cIndx);
  private:
    byte number_of_keys;
    unsigned int analog_values[MAX_NBR_VALUES];
    char key_val[MAX_NBR_VALUES];
    byte _pin_nbr
    volatile char lastKey;
    volatile int  kbdDebCntr;
    volatile int  kbdState;
    volatile int  nbr_values;



};

#endif
