#define SERIAL_BAUD   9600   //115200
#include <Arduino.h>
#include <TaHa.h>
#include "lcd_text.h"
#include "AnalogKeys.h"
#include "radio.h"
#include "json.h"
#include "relay_table.h"

#define ANALOG_KBD_COL_1_2 1
#define ANALOG_KBD_COL_3_4 2
#define RADIO_MSG_LEN   60

// Adafruit M0+RFM69 Feather:
// #8 - used as the radio CS (chip select) pin
// #3 - used as the radio GPIO0 / IRQ (interrupt request) pin.
// #4 - used as the radio Reset pin
#define RFM69_CS        8
#define RFM69_IRQN      3  
#define RFM69_RST       4
#define RFM69_FREQ      434.0   
#define RFM69_TX_IVAL_100ms  20

// LCD Display Adafruit_PCD8544 "Nokia 5110"
#define PCD_DC    5
#define PCD_CS    6
#define PCD_RST   9
#define PCD_BL    10


TaHa task[4];
TaHa scan_akbd_handle;
TaHa rd_akbd_handle;
//Adafruit_PCD8544 display = Adafruit_PCD8544(PCD_DC, PCD_CS, PCD_RST);
//LCD_Text lcd( PCD_DC, PCD_CS, PCD_RST, PCD_BL);
AnalogKeys keys_1(ANALOG_KBD_COL_1_2,8,12);
AnalogKeys keys_2(ANALOG_KBD_COL_3_4,8,12);

void setup() {
  keys_1.begin();
  unsigned int  analog_values[] = { 43, 115, 190, 428, 465, 744, 836, 960}; 
  char key_values_1[] = {'0','*','8','7','5','4','2','1'};
  char key_values_2[] = {'D','#','C','9','B','6','A','3'};
  Serial.print("number of values = ");
  Serial.println(sizeof(analog_values));
  keys_1.set_analog_values(analog_values);
  keys_1.set_key_values(key_values_1);
  keys_2.set_analog_values(analog_values);
  keys_2.set_key_values(key_values_2);
 
  while (!Serial); // wait until serial console is open, remove if not tethered to computer
  Serial.begin(SERIAL_BAUD);
  Serial.println("Taks Handler Test");
  lcd_text_init(PCD_DC, PCD_CS, PCD_RST, PCD_BL); 
  lcd_text_clear();

  // Radio rf69( RFM69_CS,RFM69_IRQN,RFM69_RST, RFM69_FREQ);      434.0   //915.0

  radio_init(RFM69_CS,RFM69_IRQN,RFM69_RST, RFM69_FREQ);
  
  task[0].set_interval(500,RUN_RECURRING, task1);
  task[1].set_interval(1000,RUN_RECURRING, task2);
  task[2].set_interval(10000,RUN_ONCE, task3);
  task[3].set_interval(11000,RUN_ONCE, task4);
  scan_akbd_handle.set_interval(10,RUN_RECURRING, scan_kbd);  
  rd_akbd_handle.set_interval(500,RUN_RECURRING, rd_kbd);  

}

void loop() {
    // put your main code here, to run repeatedly:
    for (int i=0;i<4;i++) task[i].run();
    scan_akbd_handle.run();
    rd_akbd_handle.run();
}

char tx_buff[RADIO_MSG_LEN];

void send_radio(char category, char *zone,char sub_index,char command){
    make_json_relay_array(tx_buff, category, zone, sub_index,command);
    radio_send_msg(tx_buff);
    void RadiateMsg(char *rf69_msg );
    Serial.println(tx_buff);
}

void scan_kbd(void){
    keys_1.scan();
    keys_2.scan();
}

void rd_kbd(void){
    char key; 
    key = keys_1.read();
    if (!key) key = keys_2.read();
    if (key){
        String str = "Key pressed ";
        str += key;       
        lcd_text_write(0,&str);
        lcd_text_show();
        Serial.println(str);
 
        int8_t i = 0;
        boolean found = false;
        while(!found && i < NUMBER_RELAY_ENTRIES ){
            if(key == key_function_table[i].key) {
                found = true;               
            }
            else {
                i++;
            }
        }
        if (found){
            send_radio( key_function_table[i].type,
                        key_function_table[i].zone,
                        key_function_table[i].sub_index,
                        key_function_table[i].operation);

        }
        
        /*
        switch(key) {
            case '0': send_radio('R',"ALL",'G','0'); break;
            case '1': send_radio('R',"MH1",'1','T'); break;
            case '2': send_radio('R',"MH2",'1','T'); break;
            case '3': send_radio('R',"ET_",'1','T'); break;
            case '4': send_radio('R',"MH1",'2','T'); break;
            case '5': send_radio('R',"MH2",'2','T'); break;
            case '6': send_radio('R',"K__",'1','T'); break;
            case '7': send_radio('R',"K__",'2','T'); break;
            case '8': send_radio('R',"TUP",'1','T'); break;
            case '9': send_radio('R',"TUP",'2','T'); break;
            case '*': send_radio('R',"ALL",'1','1'); break;
            case '#': send_radio('R',"PAR",'1','T'); break;
            case 'A': send_radio('R',"KHH",'1','T'); break;
            case 'B': send_radio('R',"PSH",'1','T'); break;
            case 'C': send_radio('R',"SAU",'1','T'); break;
            case 'D': send_radio('R',"ULK",'1','T'); break;
        }
        */
    } 
}
void task1(void){

}
void task2(void){
    char b[20];
    String str;
    unsigned int a;
    a = keys_1.rd_analog();
    //Serial.println(a);
    str = String(a);
    lcd_text_write(2,&str);
    lcd_text_show();
}
void task3(void){                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
    Serial.println("task_3");
    task[1].delay_task(5000);
    lcd_text_write(1,"Task3");
    lcd_text_show();
}
void task4(void){
    Serial.println("task_4");
    lcd_text_write(1,"Task4");
    lcd_text_show();
}
