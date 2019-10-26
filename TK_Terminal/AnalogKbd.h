#ifndef __akbd_h__
#define __akbd_h__
#include <Arduino.h>

#define KBD_4x4 1
#define KBD_4x3 2
#define MAX_NBR_VALUES 8
#define NBR_COL 2
#define KBD_PIN 15
#define KEY_TOL 16
#define KBD_DEB 4



class akbd
{
  public:
    akbd(byte col1,byte col2,byte kbd_mode);
    void begin(void);
    void set_aval(byte key_indx, unsigned int key_value);
    void scan(void);
    char read(void);
    int rd_analog(byte cIndx);
  private:
    byte _pin_nbr[NBR_COL];
    volatile char lastKey;
    volatile int  kbdDebCntr;
    volatile int  kbdState;
    volatile int  nbr_values;
    unsigned int  key_val[MAX_NBR_VALUES];
    char key_char[MAX_NBR_VALUES][NBR_COL];




};

#endif
