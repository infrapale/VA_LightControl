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

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include "time.h"

/************************ Example Starts Here *******************************/

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);

const char TIME_ISO_FEED[] PROGMEM = "time/ISO-8601";
Adafruit_MQTT_Subscribe time_iso_feed = Adafruit_MQTT_Subscribe(&mqtt, TIME_ISO_FEED);

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

    // start the serial connection
    Serial.begin(115200);

    // wait for serial monitor to open
    while(! Serial);

    Serial.println("Connecting WiFi");       
    WiFi.begin(WIFI_SSID,WIFI_PASS );
    if (WiFi.status() != WL_CONNECTED) 
    {
        Serial.println("*"); 
    }
    Serial.println("WiFi Connected");      


    mqtt.subscribe(&time_iso_feed);
    Serial.print("Connecting to Adafruit IO");

}

void loop() 
{
    struct tm mqtt_tm = {0};
    struct tm *local_tm;
    time_t time;
    MQTT_connect();
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000))) {
        if (subscription == &time_iso_feed) {
            printf("Got %s\n",(char *)time_iso_feed.lastread);
            memset(&mqtt_tm,0x00,sizeof(mqtt_tm));
            strptime((char *)time_iso_feed.lastread, "%Y-%m-%dT%H:%M:%S", &mqtt_tm);
            printf("%d ",mqtt_tm.tm_year+1900);
            printf("%d ",mqtt_tm.tm_mon);
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
            printf("%d ",local_tm->tm_mon);
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
