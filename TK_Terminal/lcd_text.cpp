#include "Arduino.h"
#include "lcd_text.h"

int row_pos[PCD_ROWS] = {0,12, 24,36};
char pcd_buff[PCD_ROWS][PCD_ROW_LEN];
Adafruit_PCD8544 display = Adafruit_PCD8544(PCD_DC, PCD_CS, PCD_RST);

void lcd_text_init(void){
    display.begin();
    display.setContrast(60);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    pinMode(PCD_BL,OUTPUT);
    digitalWrite(PCD_BL,HIGH);
    lcd_text_clear(); 
}

void lcd_text_clear(void){
    for(int i = 0; i < PCD_ROWS; i++) {
       pcd_buff[i][0] = 0;
    }
    display.clearDisplay();
}
void lcd_text_show(void){
    display.clearDisplay();
    for(int i = 0; i < PCD_ROWS; i++) {
       display.setCursor(0,row_pos[i]);
       display.println(pcd_buff[i]);
    }
    display.display();
}

void lcd_text_write(byte rowNbr, char *txt){
    display.setCursor(0,row_pos[rowNbr]);
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
