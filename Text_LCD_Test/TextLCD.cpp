#include "TextLCD.h"


TextLcd::TextLcd(){
    Serial.println("TextLcd::TextLcd()");
    lcd_display.begin();
    Serial.println("TextLcd::TextLcd()  begin");
    lcd_display.setContrast(60);
    Serial.println("TextLcd::TextLcd()  contrast");
    lcd_display.setTextSize(1);
    Serial.println("TextLcd::TextLcd()  setTextSize");
    lcd_display.setTextColor(BLACK);
    Serial.println("TextLcd::TextLcd()  setTextColor");
    pinMode(PCD_BL,OUTPUT);
    digitalWrite(PCD_BL,HIGH);

    Clear();
 
}

void TextLcd::Clear(void){
    for(int i = 0; i < PCD_ROWS; i++) {
       pcd_buff[i][0] = 0;
    }
    //lcd_display->clearDisplay();
}
void TextLcd::Show(void){
    for(int i = 0; i < PCD_ROWS; i++) {
       lcd_display.setCursor(0,row_pos[i]);
       lcd_display.println("zxzs");  // pcd_buff[i]);
    }
    lcd_display.display();
}

void TextLcd::Write(byte rowNbr, char *txt){
    lcd_display.setCursor(0,row_pos[rowNbr]);
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
