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
#include <TaHa.h> 
#include <SoftwareSerial.h>
#include <avr/wdt.h>   /* Header for watchdog timers in AVR */

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
#define LM336_PIN A2

#define MAX_TX_LEN  80
#define RELAY_SW_ms 50
#define NBR_RELAYS  4 // 1,2,...n

#define SOFT_RX_PIN 11
#define SOFT_TX_PIN 10
#define SCOM_BUFF_LEN 40

SoftwareSerial softCom(SOFT_RX_PIN, SOFT_TX_PIN); 

byte softComState;
char softComBuff[SCOM_BUFF_LEN];

TaHa task_10ms_handle;

byte relay_off_on[4][2]={
    {RELAY_1A,RELAY_1B},
    {RELAY_2A,RELAY_2B},
    {RELAY_3A,RELAY_3B},
    {RELAY_4A,RELAY_4B}
      
};
uint8_t uAddr;

void setup() {
    byte i;
    wdt_disable();  /* Disable the watchdog and wait for more than 2 seconds */
    delay(2000);
    wdt_enable(WDTO_2S);  /* Enable the watchdog with a timeout of 2 seconds */
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
    Serial.println("GitHub: infrpale/VA_LightControl/T165_Bistable_Relay 2019");
    Serial.print("Unit addr= "); Serial.println(uAddr);
    uAddr = UNIT_INDX;
    //Serial.println(analogRead(LM336_PIN)); Serial.println(Temp_LM336_C());
    //Serial.print("Unit Address = "); Serial.print(unit.get_analog_value()); Serial.print("  "); Serial.println(uAddr);
    task_10ms_handle.set_interval(10,RUN_RECURRING, run_10ms);
 
    pinMode(SOFT_RX_PIN, INPUT_PULLUP);
    pinMode(SOFT_TX_PIN, OUTPUT);
    softCom.begin(9600);

    
    softCom.listen();
    if(softCom.isListening()){
        Serial.println("softCom is listening"); 
    } else {
        Serial.println("softCom is NOT listening"); 
    }   
}

void loop() {
    Serial.println("loop");
    boolean do_continue = true;
    uint8_t cindx;
    uint8_t unit_indx = 0;
    char relay_indx = '-';
    char relay_function = '-';
    cindx =0;
    while (do_continue) {
        Serial.println("loop in while");
        //Serial.print(cindx);
        wdt_reset();
        task_10ms_handle.run();
        if (softCom.available() >0) {
            Serial.println("loop in if");
            char c = softCom.read();  
            Serial.print(c);
            //Serial.println(cindx);
            switch(cindx){
                case 0: if (c!='<') do_continue = false; break;
                case 1: if (c!='#') do_continue = false; break;
                case 2: if (c!='R') do_continue = false; break;
                case 3: unit_indx = c; break;
                case 4: relay_indx = c; break;
                case 5: if (c!='=') do_continue = false; break;
                case 6: relay_function = c; break;
                case 7: if (c!='>') do_continue = false; break;
                case 8: if (c!='\r') do_continue = false; break;
                case 9: if (c=='\n') 
                    Serial.print("All received");Serial.print(unit_indx);Serial.println(uAddr);
                    if ((unit_indx==uAddr+'0') || unit_indx == '*') {
                        if((relay_indx >=0+'0') && (relay_indx <= NBR_RELAYS+'0')) {
                            switch(relay_function){
                                case '0': turn_off(relay_indx-'0'); break;
                                case '1': turn_off(relay_indx-'0'); break;
                                case 'T': toggle(relay_indx-'0');   break;
                            }
                            
                         }                       
                    }
                    if ( unit_indx == '*') {
                        if (relay_indx == '0'){
                            turn_all_off();
                        }
                    }
                    do_continue = false;
                    break;                
            }
            cindx++;
            if (cindx > MAX_TX_LEN -2) do_continue = false;              
        }
    }
}

void run_10ms(void){
    do_every_10ms();
}


void release_relays(void){
   digitalWrite(RELAY_1A,LOW);
   digitalWrite(RELAY_1B,LOW);
}


float Temp_LM336_C(void)
{
   int reading = analogRead(LM336_PIN);  
 
   // converting that reading to voltage, for 3.3v arduino use 3.3
   float voltage = reading * 5.0;
   voltage /= 1024.0; 
   float temperatureC = (voltage - 0.5) * 100 ;
   return (temperatureC);
}
