/**
 *  RFM69   ABC   PIN
 *  RST     A     27
 *  CS      C     15
 *  INT     D     32
 *  MOSI
 *  MISO
 *  SCK
 */

#define ARDUINO_RUNNING_CORE 1
#define MQTT_UPDATE_INTERVAL_ms   60000   ///< MQTT update interval, one value per time

#define WDT_TIMEOUT               10

#define MH1_BTN
//#define MH2_BTN
//#define K_BTN
//#define TK_RELAY

#define MENU_DATA
#define BROADCAST     255
#define RECEIVER      BROADCAST    // The recipient of packets

//Match frequency to the hardware version of the rf69 on your Feather
#define FREQUENCY     RF69_434MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
//#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD   115200

#define RFM69_CS      15
#define RFM69_INT     32
#define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     27
#define RFM69_FREQ    434.0   //915.0
#define RFM69_TX_IVAL_100ms  20;
#define LED           13  // onboard blinky
#define MAX_BTN       6
#define CODE_LEN      6
#define ZONE_LEN      4
#define FUNC_LEN      4
#define CODE_BUFF_LEN 32
#define CODE_BUFF_LEN_MASK 0b00011111;

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"
#include "secrets.h"
#include <rfm69_support.h>
#include <time.h>
#include <Pin_Button.h>

#ifdef K_BTN
//#include <akbd.h>
//akbd kbd(A0);
//akbd qkbd(A1);
//led_blink leds(6,7,3,9,5,4);
#endif
// Tasks
// TaHa scan_btn_handle;
//TaHa radio_send_handle;
// Code buffer
char code_buff[CODE_BUFF_LEN][CODE_LEN];  // ring buffer
char zone_buff[CODE_BUFF_LEN][ZONE_LEN];  // ring buffer
char func_buff[CODE_BUFF_LEN][FUNC_LEN];
byte code_wr_indx;
byte code_rd_indx;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);

const char TIME_ISO_FEED[] PROGMEM = "time/ISO-8601";
Adafruit_MQTT_Subscribe time_iso_feed = Adafruit_MQTT_Subscribe(&mqtt, TIME_ISO_FEED);

static TaskHandle_t htask;

void TaskSendAio( void *pvParameters );
void TaskReceiveAio( void *pvParameters );


//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************


int16_t packetnum = 0;  // packet counter, we increment per xmission
PinBtn butt[MAX_BTN];
 
void setup() {  
    BaseType_t rc;
    esp_err_t er;

    delay(2000);

    while (!Serial); // wait until serial console is open, remove if not tethered to computer
    Serial.begin(SERIAL_BAUD);

    htask = xTaskGetCurrentTaskHandle();
    
    delay(4000);
    Serial.begin(115200);   

    er = esp_task_wdt_status(htask);
    assert(er == ESP_ERR_NOT_FOUND);
    
    if ( er == ESP_ERR_NOT_FOUND ) {
        er = esp_task_wdt_init(10,true);
        assert(er == ESP_OK);
        er = esp_task_wdt_add(htask);
        assert(er == ESP_OK);
        printf("Task is subscribed to TWDT.\n");
    }

    /*
    butt[0].Init(3,'1');
    butt[1].Init(4,'2');
    butt[2].Init(5,'3');
    butt[3].Init(6,'4');
    butt[4].Init(7,'5');
    butt[5].Init(8,'6');
*/    
    // clear code and zone buffers
    for(uint8_t i=0; i<CODE_BUFF_LEN; i++){
        code_buff[i][0] = 0;
        zone_buff[i][0] = 0;
    }
    printf("step 2\n");

    code_wr_indx = 0;
    code_rd_indx = 0;

    #ifdef K_BTN
    //kbd.begin();
    //qkbd.begin();
    #endif

    pinMode(LED, OUTPUT);     
   
   
   
    Serial.println("Connecting WiFi");       
    WiFi.begin(WIFI_SSID,WIFI_PASS );
    if (WiFi.status() != WL_CONNECTED) 
    {
        Serial.println("*"); 
    }
    Serial.println("WiFi Connected");      


    mqtt.subscribe(&time_iso_feed);
    Serial.print("Connecting to Adafruit IO");

    //StartTasks();


}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Main Loop
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void loop() 
{
esp_err_t er;
    er = esp_task_wdt_add(nullptr);
    er = esp_task_wdt_status(htask);
    assert(er == ESP_OK);
    esp_task_wdt_reset();
    vTaskDelay(1000);

}

void StartTasks(void){
    BaseType_t rc;
    /* 
    xTaskCreatePinnedToCore(
       TaskSendAio
        ,  "TaskSendAio"   // A name just for humans
        ,  32000  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  4  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL 
        ,  ARDUINO_RUNNING_CORE);
    */
    
    xTaskCreatePinnedToCore(
       TaskReceiveAio
        ,  "TaskReceiveAio"   // A name just for humans
        ,  32000  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  8  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL 
        ,  ARDUINO_RUNNING_CORE);


   
    //aio_sema_handle = xSemaphoreCreateBinary();
    //assert(aio_sema_handle);
     
}



void TaskSendAio( void *pvParameters ){
    uint8_t iot_state = 0;
    BaseType_t rc;
    
    for (;;)
    {   
        /*
        //rc = xSemaphoreTake(aio_sema_handle,500); 
        rc = pdPASS;
        if (rc == pdPASS){
            io.run();        
            switch (iot_state) {
                case 0: case 1: case 2: case 3: case 4:
                    if (iot_state < NBR_LDR_RES){
                        ldr_feed[iot_state]->save(ldr_value[iot_state]);
                    }    
                    break;
                case 5:
                    temperature->save(bme.temperature);
                    break;
                case 6:    
                    humidity->save(bme.humidity);
                    break;
            }
            iot_state++;
            if (iot_state > 6 ){
                iot_state = 0;
            } 
            //rc = xSemaphoreGive(aio_sema_handle);
        }    
        */
        vTaskDelay(2000);
       
    }
}

void TaskReceiveAio( void *pvParameters ){
    uint8_t iot_receive_state = 0;
    BaseType_t rc;
    esp_err_t er;

    struct tm mqtt_tm = {0};
    struct tm *local_tm;
    time_t time;
    int8_t ret;

    er = esp_task_wdt_add(nullptr);
    assert(er == ESP_OK);

    Adafruit_MQTT_Subscribe *subscription;
    for (;;)
    {    
        printf("TaskReceiveAio: %d",iot_receive_state);
        switch (iot_receive_state) 
        {
            case 0:
                if (mqtt.connected()) 
                {
                    iot_receive_state = 2;
                    Serial.println("MQTT Connected!");
                } 
                else 
                {
                    iot_receive_state = 1;
                    Serial.println("Trying to Connected MQTT!");
                }  
                esp_task_wdt_reset();
                vTaskDelay(1000);
                break;
            case 1:
                ret = mqtt.connect();
                if (ret != 0)
                {
                    mqtt.disconnect();
                }
                else
                {
                    iot_receive_state = 2; 
                    Serial.println("MQTT Connected!");
                }
                esp_task_wdt_reset();
                vTaskDelay(1000);
                break;
            case 2:  // MQTT Connected
                subscription = mqtt.readSubscription(10);
                //subscription = NULL;
                if (subscription == &time_iso_feed) 
                {
                    printf("Got %s\n",(char *)time_iso_feed.lastread);
                }
                esp_task_wdt_reset();
                vTaskDelay(10000);
                break;
            case 3:
                break;
        }

        if (mqtt.connected()) 
        {
          return;
        }



        //rc = xSemaphoreTake(aio_sema_handle,500); 
        if (rc == pdPASS){
            io.run();
            //led_yellow->onMessage(handleMessage);
            //rc = xSemaphoreGive(aio_sema_handle);
        }
        vTaskDelay(1000);
    }
}







void mini_terminals(void){
    // if button is preseed add a command code to the ring buffer
      
/*    char btn;
    int btns;
    #if defined(MH1_BTN) || defined(MH2_BTN)
    btns = 3;
    #endif
    
    for(int i= 0; i < btns; i++){
        //btn = butt[i].Read();  //   rd_btn();
        //if(btn) break;
    }
    if (btn != 0) {
        //Serial.print("button= ");Serial.println(btn);
        switch(btn){
            #ifdef MH1_BTN
            case '1': add_code("MH1","RMH11","T"); break;
            case '2': add_code("MH1","RMH12","T"); break;
            case '3': add_code("MH1","RMH13","T"); break;
            #endif
            #ifdef MH2_BTN
            case '1': add_code("MH2","RMH21","T"); break;
            case '2': add_code("MH2","RMH22","T"); break;
            case '3': add_code("MH2","RET_1","T"); break;  
            #endif
        }           
    } 
      
*/
}



void add_code(const char *new_zone, const char *new_code, const char *new_func){
    int i;
    for(i = 0; i < CODE_LEN; i++) {
        if (new_code[i] != 0) { 
            code_buff[code_wr_indx][i] = new_code[i];
        } 
        else {
           code_buff[code_wr_indx][i] =0;
        }   
    }
    for(i = 0; i < ZONE_LEN; i++) {
        if (new_code[i] != 0) { 
            zone_buff[code_wr_indx][i] = new_zone[i];
        } 
        else {
           zone_buff[code_wr_indx][i] =0;
        }   
    }
    for(i = 0; i < FUNC_LEN; i++) {
        if (new_func[i] != 0) { 
            func_buff[code_wr_indx][i] = new_func[i];
        } 
        else {
           func_buff[code_wr_indx][i] =0;
        }   
    }
 

    code_wr_indx = ++code_wr_indx & CODE_BUFF_LEN_MASK;   
}

void radiate_msg( const char *zone, const char *relay_addr, char *func ) {
    String relay_json = JsonRelayString(zone, relay_addr, func, "" );
    char rf69_packet[RH_RF69_MAX_MESSAGE_LEN] = "";
    relay_json.toCharArray(rf69_packet, RH_RF69_MAX_MESSAGE_LEN);
    radio_send_msg(rf69_packet);
    Serial.println(rf69_packet);
}

void radio_tx_hanndler(void){
    if (code_buff[code_rd_indx][0] != 0){
        radiate_msg(zone_buff[code_rd_indx],code_buff[code_rd_indx],func_buff[code_rd_indx]);
        Serial.print(zone_buff[code_rd_indx]); Serial.println(code_buff[code_rd_indx]);
        code_buff[code_rd_indx][0] = 0;
        code_rd_indx = ++code_rd_indx & CODE_BUFF_LEN_MASK; 
        //radio_send_handle.delay_task(2000);
    }
}

void scan_btn(void){
   #if defined(MH1_BTN) || defined(MH2_BTN)
   for(int i= 0;i<MAX_BTN;i++){
       butt[i].Scan();
   }    
   //scan_btn();
   #endif
   #ifdef K_BTN
   kbd.scan();
   qkbd.scan();
   #endif
}
