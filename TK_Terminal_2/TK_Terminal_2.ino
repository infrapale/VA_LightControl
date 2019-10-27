/*********************************************************************
This is an example sketch for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include "Arduino.h"
#include <SPI.h>
#include <FreeRTOS_SAMD21.h> //samd21
#include <task.h>
#include <queue.h>
#include <RH_RF69.h>
#include <Secret.h>
#include "lcd_text.h"
#include "json.h"
#include "AnalogKbd.h"
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define PCD_DC 5
#define PCD_CS 6
#define PCD_RST 9
#define PCD_BL 10
#define PCD_ROWS 4
#define PCD_ROW_LEN 15   // 14 characters

#define  ERROR_LED_PIN  13 //Led Pin: Typical Arduino Board
//#define  ERROR_LED_PIN  2 //Led Pin: samd21 xplained board

#define ERROR_LED_LIGHTUP_STATE  LOW // the state that makes the led light up on your board, either low or high

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
#define SERIAL          Serial
#define ANALOG_KBD_COL_1_2 1
#define ANALOG_KBD_COL_3_4 2
#define RADIO_MSG_LEN   60

// Adafruit M0+RFM60 Feather:
// #8 - used as the radio CS (chip select) pin
// #3 - used as the radio GPIO0 / IRQ (interrupt request) pin.
// #4 - used as the radio Reset pin

#define RFM69_TX_IVAL_100ms  20

void Init_RFM69(byte rfm_rst_pin,float rfm_freq);
void RadiateMsg(char *rf69_msg );

akbd kbd(ANALOG_KBD_COL_1_2,ANALOG_KBD_COL_3_4,KBD_4x4);
RH_RF69 rf69(RFM69_CS, RFM69_IRQN);
//**************************************************************************
// global variables
//**************************************************************************
TaskHandle_t Handle_aTask;
TaskHandle_t Handle_bTask;
TaskHandle_t Handle_monitorTask;
TaskHandle_t Handle_lcdQueueTask;
TaskHandle_t Handle_radio;

//**************************************************************************
// Declare a variable of type QueueHandle_t.  This is used to store the queue
// that is accessed by all three tasks.
//**************************************************************************

QueueHandle_t lcdQueue;
QueueHandle_t radio_tx_Queue;

unsigned int b_cnt = 0;
//**************************************************************************
// Can use these function for RTOS delays
// Takes into account procesor speed
//**************************************************************************
void myDelayUs(int us)
{
  vTaskDelay( us / portTICK_PERIOD_US );  
}

void myDelayMs(int ms)
{
  vTaskDelay( (ms * 1000) / portTICK_PERIOD_US );  
}

void myDelayMsUntil(TickType_t *previousWakeTime, int ms)
{
  vTaskDelayUntil( previousWakeTime, (ms * 1000) / portTICK_PERIOD_US );  
}

//*****************************************************************
// Create a thread that prints out A to the screen every two seconds
// this task will delete its self after printing out afew messages
//*****************************************************************
#define SCAN_PER_READ_KBD 10
char tx_buff[RADIO_MSG_LEN];

void send_radio(char category, char *zone,char sub_index,char command){
    BaseType_t radio_tx_Status;
    make_json_relay_array(tx_buff, category, zone, sub_index,command);
    radio_tx_Status = xQueueSendToBack( radio_tx_Queue, tx_buff, pdMS_TO_TICKS(1000UL));
    radiate_msg(tx_buff);
    SERIAL.println(tx_buff);
}

static void threadA( void *pvParameters ) 
{
  
  SERIAL.println("Thread A: Started");
  int scan_per_read = SCAN_PER_READ_KBD;
  while(true)
  {
    // SERIAL.print("A");
    kbd.scan();
    if(scan_per_read-- == 0) 
    { 
      char key = kbd.read(); 
      if(key != 0)
      {   
          SERIAL.print("0: ");
          SERIAL.print(kbd.rd_analog(0));
          SERIAL.print(" 1: ");
          SERIAL.print(kbd.rd_analog(1));
          SERIAL.print(" Key = ");
          SERIAL.println(key);

          switch(key) {
              case '0': send_radio('R',"ALL",'G','0'); break;
              case '1': send_radio('R',"MH1",'1','T'); break;
              case '2': send_radio('R',"MH2",'1','T'); break;
              case '3': send_radio('R',"ET_",'1','T'); break;
              case '4': send_radio('R',"MH1",'2','T'); break;
              case '5': send_radio('R',"MH2",'2','T'); break;
              case '6': send_radio('R',"K__",'1','T'); break;
              case '7': send_radio('R',"K__",'2','T'); break;
              case '8': send_radio('R',"TUP",'1','T'); break;
              case '9': send_radio('R',"TUP",'2','T'); break;
              case '*': send_radio('R',"ALL",'1','1'); break;
              case '#': send_radio('R',"PAR",'1','T'); break;
              case 'A': send_radio('R',"KHH",'1','T'); break;
              case 'B': send_radio('R',"PSH",'1','T'); break;
              case 'C': send_radio('R',"SAU",'1','T'); break;
              case 'D': send_radio('R',"ULK",'1','T'); break;
          }
      } 
      scan_per_read = SCAN_PER_READ_KBD;
    } 
    myDelayMs(50);
  }
  
  // delete ourselves.
  // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
  SERIAL.println("Thread A: Deleting");
  vTaskDelete( NULL );
}

//*****************************************************************
// Create a thread that prints out B to the screen every second
// this task will run forever
//*****************************************************************
static void threadB( void *pvParameters ) 
{
    
    char send_buff[PCD_ROW_LEN+1];
    int measurement;
    BaseType_t lcdStatus;
 
    SERIAL.println("Thread B: Started");

    while(1)
    {
        b_cnt++;
        measurement = uxTaskGetStackHighWaterMark( Handle_bTask );
        sprintf(send_buff,"%1dThread B: %d",2, b_cnt);
        SERIAL.println(send_buff);
        lcdStatus = xQueueSendToBack( lcdQueue, send_buff, pdMS_TO_TICKS(1000UL));
        myDelayMs(5000);
    }

}

//*****************************************************************
// Task will periodicallt print out usefull information about the tasks running
// Is a usefull tool to help figure out stack sizes being used
//*****************************************************************
void taskMonitor(void *pvParameters)
{
    int x;
    int measurement;
    char m_array[PCD_ROW_LEN+1];
    BaseType_t lcdStatus;
     
    SERIAL.println("Task Monitor: Started");
 
    // run this task afew times before exiting forever
    for(x=0; x<100; ++x)
    {
      //LCD_Text_Write(0, "Task Monitor");
      SERIAL.println("");
      SERIAL.println("******************************");
      SERIAL.println("[Stacks Free Bytes Remaining] ");

      measurement = uxTaskGetStackHighWaterMark( Handle_aTask );
      sprintf(m_array,"%1dThread A: %d",1,measurement);
      SERIAL.println(m_array);
      lcdStatus = xQueueSendToBack( lcdQueue, m_array, pdMS_TO_TICKS(1000UL));
      if(lcdStatus != pdPASS){
           SERIAL.println("Send to LCD queue failed");
      }
      /*
      measurement = uxTaskGetStackHighWaterMark( Handle_bTask );
      sprintf(m_array,"%1dThread B: %d",2,measurement);
      SERIAL.println(m_array);
      lcdStatus = xQueueSendToBack( lcdQueue, m_array, pdMS_TO_TICKS(1000UL));
      */
      measurement = uxTaskGetStackHighWaterMark( Handle_monitorTask );
      sprintf(m_array,"%1dStack: %d",3,measurement);
      SERIAL.println(m_array);
      lcdStatus = xQueueSendToBack( lcdQueue, m_array, pdMS_TO_TICKS(1000UL));
      
      SERIAL.println("******************************");
      // lcd_text_show();
      myDelayMs(10000); // print every 10 seconds
    
    }

    // delete ourselves.
    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    SERIAL.println("Task Monitor: Deleting");
    vTaskDelete( NULL );

}

static void lcdQueueTask( void *pvParameters )
{
    /* Declare the variable that will hold the values received from the queue. */
    char m_array[PCD_ROW_LEN+1];
    BaseType_t lcdStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 1000UL );

  /* This task is also defined within an infinite loop. */
  for( ;; )
  {
    /* As this task unblocks immediately that data is written to the queue this
    call should always find the queue empty. */
    if( uxQueueMessagesWaiting( lcdQueue ) != 0 )
    {
      Serial.println( "Queue should have been empty!" );
    }

    /* The first parameter is the queue from which data is to be received.  The
    queue is created before the scheduler is started, and therefore before this
    task runs for the first time.

    The second parameter is the buffer into which the received data will be
    placed.  In this case the buffer is simply the address of a variable that
    has the required size to hold the received data.

    the last parameter is the block time – the maximum amount of time that the
    task should remain in the Blocked state to wait for data to be available should
    the queue already be empty. */
    lcdStatus = xQueueReceive( lcdQueue, m_array, pdMS_TO_TICKS( 5000UL ) );

    if( lcdStatus == pdPASS )
    {
        /* Data was successfully received from the queue, print out the received
        value. */
        
        lcd_text_write(m_array[0]-'0', &m_array[1]);
        lcd_text_show();
        //vPrintStringAndNumber( "Received = ", lReceivedValue );
    }
    else
    {
      /* We did not receive anything from the queue even after waiting for 100ms.
      This must be an error as the sending tasks are free running and will be
      continuously writing to the queue. */
      //vPrintString( "Could not receive from the queue.\r\n" );
      lcd_text_write(3, "no data!");
      lcd_text_show();
    }
  }
}

static void radioTask( void *pvParameters )
{
    char tx_array[RADIO_MSG_LEN];
    BaseType_t radio_tx_status;
    /* Declare the variable that will hold the values received from the queue. */
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 100UL );

  /* This task is also defined within an infinite loop. */
  for( ;; )
  {
    /* As this task unblocks immediately that data is written to the queue this
    call should always find the queue empty. */
    if( uxQueueMessagesWaiting( radio_tx_Queue ) != 0 )
    {
        Serial.println( "Queue should have been empty!" );
    }
    //radio_tx_status = xQueueReceive( radio_tx_Queue, tx_array, pdMS_TO_TICKS( 500UL ) );

    if( radio_tx_status == pdPASS )
    {
        radiate_msg(tx_array);
        /* Data was successfully received from the queue, print out the received
        value. */

    }
    else
    {
      /* We did not receive anything from the queue even after waiting for 100ms.
      This must be an error as the sending tasks are free running and will be
      continuously writing to the queue. */
    }
  }
}







void setup()   {
    SERIAL.begin(115200);
    vNopDelayMS(1000); // prevents usb driver crash on startup, do not omit this
    while (!SERIAL) ;  // Wait for serial terminal to open port before starting program
    SERIAL.println("TK Light Terminal");

  
    lcd_text_init();
    lcd_text_clear();
    lcd_text_write(0,"-TK Terminal-");
    lcd_text_write(1,"rivi 1");
    lcd_text_write(2,"rivi 2");
    lcd_text_write(3,"rivi 3");
    lcd_text_show();
    kbd.begin();

    Init_RFM69(RFM69_RST,RFM69_FREQ);
    radiate_msg("RFM69 test message");

    lcdQueue = xQueueCreate( 4, sizeof( char )*(PCD_ROW_LEN+1) );
    if( lcdQueue != NULL) 
    {
 
        vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);

        // Create the threads that will be managed by the rtos
        // Sets the stack size and priority of each task
        // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
        xTaskCreate(threadA,     "Task A",       256, NULL, tskIDLE_PRIORITY + 3, &Handle_aTask);
        xTaskCreate(threadB,     "Task B",       256, NULL, tskIDLE_PRIORITY + 2, &Handle_bTask);
        xTaskCreate(taskMonitor, "Task Monitor", 1024, NULL, tskIDLE_PRIORITY + 1, &Handle_monitorTask);
        xTaskCreate(lcdQueueTask, "LCD Queue", 1024, NULL, tskIDLE_PRIORITY + 1, &Handle_lcdQueueTask);
        // xTaskCreate(radioTask, "RFM 69", 2048, NULL, tskIDLE_PRIORITY + 3, &Handle_radio);

        // Start the RTOS, this function will never return and will schedule the tasks.
        vTaskStartScheduler();
    }
    else
    {
        SERIAL.println("LCD queue failure");
    }
  
}



                                
void loop() {
      // Optional commands, can comment/uncomment below
    // SERIAL.print("."); //print out dots in terminal, we only do this when the RTOS is in the idle state
    vNopDelayMS(100);

}

void Init_RFM69(byte rfm_rst_pin,float rfm_freq)
{   
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, LOW);


     // manual reset
     digitalWrite(RFM69_RST, HIGH);
     delay(100);
     digitalWrite(RFM69_RST, LOW);
     delay(100);
  
     if (!rf69.init()) {
         Serial.println("RFM69 rf69 init failed");
         while (1);
     }
     Serial.println("RFM69 rf69 init OK!");
  
     // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
     // No encryption
     if (!rf69.setFrequency(rfm_freq)) {
          Serial.println("setFrequency failed");
     }

     // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
     // ishighpowermodule flag set like this:
     rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

     // in secret.h          1234567890123456
     //uint8_t rfm69_key[] ="Xyzabde123456789"; //exactly the same 16 characters/bytes on all nodes!
     rf69.setEncryptionKey(rfm69_key);
     Serial.print("RFM69 rf69 @");  Serial.print((int)rfm_freq,DEC);  Serial.println(" MHz");
    
} 

void radiate_msg( char *rf69_msg ) {
    rf69.waitPacketSent();
    rf69.send((uint8_t*)rf69_msg, strlen(rf69_msg));
}

 
