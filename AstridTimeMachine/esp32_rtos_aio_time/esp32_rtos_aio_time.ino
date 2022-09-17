/**
 * RuuviTag Adafruit IO Gateway
 * @author TomHöglund 2021
 * 
 * https://github.com/ruuvi/ruuvi-sensor-protocols/blob/master/dataformat_03.md
 * https://tutorial.cytron.io/2020/01/15/send-sensors-data-to-adafruit-io-using-esp32/
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html
 * https://github.com/adafruit/Adafruit_MQTT_Library/blob/master/Adafruit_MQTT.h
 * 
 */
 
#include <Wire.h>
#include <SPI.h>
#include <time.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAddress.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <esp_task_wdt.h>
#include <Adafruit_Sensor.h>
include <ILI9341_t3.h>
//#include <font_Arial.h> // from ILI9341_t3
#include <XPT2046_Touchscreen.h>


#define ARDUINO_RUNNING_CORE 1

/// SSID Definitions
//#define  VILLA_ASTRID
//#define  H_MOKKULA
#define PIRPANA
#include "secrets.h"

#include "RuuviTag.h"
#include "config.h"
#include "helpers.h"

#define NBR_SENSORS               7       ///< Number of sensor values
#define CAPTION_LEN               40      ///< Length of value name
#define MAC_ADDR_LEN              18      ///< Length of the BLE MAC address string
#define MQTT_UPDATE_INTERVAL_ms   60000   ///< MQTT update interval, one value per time

#define WDT_TIMEOUT           10
//#define ARDUINO_RUNNING_CORE  1

#define LED_YELLOW   33
#define LDR_PIN      34
#define NBR_LDR_RES  5

#define TOUCH_CS  25

#define CS_PIN  8
#define TFT_DC  9
#define TFT_CS 10
// MOSI=11, MISO=12, SCK=13

XPT2046_Touchscreen ts(TOUCH_CS);
#define TIRQ_PIN  2
//XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);


int scanTime = 2; //In seconds
BLEScan* pBLEScan;
uint16_t ldr_value[NBR_LDR_RES];
uint8_t  ldr_select_pin[NBR_LDR_RES] = {15,16,17,18,19};

const char* ssid     = WIFI_SSID;            //Main Router      
const char* password = WIFI_PASS;            //Main Router Password


WiFiClient client;

typedef enum
{
    VALUE_TYPE_UNDEFINED = 0,
    VALUE_TYPE_FLOAT,
} value_type_et;


typedef struct
{
    Adafruit_MQTT_Publish *ada_mqtt_publ;
    char          caption[CAPTION_LEN];
    value_type_et value_type;
    float         *data_ptr;
    bool          *updated_ptr;   
    float         value;
    bool          updated;
} sensor_st;



void StartTasks(void);
static SemaphoreHandle_t sema_wifi_avail;
static SemaphoreHandle_t sema_mqtt_avail;

 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);
const char TIME_ISO_FEED[] PROGMEM = "time/ISO-8601";
Adafruit_MQTT_Subscribe time_iso_feed = Adafruit_MQTT_Subscribe(&mqtt, TIME_ISO_FEED);



//  Villa Astrid
Adafruit_MQTT_Publish va_parvi_temp  = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/villaastrid.parvi-temp");
Adafruit_MQTT_Publish va_parvi_hum   = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/villaastrid.parvi-hum");
Adafruit_MQTT_Publish va_parvi_light = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/villaastrid.parvi-light");
Adafruit_MQTT_Publish va_tupa_temp   = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/villaastrid.tupa-temp");
Adafruit_MQTT_Publish va_tupa_hum    = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/villaastrid.tupa-hum");
Adafruit_MQTT_Publish va_ulko_temp   = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/villaastrid.ulko-temp");
Adafruit_MQTT_Publish va_ulko_hum    = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/villaastrid.ulko-hum");




/// Sensor MQTT pointers, other data will be initialized in the setup function
sensor_st sensor[NBR_SENSORS]= 
{
    { &va_tupa_temp, "", VALUE_TYPE_FLOAT, NULL,NULL,0.0,false}, 
    { &va_tupa_hum, "", VALUE_TYPE_FLOAT, NULL,NULL,0.0,false}, 
    { &va_ulko_temp, "", VALUE_TYPE_FLOAT, NULL,NULL,0.0,false}, 
    { &va_ulko_hum, "", VALUE_TYPE_FLOAT, NULL,NULL,0.0,false},
    { &va_parvi_temp, "", VALUE_TYPE_FLOAT, NULL,NULL,0.0,false},
    { &va_parvi_hum, "", VALUE_TYPE_FLOAT, NULL,NULL,0.0,false}, 
    { &va_parvi_light, "", VALUE_TYPE_FLOAT, NULL,NULL,0.0,false}, 
};

RuuviTag  ruuvi_tag;

extern bool         loopTaskWDTEnabled;
static TaskHandle_t htask;


/**
 * Class that scans for BLE devices
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
    void onResult(BLEAdvertisedDevice advertisedDevice) 
    {
        //Scans for specific BLE MAC addresses 
        //Serial.println(advertisedDevice.getAddress().toString().c_str());
        esp_task_wdt_reset();

        String mac_addr = advertisedDevice.getAddress().toString().c_str();
        String raw_data = String(BLEUtils::buildHexData(nullptr, (uint8_t*)advertisedDevice.getManufacturerData().data(), advertisedDevice.getManufacturerData().length()));
        raw_data.toUpperCase();

        ruuvi_tag.decode_raw_data(mac_addr, raw_data, advertisedDevice.getRSSI());  
       
    }
    
};



void setup() 
{
    BaseType_t rc;
    esp_err_t er;

    htask = xTaskGetCurrentTaskHandle();
    loopTaskWDTEnabled = true;
    
    delay(4000);
    Serial.begin(115200);   

    er = esp_task_wdt_status(htask);
    assert(er == ESP_ERR_NOT_FOUND);
    // Initialize 
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(ILI9341_BLACK);
    ts.begin();
    ts.setRotation(1);
    

    if ( er == ESP_ERR_NOT_FOUND ) {
        er = esp_task_wdt_init(10,true);
        assert(er == ESP_OK);
        er = esp_task_wdt_add(htask);
        assert(er == ESP_OK);
        printf("Task is subscribed to TWDT.\n");
    }
    
    pinMode(LED_YELLOW, OUTPUT);
    digitalWrite(LED_YELLOW,LOW);

    Serial.println("Setup BLE Scanning...");
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  // less or equal setInterval value
    
    pinMode(LED_YELLOW, OUTPUT);
    digitalWrite(LED_YELLOW,LOW);

    pinMode(LDR_PIN,INPUT);

    /// Refine ruuvi tag data
    //ruuvi_tag.add(String("e6:2c:8d:db:22:35"),"home_indoor");
    ruuvi_tag.add(String("ea:78:e2:12:36:f8"),"va_tupa");
    ruuvi_tag.add(String("ed:9a:ab:c6:30:72"),"va_outdoor");

    /// Ruuvi Sensors
    strncpy(sensor[0].caption,ruuvi_tag.ruuvi[0].location,CAPTION_LEN-RUUVI_LOCATION_LEN);
    strcat(sensor[0].caption, "_temperature");

    strncpy(sensor[1].caption,ruuvi_tag.ruuvi[0].location,CAPTION_LEN-RUUVI_LOCATION_LEN);
    strcat(sensor[1].caption, "_humidity");

    strncpy(sensor[2].caption,ruuvi_tag.ruuvi[1].location,CAPTION_LEN-RUUVI_LOCATION_LEN);
    strcat(sensor[2].caption, "_temperature");
    
    strncpy(sensor[3].caption,ruuvi_tag.ruuvi[1].location,CAPTION_LEN-RUUVI_LOCATION_LEN);
    strcat(sensor[3].caption, "_humidity");
    
    // BME680 Sensors    
    strcpy(sensor[4].caption,"va_parvi_temperature");
    strcpy(sensor[5].caption,"va_parvi_humidity");
    // LDR Sensor
    strcpy(sensor[6].caption,"va_parvi_light");

    //strncpy(sensor[2].caption,ruuvi_tag.ruuvi[2].location,CAPTION_LEN-RUUVI_LOCATION_LEN);
    //strcat(sensor[2].caption, "_temperature");
    
    
    sensor[0].data_ptr = &ruuvi_tag.ruuvi[0].temp_fp;
    sensor[1].data_ptr = &ruuvi_tag.ruuvi[0].humidity;
    sensor[2].data_ptr = &ruuvi_tag.ruuvi[1].temp_fp;
    sensor[3].data_ptr = &ruuvi_tag.ruuvi[1].humidity;

    sensor[0].updated_ptr = &ruuvi_tag.ruuvi[0].updated;
    sensor[1].updated_ptr = &ruuvi_tag.ruuvi[0].updated;
    sensor[2].updated_ptr = &ruuvi_tag.ruuvi[1].updated;
    sensor[3].updated_ptr = &ruuvi_tag.ruuvi[1].updated;

    for (uint8_t i = 4; i < 7;i++){
        sensor[i].data_ptr    = &sensor[i].value;
        sensor[i].updated_ptr = &sensor[i].updated;       
    }
 
    sema_wifi_avail = xSemaphoreCreateBinary();
    sema_mqtt_avail = xSemaphoreCreateBinary();

    assert(sema_wifi_avail);
    assert(sema_mqtt_avail);
    rc = xSemaphoreGive(sema_wifi_avail);
    rc = xSemaphoreGive(sema_mqtt_avail);
    StartTasks();
}

void loop() {
    esp_err_t er;
    er = esp_task_wdt_add(nullptr);
    er = esp_task_wdt_status(htask);
    assert(er == ESP_OK);
    esp_task_wdt_reset();
    vTaskDelay(1000);
}

void StartTasks(void){
    BaseType_t rc;

    rc = xSemaphoreTake(sema_wifi_avail,portMAX_DELAY);
    assert(rc == pdPASS);
    rc = xSemaphoreTake(sema_mqtt_avail,portMAX_DELAY);
    assert(rc == pdPASS);

    
    xTaskCreatePinnedToCore(
       TaskConnectWiFi
        ,  "TaskConnectWiFi" 
        ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL 
        ,  ARDUINO_RUNNING_CORE);
    
  
    xTaskCreatePinnedToCore(
       TaskConnectMqtt
        ,  "TaskConnectMqtt" 
        ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL 
        ,  ARDUINO_RUNNING_CORE);


     xTaskCreatePinnedToCore(
       TaskScanBle
        ,  "TaskScanBle" 
        ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL 
        ,  ARDUINO_RUNNING_CORE);

      
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
        //printf("WiFi state: %d\n", state);
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
                    digitalWrite(LED_YELLOW,LOW);
                    vTaskDelay(1000);
                    if (--retries == 0) state = 3;
                    else Serial.println("Waiting for WiFi"); 
                }
                else {
                    digitalWrite(LED_YELLOW,HIGH);
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
    struct    tm mqtt_tm = {0};
    struct    tm *local_tm;
    time_t    time;


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
                    else
                    {
                        ret = mqtt.connect();
                        if (ret != 0)  // connect will return 0 for connected
                        {   
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
                    if (*sensor[sensor_indx].updated_ptr)
                    {
                        printf("%s %f\n",sensor[sensor_indx].caption,*sensor[sensor_indx].data_ptr);
                        sensor[sensor_indx].ada_mqtt_publ->publish(*sensor[sensor_indx].data_ptr);      
                    }
                    if(++sensor_indx >= NBR_SENSORS) sensor_indx = 0;
                    state++;
                    interval_sec = 15;

                } else {
                    interval_sec = 15;
                    state = 99;
                }
                esp_task_wdt_reset();
                vTaskDelay(1000);
                break;
            
            case 6:  // Fetch time
                if (mqtt.connected())
                {
                    Adafruit_MQTT_Subscribe *subscription;
                    while ((subscription = mqtt.readSubscription(5000))) 
                    {
                        if (subscription == &time_iso_feed) 
                        {
                            printf("Got %s\n",(char *)time_iso_feed.lastread);
                            memset(&mqtt_tm,0x00,sizeof(mqtt_tm));
                            strptime((char *)time_iso_feed.lastread, "%Y-%m-%dT%H", &mqtt_tm);
                        }    
                    }
                }  
                state++;  
                esp_task_wdt_reset();
                vTaskDelay(1000);
                break;    

            case 7: // Release WiFI and MQTT
                mqtt.disconnect();  
                rc = xSemaphoreGive(sema_wifi_avail);  
                state++;
                break;

            case 8: //Wait for next MQTT update
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

void TaskScanBle( void *pvParameters ){
    BaseType_t rc;
    esp_err_t er;
    uint8_t   state = 0;
    BLEScanResults foundDevices;
    
    er = esp_task_wdt_add(nullptr);
    assert(er == ESP_OK);

    for (;;)
    {   
        printf("Scan BLE state: %d\n", state);
        switch(state) {
            case 0:   // Initial state
                state++;
                break; 
            case 1:   // BLE Scan
                foundDevices = pBLEScan->start(scanTime, false);          
                esp_task_wdt_reset();
                state++;
                vTaskDelay(4000);           
                break; 
            case 2:   // Clear BLE results
                pBLEScan->clearResults(); 
                esp_task_wdt_reset();
                state--;
                vTaskDelay(1000);
                break; 
        }     
    }
}

/*
void TaskSendMqtt( void *pvParameters ){
    
    for (;;)
    {   
        digitalWrite(LED_BLUE,HIGH);
        vTaskDelay(1000);
        digitalWrite(LED_BLUE,LOW);
        vTaskDelay(1000);
    }
}
*/
/*
void lo_op() 
{ 
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
    
}
*/