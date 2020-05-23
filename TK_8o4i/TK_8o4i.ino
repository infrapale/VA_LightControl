//----------------------------------------------------------
// TK_8o4i
// Input event: <#Iun=e> cr/lf, u=unit, n= io nbr, e= H|L
//----------------------------------------------------------
#include <Arduino.h>
#include <TaHa.h>
//#include <Pin_Button.h>
#include "digi_input.h"
#include <avr/wdt.h>   /* Header for watchdog timers in AVR */
#define UNIT_ADDR '1'
#define MAX_INP 4
#define SERIAL_BAUD   9600   //115200

DigiInp inp[MAX_INP];
TaHa scan_btn_handle;

void setup() {
    wdt_disable();  /* Disable the watchdog and wait for more than 2 seconds */
    delay(2000);
    while (!Serial); // wait until serial console is open, remove if not tethered to computer
    Serial.begin(SERIAL_BAUD);
    wdt_enable(WDTO_2S);
    Serial.print("TK_8o4i, unit address: ");
    Serial.println(UNIT_ADDR);    
    inp[0].Init(10,'3');
    inp[1].Init(11,'2');
    inp[2].Init(12,'1');
    inp[3].Init(13,'4');  //!!! LED need to be removed

    scan_btn_handle.set_interval(10,RUN_RECURRING, scan_btn);

}

void loop() {
 
    scan_btn_handle.run();
    for(int i= 0; i < MAX_INP; i++){
        char input = inp[i].Read();  //   
        if(input) {
            send_io_event(inp[i].GetId(),input);
        }
    }
    wdt_reset();
}

void send_io_event(char id, char ev){
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
