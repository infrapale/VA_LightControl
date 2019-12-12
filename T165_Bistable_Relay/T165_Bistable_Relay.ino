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
//#include <Arduino.h>
//#include <Astrid.h>
//#include <VillaAstrid.h>
#include <TaHa.h> 
// #include <SmartLoop.h>
#include <UnitAddr.h>

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
#define MH1
#undef  MH2
#define NBR_RELAYS  4 // 1,2,...n

TaHa task_10ms_handle;

byte relay_off_on[4][2]={
    {RELAY_1A,RELAY_1B},
    {RELAY_2A,RELAY_2B},
    {RELAY_3A,RELAY_3B},
    {RELAY_4A,RELAY_4B}
      
};
UnitAddr unit(ADDR_PIN);
char uAddr;
boolean msgReady;
float cal = 1.0;
char rxBuff[MAX_TX_LEN];
uint8_t cindx=0;

void setup() {
   byte i;
   for (i=0;i<4;i++){
      pinMode( relay_off_on[i][0], OUTPUT); 
      pinMode( relay_off_on[i][1], OUTPUT); 
      digitalWrite(relay_off_on[i][0],LOW);
      digitalWrite(relay_off_on[i][1],LOW);
   }
   delay(2000);
   Serial.begin(9600); 
   
   uAddr = '0';
   //Serial.println(analogRead(LM336_PIN)); Serial.println(Temp_LM336_C());
   Serial.print("Unit Address = "); Serial.print(unit.get_analog_value()); Serial.print("  "); Serial.println(uAddr);
   task_10ms_handle.set_interval(10,RUN_RECURRING, run_10ms);


}

void loop() {
    boolean do_continue = true;
    uint8_t cindx;
    char unit_indx = '-';
    char relay_indx = '-';
    char relay_function = '-';
    while (do_continue) {
        task_10ms_handle.run();
        if (Serial.available() >0) {
            char c = Serial.read();  
            switch(cindx){
                case 0: if (c!='<') do_continue = false; break;
                case 1: if (c!='#') do_continue = false; break;
                case 2: if (c!='R') do_continue = false; break;
                case 3: unit_indx = c-'0'; break;
                case 4: relay_indx = c-'0'; break;
                case 5: if (c!='=') do_continue = false; break;
                case 6: relay_function = c; break;
                case 7: if (c!='\r') do_continue = false; break;
                case 8: if (c=='\n') 
                    if ((unit_indx==uAddr) &&
                       ((relay_indx >=0) && (relay_indx < NBR_RELAYS))) {
                        switch(relay_function){
                            case '0': turn_off(relay_indx); break;
                            case '1': turn_off(relay_indx); break;
                            case 'T': toggle(relay_indx);   break;
                       } 
                       do_continue = false; 
                    }
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
