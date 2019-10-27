/*********************************************************************
lcd_text class
**********************************************************************
Hardware:  PCD8544  alias Nokia 5110 display
Interface:
        LCD_Text( int8_t pcd_dc, int8_t pcd_cs, int8_t pcd_rst, int8_t pcd_bl );
        void init(void);
        void clear(void);
        void show(void);
        void write(byte rowNbr, char *txt);
        void write(byte rowNbr, String *s);

*********************************************************************/
#include "Arduino.h"
#include "lcd_text.h"


// Adafruit_PCD8544 display = Adafruit_PCD8544(PCD_DC, PCD_CS, PCD_RST);
static Adafruit_PCD8544 *display;
const int row_pos[PCD_ROWS] = {0,12, 24,36};
char pcd_buff[PCD_ROWS][PCD_ROW_LEN];
int8_t pcd_bl;
 

void lcd_text_init(int8_t pcd_dc, int8_t pcd_cs, int8_t pcd_rst, int8_t _pcd_bl){
    pcd_bl = _pcd_bl;
    display = new Adafruit_PCD8544(pcd_dc, pcd_cs, pcd_rst); 
    display->begin();
    display->setContrast(50);
    display->setTextSize(1);
    display->setTextColor(BLACK);
    pinMode(pcd_bl,OUTPUT);
    digitalWrite(pcd_bl,HIGH);
    lcd_text_clear(); 

}
void lcd_text_clear(void){
    for(int i = 0; i < PCD_ROWS; i++) {
       pcd_buff[i][0] = 0;
    }
    display->clearDisplay();
}
void lcd_text_show(void){
    display->clearDisplay();
    for(int i = 0; i < PCD_ROWS; i++) {
        display->setCursor(0,row_pos[i]);
        display->println(pcd_buff[i]);
    }
    display->display();
}

void _write(byte rowNbr, char *txt){
    //display.setCursor(0,row_pos[rowNbr]);
    int i;
    if (rowNbr < PCD_ROWS) {
        for(i = 0; (i < PCD_ROW_LEN -1) && txt[i]; i++){
            pcd_buff[rowNbr][i] = txt[i];
        } 
        for(; (i < PCD_ROW_LEN -1); i++){
            pcd_buff[rowNbr][i] = ' ';
        }
        pcd_buff[rowNbr][PCD_ROW_LEN-1] = 0;  
    }    
}
void lcd_text_write(byte rowNbr, char *txt){
    _write(rowNbr, txt);
}
void lcd_text_write(byte rowNbr, String *s){
    char b[PCD_ROW_LEN];
    s->toCharArray(b,PCD_ROW_LEN);  
    _write(rowNbr, b); 
}
