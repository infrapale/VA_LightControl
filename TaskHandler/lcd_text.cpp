/*********************************************************************
lcd_text class
**********************************************************************
Hardware:  PCD8544  alias Nokia 5110 display
Interface:
        LCD_Text();
        void init(void);
        void clear(void);
        void show(void);
        void write(byte rowNbr, char *txt);
        void write(byte rowNbr, String *s);

*********************************************************************/
#include "Arduino.h"
#include "lcd_text.h"


Adafruit_PCD8544 display = Adafruit_PCD8544(PCD_DC, PCD_CS, PCD_RST);

LCD_Text::LCD_Text( int8_t pcd_dc, int8_t pcd_cs, int8_t pcd_rst, int8_t pcd_bl){    //:Adafruit_PCD8544( pcd_dc, pcd_cs, pcd_rst){
     this->pcd_bl = pcd_bl;
     //display.re_init(pcd_dc, pcd_cs, pcd_rst);
     init();
}
void LCD_Text::init(void){
    display.begin();
    display.setContrast(50);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    pinMode(pcd_bl,OUTPUT);
    digitalWrite(pcd_bl,HIGH);
    clear(); 

}

void LCD_Text::clear(void){
    for(int i = 0; i < PCD_ROWS; i++) {
       pcd_buff[i][0] = 0;
    }
    display.clearDisplay();
}
void LCD_Text::show(void){
    display.clearDisplay();
    for(int i = 0; i < PCD_ROWS; i++) {
        display.setCursor(0,row_pos[i]);
        display.println(pcd_buff[i]);
    }
    display.display();
}

void LCD_Text::_write(byte rowNbr, char *txt){
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
void LCD_Text::write(byte rowNbr, char *txt){
    _write(rowNbr, txt);
}
void LCD_Text::write(byte rowNbr, String *s){
    char b[PCD_ROW_LEN];
    s->toCharArray(b,PCD_ROW_LEN);  
    _write(rowNbr, b); 
}
