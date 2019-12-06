#ifndef __LCD_TEXT_H__
#define __LCD_TEXT_H__
#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


#define PCD_ROWS    4
#define PCD_ROW_LEN 15 


void lcd_text_init(int8_t pcd_dc, int8_t pcd_cs, int8_t pcd_rst, int8_t pcd_bl);
void lcd_text_clear(void);
void lcd_text_show(void);
void lcd_text_write(byte rowNbr, char *txt);
void lcd_text_write(byte rowNbr, String *s);
void _write(byte rowNbr, char *txt);

#endif
