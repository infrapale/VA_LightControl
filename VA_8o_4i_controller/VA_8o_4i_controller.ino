///////////////////////////////////////////////////////////////////////////
////   Sketch: VA 8 Out 4 Inp Controller
////   
////                VA_8o_4i_controller.ino
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
#include <avr/wdt.h>   /* Header for watchdog timers in AVR */

#define UNIT_INDX 2

#define OUT_1_PIN 8
#define OUT_2_PIN 8
#define OUT_3_PIN 8
#define OUT_4_PIN 8
#define OUT_5_PIN 8
#define OUT_6_PIN 8
#define OUT_7_PIN 8
#define OUT_8_PIN 8

#define INP_1_PIN 8
#define INP_2_PIN 8
#define INP_3_PIN 8
#define INP_4_PIN 8

#define ADDR_PIN A3
#define LM336_PIN A2

#define MAX_TX_LEN    80
#define NBR_OUT_PINS  8
#define NBR_INP_PINS  4

#define SCOM_BUFF_LEN 40

typedef enum 
{
    STATE_NOT_DEFINED = 0,
    STATE_ON,
    STATE_OFF
} io_state_et;

typedef struct 
{
    io_state_et state;
    bool changed;    
}   io_st;


typedef struct 
{
    io_st out[NBR_OUT_PINS];
    io_st inp[NBR_INP_PINS];    
}   io_data_st;


io_data_st  io;
char rx_buf[SCOM_BUFF_LEN];

TaHa task_10ms_handle;
TaHa task_100ms_handle;

uint8_t out_pin[NBR_OUT_PINS] = {
    OUT_1_PIN, OUT_2_PIN, OUT_3_PIN, OUT_4_PIN, 
    OUT_5_PIN, OUT_6_PIN, OUT_7_PIN, OUT_8_PIN       
};

uint8_t omp_pin[NBR_INP_PINS] = {
    INP_1_PIN, INP_2_PIN, INP_3_PIN, INP_4_PIN
};


uint8_t uAddr = 0x01;

void read_serial_command(void);

void setup() {


    wdt_disable();  /* Disable the watchdog and wait for more than 2 seconds */
    delay(2000);
    wdt_enable(WDTO_2S);  /* Enable the watchdog with a timeout of 2 seconds */
    for (uint8_t i=0; i<NBR_OUT_PINS; i++)
    {
         pinMode( out_pin[i], OUTPUT); 
         digitalWrite(out_pin[i],LOW);
         io.out[i].state = STATE_NOT_DEFINED;
         io.out[i].changed = true;
    }
    for (uint8_t i=0; i < NBR_INP_PINS; i++)
    {
         pinMode( out_pin[i], INPUT_PULLUP); 
         io.inp[i].state = STATE_NOT_DEFINED;
         io.inp[i].changed = true;
    }

    //InitRelays();
    uAddr = UNIT_INDX;
    Serial.begin(9600); 
    Serial.println();
    Serial.println("GitHub: infrpale/VA_8o_4i");
    Serial.print("Unit addr= "); Serial.println(uAddr);
    //Serial.println(analogRead(LM336_PIN)); Serial.println(Temp_LM336_C());
    //Serial.print("Unit Address = "); Serial.print(unit.get_analog_value()); Serial.print("  "); Serial.println(uAddr);
    task_10ms_handle.set_interval(10,RUN_RECURRING, run_10ms);
    task_100ms_handle.set_interval(100,RUN_RECURRING, run_100ms);
 
}



void loop() {
    Serial.println("loop");
    boolean do_continue = true;

    Serial.println("loop in while");
        //Serial.print(cindx);
        //wdt_reset();
    //task_10ms_handle.run();
}

void read_serial_command(void)
{
    static uint8_t   cindx = 0;
    static uint32_t  last_received_millis = millis(); 

    char unit_char;
    char relay_char;
    char relay_function;
    uint8_t relay_indx;
    bool do_continue = true;

    if (Serial.available() > 0) 
    {
        Serial.println("loop in if");
        char c = Serial.read();  
        Serial.print(c);
        last_received_millis = millis();         
        //Serial.println(cindx);
        switch(cindx){
          // <#R10=T>
            case 0: if (c!='<') do_continue = false; break;
            case 1: if (c!='#') do_continue = false; break;
            case 2: if (c!='R') do_continue = false; break;
            case 3: unit_char = c; break;
            case 4: relay_indx = c; break;
            case 5: if (c!='=') do_continue = false; break;
            case 6: relay_function = c; break;
            case 7: if (c!='>') do_continue = false; break;
            case 8: 
                if ((c=='\n') || (c!='\r')) 
                {
                    Serial.print("All received");Serial.print(unit_char);Serial.println(uAddr);
                    if ((unit_char == uAddr+'0') ) 
                    {
                       relay_indx = relay_char -'0';
                        if((relay_indx >=0) && (relay_indx <= NBR_OUT_PINS)) 
                        {
                            switch(relay_function)
                            {
                                case '0': 
                                    io.out[relay_indx].state = STATE_OFF;
                                    io.out[relay_indx].changed = true;
                                    break;
                                case '1': 
                                    io.out[relay_indx].state = STATE_ON;
                                    io.out[relay_indx].changed = true;
                                    break;
                                case 'T': 
                                    if (io.out[relay_indx].state == STATE_ON)
                                        io.out[relay_indx].state = STATE_OFF;
                                    else    
                                        io.out[relay_indx].state = STATE_ON;

                                    io.out[relay_indx].changed = true;
                                    break;
                            }
                            
                          }                       
                    }
                    else if ( relay_char == '*') 
                    {
                        if (relay_function == '0')
                        {
                            for (uint8_t i=0; i<NBR_OUT_PINS; i++)
                            {
                                 io.out[i].state = STATE_OFF;
                                 io.out[i].changed = true;                                
                            }
                        }
                        else if (relay_function == '1')
                        {
                            for (uint8_t i=0; i<NBR_OUT_PINS; i++)
                            {
                                 io.out[i].state = STATE_ON;
                                 io.out[i].changed = true;
                            }
                        }

                    }
                    do_continue = false;

                }
                break;

        }
        cindx++;
        if (cindx > MAX_TX_LEN -2) do_continue = false;              
    }
    else
    {
        if ((millis() - last_received_millis) > 1000)
        {
            do_continue = false;
        }             
    }    
    if (!do_continue)
    {
        cindx = 0;
    }

}



void run_10ms(void){
    //do_every_10ms();
}

void run_100ms(void){
    static uint8_t out_indx = 0;

    if (out_indx >= NBR_OUT_PINS ) out_indx = 0;
    if (io.out[out_indx].changed)
    {
        io.out[out_indx].changed = false;
        if (io.out[out_indx].state == STATE_ON)        
            digitalWrite(out_pin[out_indx],HIGH);
        else
            digitalWrite(out_pin[out_indx],LOW);
        Serial.println("chaning output");    
    }
    out_indx++;
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
