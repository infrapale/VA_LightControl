//----------------------------------------------------------
// Input event: <#Iun=e> cr/lf, u=unit, n= io nbr, e= H|L
//----------------------------------------------------------
// A3R10  Piha
// A3R11  Etuovi  OK
// A3R12  Terassi OK
// A3R13  Piha
// A3R14
//----------------------------------------------------------
#include <Arduino.h>
#include <TaHa.h>
//#include <Pin_Button.h>
#include "digi_input.h"
#include <avr/wdt.h>   /* Header for watchdog timers in AVR */
#define UNIT_ADDR '1'
#define MAX_INP 4
#define SERIAL_BAUD   9600   //115200
#define SW_ACTIVE A2
#define BOARD_LED_PIN   13   // REMOVED
#define RED_LED_PIN 8
#define BEEP_PIN    9
DigiInp inp[MAX_INP];
TaHa scan_btn_handle;
TaHa one_sec_handle;

boolean alarm_on;
uint8_t alarm_cntr;

void setup() {
    wdt_disable();  /* Disable the watchdog and wait for more than 2 seconds */
    delay(2000);
    while (!Serial); // wait until serial console is open, remove if not tethered to computer
    Serial.begin(SERIAL_BAUD);
    wdt_enable(WDTO_2S);
    Serial.print("TK_8o4i, unit address: ");
    Serial.println(UNIT_ADDR);   
    pinMode(SW_ACTIVE,INPUT); 
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BEEP_PIN, OUTPUT);
    digitalWrite(RED_LED_PIN,LOW);

    digitalWrite(BEEP_PIN,LOW);
    inp[0].Init(10,'3');
    inp[1].Init(11,'2');
    inp[2].Init(12,'1');
    inp[3].Init(13,'4');  //!!! LED need to be removed

    scan_btn_handle.set_interval(10,RUN_RECURRING, scan_btn);
    one_sec_handle.set_interval(1000,RUN_RECURRING, one_sec);
    alarm_on = false;
    alarm_cntr =0;
}

void loop() {
    
    scan_btn_handle.run();
    one_sec_handle.run();
    for(int i= 0; i < MAX_INP; i++){
        char input = inp[i].Read();  //   
        if(input) {
            send_io_event(inp[i].GetId(),input);
            if (alarm_on){
                if(input=='L') {
                    alarm_cntr = 5;
                                     
                }
            }
        }
    }
    int sw_active = digitalRead(SW_ACTIVE);
    if(sw_active){
        //digitalWrite(RED_LED_PIN,SW_ACTIVE);  
        alarm_on = true; 
    } else {
        alarm_on = false;
    }
   
    //Serial.println(digitalRead(SW_ACTIVE));
    wdt_reset();
}

void send_io_event(char id, char ev){
    digitalWrite(RED_LED_PIN,HIGH);
    Serial.print("<#I");
    Serial.print(UNIT_ADDR);
    Serial.print(id);
    Serial.print('=');
    Serial.print(ev);
    Serial.println('>');
    
}
void scan_btn(void){
   for(int i= 0;i<MAX_INP;i++){
       inp[i].Scan();
   }    
}

unsigned int blink_cntr;

void one_sec(void){
    if (++blink_cntr> 3){
        digitalWrite(RED_LED_PIN,HIGH);
        blink_cntr = 0;
    } else {
         digitalWrite(RED_LED_PIN,LOW); 
    }
    if(alarm_cntr > 0){
        digitalWrite(BEEP_PIN,HIGH); 
        alarm_cntr--;
    } else {    
        digitalWrite(BEEP_PIN,LOW);
    }
}
