#define SERIAL_BAUD   9600   //115200
#include <Arduino.h>
#include "TaHa.h"
#include "lcd_text.h"
#include "AnalogKeys.h"
#include <RH_RF69.h>
#include "json.h"


#define ANALOG_KBD_COL_1_2 1
#define ANALOG_KBD_COL_3_4 2
#define RADIO_MSG_LEN   60

// Adafruit M0+RFM60 Feather:
// #8 - used as the radio CS (chip select) pin
// #3 - used as the radio GPIO0 / IRQ (interrupt request) pin.
// #4 - used as the radio Reset pin
#define RFM69_CS        8
#define RFM69_IRQN      3  
#define RFM69_RST       4
#define RFM69_FREQ      434.0   
#define RFM69_TX_IVAL_100ms  20


#define PCD_DC    5
#define PCD_CS    6
#define PCD_RST   9
#define PCD_BL    10


TaHa task[4];
TaHa scan_akbd_handle;
//Adafruit_PCD8544 display = Adafruit_PCD8544(PCD_DC, PCD_CS, PCD_RST);
LCD_Text lcd( PCD_DC, PCD_CS, PCD_RST, PCD_BL);
AnalogKeys keys_1(ANALOG_KBD_COL_1_2,8,12);
AnalogKeys keys_2(ANALOG_KBD_COL_3_4,8,12);
RH_RF69 rf69(RFM69_CS, RFM69_IRQN);

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
  lcd.init(); 
  lcd.clear();
  
  task[0].set_interval(500,RUN_RECURRING, task1);
  task[1].set_interval(1000,RUN_RECURRING, task2);
  task[2].set_interval(10000,RUN_ONCE, task3);
  task[3].set_interval(11000,RUN_ONCE, task4);
  scan_akbd_handle.set_interval(10,RUN_RECURRING, scan_kbd);  

}

void loop() {
    // put your main code here, to run repeatedly:
    for (int i=0;i<4;i++) task[i].run();
    scan_akbd_handle.run();
}
char tx_buff[RADIO_MSG_LEN];

void send_radio(char category, char *zone,char sub_index,char command){
    make_json_relay_array(tx_buff, category, zone, sub_index,command);
    // radiate_msg(tx_buff);
    Serial.println(tx_buff);
}

void scan_kbd(void){
    keys_1.scan();
    keys_2.scan();
}

void task1(void){
    char key; 
    key = keys_1.read();
    if (!key) key = keys_2.read();
    if (key){
        String str = "Key pressed ";
        str += key;       
        lcd.write(0,&str);
        lcd.show();
        Serial.println(str);
    }
}
void task2(void){
    char b[20];
    String str;
    unsigned int a;
    a = keys_1.rd_analog();
    //Serial.println(a);
    str = String(a);
    lcd.write(2,&str);
    lcd.show();
}
void task3(void){                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
    Serial.println("task_3");
    task[1].delay_task(5000);
    lcd.write(1,"Task3");
    lcd.show();
}
void task4(void){
    Serial.println("task_4");
    lcd.write(1,"Task4");
    lcd.show();
}
