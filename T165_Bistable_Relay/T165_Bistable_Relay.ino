///////////////////////////////////////////////////////////////////////////
////   Sketch: T165 8 Bi-Relay
////   
////                  8out4inp.c
///////////////////////////////////////////////////////////////////////////
////   Message syntax: "<#Rnr=x>"
////       u = unit index 1,2,...
////       r = relay index 1,2,3,4
////       x = 0: off
////       x = 1: on
////       x = T: toggle
////
////       Example:  <#R24=1>   relay unit = 2, relay 4, switch on 
///////////////////////////////////////////////////////////////////////////
#include "Arduino.h"
#include "AVR_Watchdog.h"

#include <TaHa.h> 
#include <SoftwareSerial.h>
//#include <avr/wdt.h>   /* Header for watchdog timers in AVR */

#define UNIT_INDX 2

#define RELAY_1A 8
#define RELAY_1B 9
#define RELAY_2A 6
#define RELAY_2B 7
#define RELAY_3A 4
#define RELAY_3B 5
#define RELAY_4A 2
#define RELAY_4B 3

#define ADDR_PIN A3

#define MAX_TX_LEN  80
#define RELAY_SW_ms 50
#define NBR_RELAYS  4 // 1,2,...n

#define SCOM_BUFF_LEN 40

byte softComState;
char softComBuff[SCOM_BUFF_LEN];

TaHa task_10ms_handle;
AVR_Watchdog watchdog(4);

byte relay_off_on[4][2]={
    {RELAY_1A,RELAY_1B},
    {RELAY_2A,RELAY_2B},
    {RELAY_3A,RELAY_3B},
    {RELAY_4A,RELAY_4B}
      
};
uint8_t uAddr;

void setup() {
    byte i;
    delay(2000);
    watchdog.set_timeout(120);
    for (i=0;i<4;i++){
         pinMode( relay_off_on[i][0], OUTPUT); 
         pinMode( relay_off_on[i][1], OUTPUT); 
         digitalWrite(relay_off_on[i][0],LOW);
         digitalWrite(relay_off_on[i][1],LOW);
    }
    InitRelays();
    uAddr = UNIT_INDX;
    Serial.begin(9600); 
    Serial.println();
    Serial.println("GitHub: infrpale/VA_LightControl/T165_Bistable_Relay 2019, 2023");
    Serial.print("Unit addr= "); Serial.println(uAddr);
    uAddr = UNIT_INDX;
    //Serial.println(analogRead(LM336_PIN)); Serial.println(Temp_LM336_C());
    //Serial.print("Unit Address = "); Serial.print(unit.get_analog_value()); Serial.print("  "); Serial.println(uAddr);
    task_10ms_handle.set_interval(10,RUN_RECURRING, run_10ms); 
}


void read_and_parse(char c)
{
    static uint8_t cindx = 0;
    static boolean do_continue = true;
    static uint8_t unit_indx = 0;
    static char relay_indx = '-';
    static char relay_function = '-';

    //Serial.print(cindx); Serial.print(": ");  Serial.println(c); 
    watchdog.clear();
    switch(cindx)
    {
        case 0: if (c!='<') do_continue = false; break;
        case 1: if (c!='#') do_continue = false; break;
        case 2: if (c!='R') do_continue = false; break;
        case 3: unit_indx = c; break;
        case 4: relay_indx = c; break;
        case 5: if (c!='=') do_continue = false; break;
        case 6: relay_function = c; break;
        case 7: if (c!='>') do_continue = false; break;
        case 8: 
            if ((c=='\r') || (c=='\n'))
            {
                //Serial.print("All received");Serial.print(unit_indx);Serial.println(uAddr);
                if ((unit_indx==uAddr+'0') || unit_indx == '*') 
                {
                    if((relay_indx >=0+'0') && (relay_indx <= NBR_RELAYS+'0')) 
                    {
                        switch(relay_function)
                        {
                            case '0': turn_off(relay_indx-'0'); break;
                            case '1': turn_on(relay_indx-'0'); break;
                            case 'T': toggle(relay_indx-'0');   break;
                        }                        
                    }   
                    if ((relay_indx == '*') && (relay_function=='0'))
                    {
                        turn_all_off();
                    }
                }
                do_continue = false;
            }
            break;                
    }

    cindx++;
    if (cindx > MAX_TX_LEN -2) do_continue = false;              

    if (!do_continue)
    {
        cindx = 0;
        do_continue = true;

    }
}

void loop() 
{
    //watchdog.clear();
    task_10ms_handle.run();
    if (Serial.available() >0) 
    {  
        char c = Serial.read();  
        read_and_parse(c);
    }
}



void run_10ms(void){
    do_every_10ms();
}


void release_relays(void){
   digitalWrite(RELAY_1A,LOW);
   digitalWrite(RELAY_1B,LOW);
}


