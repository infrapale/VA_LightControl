#ifndef _LCD_TEXT
#define _LCD_TEXT
#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define PCD_DC 5
#define PCD_CS 6
#define PCD_RST 9
#define PCD_BL 10

#define PCD_ROWS 4
#define PCD_ROW_LEN 15   // 14 characters


void lcd_text_init(void);
void lcd_text_clear(void);
void lcd_text_show(void);
void lcd_text_write(byte rowNbr, char *txt);
#endif
