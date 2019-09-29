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
#include "lcd_text.h"
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

//**************************************************************************
// global variables
//**************************************************************************
TaskHandle_t Handle_aTask;
TaskHandle_t Handle_bTask;
TaskHandle_t Handle_monitorTask;

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
static void threadA( void *pvParameters ) 
{
  
  SERIAL.println("Thread A: Started");
  for(int x=0; x<20; ++x)
  {
    SERIAL.print("A");
    lcd_text_write(2, "thread A");
    lcd_text_show();  
    myDelayMs(2000);
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
  SERIAL.println("Thread B: Started");

  while(1)
  {
    SERIAL.println("B");
    lcd_text_write(3, "thread B");
    lcd_text_show();
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
    // String meas;
    char m_array[15];
    
    SERIAL.println("Task Monitor: Started");
 
    // run this task afew times before exiting forever
    for(x=0; x<10; ++x)
    {
      //LCD_Text_Write(0, "Task Monitor");
      SERIAL.println("");
      SERIAL.println("******************************");
      SERIAL.println("[Stacks Free Bytes Remaining] ");

      measurement = uxTaskGetStackHighWaterMark( Handle_aTask );
      sprintf(m_array,"Thread A: %d",measurement);
      SERIAL.println(m_array);
      lcd_text_write(1, m_array);
      
      measurement = uxTaskGetStackHighWaterMark( Handle_bTask );
      sprintf(m_array,"Thread B: %d",measurement);
      SERIAL.println(m_array);
      lcd_text_write(2, m_array);
      
      measurement = uxTaskGetStackHighWaterMark( Handle_monitorTask );
      sprintf(m_array,"Stack: %d",measurement);
      SERIAL.println(m_array);
      lcd_text_write(3, m_array);
      
      SERIAL.println("******************************");
      lcd_text_show();
      myDelayMs(10000); // print every 10 seconds
      
    }

    // delete ourselves.
    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    SERIAL.println("Task Monitor: Deleting");
    vTaskDelete( NULL );

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
 
  vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);

  // Create the threads that will be managed by the rtos
  // Sets the stack size and priority of each task
  // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
  xTaskCreate(threadA,     "Task A",       256, NULL, tskIDLE_PRIORITY + 3, &Handle_aTask);
  xTaskCreate(threadB,     "Task B",       256, NULL, tskIDLE_PRIORITY + 2, &Handle_bTask);
  xTaskCreate(taskMonitor, "Task Monitor", 1024, NULL, tskIDLE_PRIORITY + 1, &Handle_monitorTask);

  // Start the RTOS, this function will never return and will schedule the tasks.
  vTaskStartScheduler();

 
}



                                
void loop() {
      // Optional commands, can comment/uncomment below
    SERIAL.print("."); //print out dots in terminal, we only do this when the RTOS is in the idle state
    vNopDelayMS(100);

}


 
