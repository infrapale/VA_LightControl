#ifndef __LCD_TEXT_H__
#define __LCD_TEXT_H__
#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define PCD_DC 5
#define PCD_CS 6
#define PCD_RST 9
#define PCD_BL 10

#define PCD_ROWS    4
#define PCD_ROW_LEN 15   // 14 characters


class LCD_Text {
    public:
        LCD_Text();
        void init(void);
        void clear(void);
        void show(void);
        void write(byte rowNbr, char *txt);
        void write(byte rowNbr, String *s);
    private:
        void _write(byte rowNbr, char *txt);
        const int row_pos[PCD_ROWS] = {0,12, 24,36};
        char pcd_buff[PCD_ROWS][PCD_ROW_LEN];
};
#endif
