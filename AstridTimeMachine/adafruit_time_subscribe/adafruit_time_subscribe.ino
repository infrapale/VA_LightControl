// Adafruit IO Time Topic Subscription Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Adam Bachman, Brent Rubell for Adafruit Industries
// Copyright (c) 2018 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

//https://www.tutorialspoint.com/c_standard_library/c_standard_library_quick_guide.htm

/************************** Configuration ***********************************/

#define ARDUINO_RUNNING_CORE 1
// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include "time.h"
#include <esp_task_wdt.h>

/************************ Example Starts Here *******************************/

const char* ssid     = WIFI_SSID;            //Main Router      
const char* password = WIFI_PASS;            //Main Router Password

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);

const char TIME_ISO_FEED[] PROGMEM = "time/ISO-8601";
Adafruit_MQTT_Subscribe time_iso_feed = Adafruit_MQTT_Subscribe(&mqtt, TIME_ISO_FEED);

static TaskHandle_t htask;
static SemaphoreHandle_t sema_wifi_avail;
static SemaphoreHandle_t sema_mqtt_avail;

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void setup() 
{
    BaseType_t rc;
    esp_err_t er;

    htask = xTaskGetCurrentTaskHandle();

    

    // start the serial connection
    Serial.begin(115200);

    // wait for serial monitor to open
    while(! Serial);

    /*
    Serial.println("Connecting WiFi");       
    WiFi.begin(WIFI_SSID,WIFI_PASS );
    if (WiFi.status() != WL_CONNECTED) 
    {
        Serial.println("*"); 
    }
    Serial.println("WiFi Connected");      
    */
    /*
    mqtt.subscribe(&time_iso_feed);
    Serial.print("Connecting to Adafruit IO");
    */
    StartTasks();
}

void loop() 
{   /*
    struct tm mqtt_tm = {0};
    struct tm tm = {0};
    struct tm *local_tm;
    time_t time;
    char dateStr[] = "2022-01-09";
    strptime(dateStr, "%Y-%m-%d", &tm);
    printf("%d ",tm.tm_year+1900);
    printf("%d ",tm.tm_mon);
    printf("%d ",tm.tm_mday);
    printf("\n");
    
    MQTT_connect();
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000))) {
        if (subscription == &time_iso_feed) {
            printf("Got %s\n",(char *)time_iso_feed.lastread);
            memset(&mqtt_tm,0x00,sizeof(mqtt_tm));
            strptime((char *)time_iso_feed.lastread, "%Y-%m-%dT%H", &mqtt_tm);
            //strptime((char *)time_iso_feed.lastread, "%Y-%m-%dT%H:%M:%S", &mqtt_tm);

            printf("%d ",mqtt_tm.tm_year+1900);
            printf("%d ",mqtt_tm.tm_mon+1);
            printf("%d ",mqtt_tm.tm_mday);
            printf("%d ",mqtt_tm.tm_hour);
            printf("%d ",mqtt_tm.tm_min);
            printf("%d ",mqtt_tm.tm_sec);
            printf("%d ",mqtt_tm.tm_wday);
            printf("%d ",mqtt_tm.tm_yday);
            printf("%d\n",mqtt_tm.tm_isdst);
            time= mktime(&mqtt_tm);
            printf("%d\n",time);
            time = time + 3*60*60;
            local_tm = localtime(&time);
            printf("%d ",local_tm->tm_year+1900);
            printf("%d ",local_tm->tm_mon+1);
            printf("%d ",local_tm->tm_mday);
            printf("%d ",local_tm->tm_hour);
            printf("%d ",local_tm->tm_min);
            printf("%d ",local_tm->tm_sec);
            printf("%d ",local_tm->tm_wday);
            printf("%d ",local_tm->tm_yday);
            printf("%d\n",local_tm->tm_isdst);
        }
    }
    */
    
}


void TaskConnectWiFi( void *pvParameters ){
    uint8_t state;
    int8_t  ret;
    uint8_t retries = 6;
    BaseType_t rc;
    esp_err_t er;

    state = 0;
    er = esp_task_wdt_add(nullptr);
    assert(er == ESP_OK);
    
    for (;;)
    {
        printf("WiFi state: %d\n", state);
        switch(state)
        {
            case 0:   // initial
                if (WiFi.status() != WL_CONNECTED){
                    Serial.println("Connecting WiFi");       
                    WiFi.begin(ssid, password); 
                    retries = 6;
                    state++;
                }
                else state = 2;
                vTaskDelay(1000);
                break;
            case 1:   // Check for the connection
                if (WiFi.status() != WL_CONNECTED) {
                    vTaskDelay(1000);
                    if (--retries == 0) state = 3;
                    else Serial.println("Waiting for WiFi"); 
                }
                else {
                    Serial.println("Connected to WiFi");
                    rc = xSemaphoreGive(sema_wifi_avail);
                    state = 2;
                }             
                esp_task_wdt_reset();
                vTaskDelay(1000);
                break;
            case 2:   // 
                if (WiFi.status() != WL_CONNECTED) state = 0;
                esp_task_wdt_reset();
                vTaskDelay(1000);
                break;
            case 3:   // 
                Serial.println("WiFi Retry limit reached -> WDT reset…");
                vTaskDelay(1000);  //        while (1); 
                break;
        }
    }
}





void TaskConnectMqtt( void *pvParameters ){
    uint8_t state;
    int8_t ret;
    uint8_t retries = 3;
    BaseType_t rc;
    esp_err_t er;
    uint8_t   wifi_timeout;
    uint8_t   mqtt_timeout;
    uint8_t   sensor_indx;
    uint8_t   interval_sec;
    
    state = 0;
    er = esp_task_wdt_add(nullptr);
    assert(er == ESP_OK);
    
    for (;;)
    { 
        printf("MQTT state: %d\n", state);
        switch(state) {
            case 0: // initial
                sensor_indx  = 0;
                state++;
                break;
            case 1: //Re-run MQTT action
                wifi_timeout = 10;
                mqtt_timeout = 10;
                state++;
                break;
                
            case 2: // waiting for WiFi
                rc = xSemaphoreTake(sema_wifi_avail, 1000);
                if(rc == pdPASS) state++;
                else {
                    if(--wifi_timeout == 0) state = 99;
                }
                esp_task_wdt_reset();
                vTaskDelay(1000);
                break;
            case 3: // WiFi is available
                if (mqtt.connected()){
                    printf("MQTT was already connected\n");
                    state++;
                }
                else {                
                    printf("Connecting to MQTT…\n ");  
                    if (WiFi.status() != WL_CONNECTED)
                    {
                        printf("WiFi is not connected\n ");  
                        state = 99;  //restart
                    }
                    else{
                        ret = mqtt.connect();
                        if (ret != 0) {    // connect will return 0 for connected
                            printf("%s\n",mqtt.connectErrorString(ret));
                            printf("Retrying MQTT connection…\n");
                            mqtt.disconnect();          
                            if (--mqtt_timeout == 0) state = 6;
                        }
                        else {
                            esp_task_wdt_reset();
                            vTaskDelay(100);
                            state++;
                        }
                    }
                }
                
            case 4: // MQTT is connected
                printf("MQTT is Connected!\n"); 
                rc = xSemaphoreGive(sema_mqtt_avail);
                esp_task_wdt_reset();
                state++;
                vTaskDelay(100);
                break;
            case 5: // MQTT actions
                if (mqtt.connected()){
                    /*
                    if (*sensor[sensor_indx].updated_ptr)
                    {
                        printf("%s %f\n",sensor[sensor_indx].caption,*sensor[sensor_indx].data_ptr);
                        sensor[sensor_indx].ada_mqtt_publ->publish(*sensor[sensor_indx].data_ptr);      
                    }
                    if(++sensor_indx >= NBR_SENSORS) sensor_indx = 0;
                    */
                    state++;
                    interval_sec = 15;

                } else {
                    interval_sec = 15;
                    state = 99;
                }
                esp_task_wdt_reset();
                vTaskDelay(1000);
                break;

            case 6: // Release WiFI and MQTT
                mqtt.disconnect();  
                rc = xSemaphoreGive(sema_wifi_avail);  
                state++;
                break;

            case 7: //Wait for next MQTT update
                if(--interval_sec == 0) state = 1;
                esp_task_wdt_reset();
                vTaskDelay(1000);          
                break;

            case 99: //
                printf("Retry limit reached -> WDT reset…\n");
                rc = xSemaphoreGive(sema_wifi_avail);
                rc = xSemaphoreGive(sema_mqtt_avail);
                vTaskDelay(10000);
                break;
            default:
                printf("Fatal error: incorrect MQTT state -> WDT reset…\n");
                vTaskDelay(100);
                state = 99;
                break;           
        }
    }
}







void StartTasks(void){
    BaseType_t rc;

    //rc = xSemaphoreTake(sema_wifi_avail,portMAX_DELAY);
    //assert(rc == pdPASS);
    //rc = xSemaphoreTake(sema_mqtt_avail,portMAX_DELAY);
    //assert(rc == pdPASS);

    
    xTaskCreatePinnedToCore(
       TaskConnectWiFi
        ,  "TaskConnectWiFi" 
        ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL 
        ,  ARDUINO_RUNNING_CORE);
    
  
    /*
    xTaskCreatePinnedToCore(
       TaskConnectMqtt
        ,  "TaskConnectMqtt" 
        ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL 
        ,  ARDUINO_RUNNING_CORE);
    */
    /* 
    xTaskCreatePinnedToCore(
       TaskSendMqtt
        ,  "TaskSendMqtt" 
        ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL 
        ,  ARDUINO_RUNNING_CORE);
    */
}
