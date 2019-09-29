#ifndef TextLCD
#define TextLCD
#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define PCD_DC 5
#define PCD_CS 6
#define PCD_RST 9
#define PCD_BL 10


#define PCD_ROWS 4
#define PCD_ROW_LEN 15   // 14 characters

class TextLcd{
public:
    TextLcd();
    void Clear(void);
    void Show(void);
    void Write(byte rowNbr, char *txt);
private:
    int row_pos[PCD_ROWS] = {0,12, 24,36};
    int pcd_buff[PCD_ROWS][PCD_ROW_LEN];
    Adafruit_PCD8544 lcd_display = Adafruit_PCD8544(PCD_DC, PCD_CS, PCD_RST);
};
#endif
