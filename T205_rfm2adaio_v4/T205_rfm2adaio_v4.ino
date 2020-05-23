
/***************************************************

T205 RFM69 Relay to Adafruit IO
 
****************************************************/

#include <SPI.h>
#include <Wire.h>
#include "config.h"
#include <ESP8266WiFi.h>
//#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "ssd1306_feather.h"
#include <RH_RF69.h>
//#include "RTClib.h"
//#include <SPI.h>
//#include <SD.h>


/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 434.0   //915.0
#define SEALEVELPRESSURE_HPA (1013.25)
#define DISPLAY_FALLBACK_MILLIS 15000
#define SD_CS   16  //ESP8266

#if defined (__AVR_ATmega328P__)  // Feather 328P w/wing
  #define RFM69_INT     3  // 
  #define RFM69_CS      4  //
  #define RFM69_RST     2  // "A"
  #define LED           13
#endif

#if defined (Astrid_ATmega328P)  // made  by Tom Höglund
  #define RFM69_INT     2  // 
  #define RFM69_CS      10  //
  #define RFM69_RST     9  // 
  #define LED           13
#endif

#if defined(ESP8266)    // ESP8266 feather w/wing
  #define RFM69_CS      2    // "E"
  #define RFM69_RST     16   // "D"
  #define RFM69_INT     15   // "B"
  #define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ )
  #define LED           0
#endif

#if defined(ESP32)    // ESP32 feather w/wing
  #define RFM69_RST     13   // same as LED
  #define RFM69_CS      33   // "B"
  #define RFM69_INT     27   // "A"
  #define LED           13
#endif

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);
Adafruit_BME680 bme; // I2C
//RTC_PCF8523 rtc;
// set up variables using the SD utility library functions:
//Sd2Card card;
//SdVolume volume;
//SdFile root;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8

boolean bme_ok = false;

int16_t packetnum = 0;  // packet counter, we increment per xmission
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ONOFF_FEED");
Adafruit_MQTT_Subscribe slider = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/villaastrid.ilmalampopumppu-temp");

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish water_temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.water-temp");
Adafruit_MQTT_Publish dock_temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.dock-temp");
Adafruit_MQTT_Publish dock_pres = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.dock-pres");
Adafruit_MQTT_Publish dock_temp_dht22 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.dock-temp-dht22");
Adafruit_MQTT_Publish dock_hum_dht22 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.dock-hum-dht22");
Adafruit_MQTT_Publish dock_ldr1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.dock-ldr1");
Adafruit_MQTT_Publish dock_ldr2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.dock-ldr2");



Adafruit_MQTT_Publish od1_temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.outdoor1-temp");
Adafruit_MQTT_Publish od1_temp_dht22 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.outdoor1-temp-dht22");
Adafruit_MQTT_Publish od1_hum_dht22 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.outdoor1-hum-dht22");
Adafruit_MQTT_Publish od1_ldr1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.outdoor1-ldr1");
Adafruit_MQTT_Publish od1_ldr2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.outdoor1-ldr2");

Adafruit_MQTT_Publish od_PIR_front_door = Adafruit_MQTT_Publish(&mqtt, "infrapale/feeds/villaastrid.va-pir-front-door" );

Adafruit_MQTT_Publish od_PIR_terrace_door = Adafruit_MQTT_Publish(&mqtt, "infrapale/feeds/villaastrid.va-pir-terrace-door" );

Adafruit_MQTT_Publish tupa_temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.tupa-temp");
Adafruit_MQTT_Publish tupa_hum = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.tupa-hum");
Adafruit_MQTT_Publish tupa_pres = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.tupa-pres");
Adafruit_MQTT_Publish tupa_gas = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.tupa-gas");


//Adafruit_MQTT_Publish water_temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.water-temp");
//Adafruit_MQTT_Publish water_temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/villaastrid.water-temp");
Adafruit_MQTT_Publish test_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/test-feed");
Adafruit_MQTT_Publish* active_feed;

/*************************** Sketch Code ************************************/
long int poll_rfm_millis;
long int mqtt_connect_millis;
long int bme_read_millis;
long int display_default_millis;
long int bme_save_millis; 
byte bme_indx;
boolean sd_card_ok;
String yyyy_mm_dd;
//DateTime now;
//File myFile;

unsigned int pir_cntr_front_door = 0;
unsigned int pir_cntr_terrace_door = 0;


void setup() {
  delay(4000);
  ssd1306_setup();
//  if (! rtc.begin()) {
//    Serial.println("Couldn't find RTC");
//    while (1);
//  }
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.begin(115200);
  //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer
  Serial.println("T205 RFM69 to Adafruit IO Relay");
  row[0] = "T2015 Radio-WiFi";
  
  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    row[2] ="RFM69 failed, stopped";
    display_rows();
    while (1);
  }
  Serial.println("RFM69 radio init OK!");
  row[2]="RFM69 = OK";
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  uint8_t rfm69_key[] = RFM69_ENCR_KEY; //exactly the same 16 characters/bytes on all nodes!
  rf69.setEncryptionKey(rfm69_key);

/*  if (!SD.begin(SD_CS)) {
    Serial.println("SD initialization failed!");
    sd_card_ok = false;
  }
  else {
     Serial.println("SD initialization done.");
     sd_card_ok = true;
  }   
*/
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP().toString());
  row[1] = "IP:"; row[1].concat(WiFi.localIP().toString());
  Serial.println(row[1]);
 
  //mqtt.subscribe(&slider);
  
  // Setup MQTT subscription for onoff feed.
  // mqtt.subscribe(&onoffbutton);
  // connect to io.adafruit.com
  
  //now = rtc.now();
  //yyyy_mm_dd = String(now.year())+String(now.month())+String(now.day());
  //Serial.println(yyyy_mm_dd);

  poll_rfm_millis = millis();
  mqtt_connect_millis = millis();
  display_default_millis =millis();
  bme_read_millis = millis();
  bme_save_millis = millis();
  bme_ok = bme_setup();
  display_rows();
  bme_indx =0;
  //delay(5000);
}


void loop(void) {
   //now = rtc.now();
   // Serial.print(bme_ok); Serial.println(sd_card_ok);
   // ping the server to keep the mqtt connection alive
   if(! mqtt.ping()) {
     mqtt.disconnect();
   }   
   if ((millis() - mqtt_connect_millis ) > 1000 ){
      MQTT_connect();
      mqtt_connect_millis = millis();      
   }
   if ((millis() - bme_read_millis ) > 30000 ){
      if (bme_ok) publish_bme_data();
      bme_read_millis = millis();      
   }
    if ((millis() - bme_save_millis ) > 30000 ){
      //if (bme_ok && sd_card_ok) save_bme_data();
      bme_save_millis = millis();      
   }
   
   if ((millis() - display_default_millis ) > DISPLAY_FALLBACK_MILLIS ){
      if (bme_ok){
         display_bme_values();
      }
      display_default_millis = millis();      
   }

   Adafruit_MQTT_Subscribe *subscription;
   while ((subscription = mqtt.readSubscription(5000))) {
     if (subscription == &slider) {
       Serial.print(F("Got: "));
       Serial.println((char *)slider.lastread);
     }
   }  
   RadioRxHandler();
}
void display_bme_values(){
   row[0] = "Temp=";row[0].concat(bme.temperature); row[0].concat("C");
   row[0].concat(" Hum=");row[0].concat(String(bme.humidity,0)); row[0].concat("%");
   row[1] = "P=";row[1].concat(String(bme.pressure/100,0)); row[1].concat("hPa ");
   row[1].concat("Gas=");row[1].concat(String(bme.gas_resistance / 1000.0,0)); row[1].concat("kOhm");
   display_rows();
}

void save_bme_data(void){
  /*
  String file_name = "test001.txt";
  //String file_name = String(now.year()+'_'+now.month()+'_'+now.day()+".txt");
  //String file_name = String(now.year())+'_'+String(now.month())+'_'+String(now.day())+".txt";
  Serial.print("Filename= ");Serial.println(file_name);
  if (SD.exists(file_name)) {
    Serial.println("file exists.");
  } else {
    Serial.println("file doesn't exist.");
    myFile = SD.open(file_name, FILE_WRITE);
    myFile.close();
  }
  */
}

void publish_bme_data(void){
  float value;
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
  Serial.print("Pressure = "); Serial.print(bme.pressure / 100.0); Serial.println(" hPa");
  Serial.print("Humidity = "); Serial.print(bme.humidity); Serial.println(" %");
  Serial.print("Gas = "); Serial.print(bme.gas_resistance / 1000.0); Serial.println(" KOhms");
  
  switch(bme_indx){
    case 0: 
      value = bme.temperature;
      active_feed = &tupa_temp;
      break;
    case 1: 
      value = bme.humidity;
      active_feed = &tupa_hum;
      break;
    case 2: 
      value = bme.pressure;
      active_feed = &tupa_pres;
      break;
    case 3: 
      value = bme.gas_resistance;
      active_feed = &tupa_gas;
      break;
  }
  //Serial.print(active_feed -> feed);
  if (!active_feed -> publish(value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  if(++bme_indx > 3) bme_indx = 0;
}


void RadioRxHandler(void){
   String sensor_zone;
   String sensor_name;
   String sensor_value;
   String inputString;

   if (rf69.available()) {
    // Should be a message for us now   
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      Serial.print("Received [");
      Serial.print(len);
      Serial.print("]: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf69.lastRssi(), DEC);
      inputString = (char*)buf;
      sensor_zone = parse_json(inputString,"{\"Z");
      sensor_name = parse_json(inputString,",\"S");
      if (sensor_name = "") sensor_name = parse_json(inputString,",\"E");
      sensor_value = parse_json(inputString,",\"V");
      row[2] = sensor_zone; row[2].concat(":");
      row[2].concat(sensor_name);row[2].concat(":");
      row[2].concat(sensor_value);
      display_rows();
      Serial.println(sensor_zone); Serial.println(sensor_name); Serial.println(sensor_value);
      if (sensor_zone == "Dock"){
        if (sensor_name == "T_Water") active_feed = &water_temp;
        if (sensor_name == "T_bmp180") active_feed = &dock_temp;
        if (sensor_name == "P_bmp180") active_feed = &dock_pres;
        if (sensor_name == "T_dht22") active_feed = &dock_temp_dht22;
        if (sensor_name == "H_dht22") active_feed = &dock_hum_dht22;
        if (sensor_name == "ldr1") active_feed = &dock_ldr1;
        if (sensor_name == "ldr2") active_feed = &dock_ldr2;
      }
      if (sensor_zone == "OD_1"){
         if (sensor_name == "Temp") active_feed = &od1_temp;     
         if (sensor_name == "Temp2") active_feed = &od1_temp_dht22;     
         if (sensor_name == "Hum") active_feed = &od1_hum_dht22;     
         if (sensor_name == "Light1") active_feed = &od1_ldr1;     
         if (sensor_name == "Light2") active_feed = &od1_ldr2;
      }    
      if (sensor_zone == "TK"){
           if (sensor_name == "Etu-ovi") {
               active_feed = &od_PIR_front_door;   
               if (sensor_value == "PIR_Low") pir_cntr_front_door++;
               sensor_value = String(pir_cntr_front_door);
           }
           if (sensor_name == "Teras"){ 
               active_feed = &od_PIR_terrace_door; 
               if (sensor_value == "PIR_Low") pir_cntr_terrace_door++;
               sensor_value = String(pir_cntr_terrace_door);
               Serial.println( pir_cntr_terrace_door);
   
           }     
      } 
      Serial.println(sensor_value);

      if (! active_feed -> publish(sensor_value.toFloat())) {
         Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
    } else {
      Serial.println("Receive failed");
    }
  }  
}

String parse_json(String fromStr, char *tag){
   int pos1;
   int pos2;
   //Serial.println(fromStr); Serial.println(tag);
   pos1 = fromStr.indexOf(tag);
   if (pos1 >= 0){
       if (tag == ",\"V"){
          pos1 = fromStr.indexOf(":",pos1) +1;
          if (fromStr.charAt(pos1) == '\"') pos1++;
          pos2 = fromStr.indexOf(",",pos1);
          if (fromStr.charAt(pos2-1) == '\"') pos2--;
       }
       else{
          pos1 = fromStr.indexOf(":\"",pos1) +2;
          pos2 = fromStr.indexOf("\"",pos1);
       }
       return(fromStr.substring(pos1,pos2)); 
   }
   else {
      return("");
   }
    
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    // check if its the slider feed
    
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

boolean bme_setup(){
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    return(false);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  return(true);
}


void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
