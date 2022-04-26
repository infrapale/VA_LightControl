/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

https://github.com/espressif/arduino-esp32/blob/master/libraries/SPI/examples/SPI_Multiple_Buses/SPI_Multiple_Buses.ino


 ****************************************************/


#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>

// For the Adafruit shield, these are the default.
#define TFT_DC    27
#define TFT_CS    33
#define TFT_MOSI  23  
#define TFT_MISO  19
#define TFT_SCK   18
#define TFT_RST   32

#define TOUCH_CS  25

XPT2046_Touchscreen ts(TOUCH_CS);
//#define TIRQ_PIN  2
//XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

#define NUMBER_OF_TILES 4

typedef enum
{
    SPI_CS_TFT = 0,
    SPI_CS_TOUCH,
    SPI_CS_SD
} spi_cs_et;

typedef struct
{
    uint16_t x;
    uint16_t y;
} pos_st;

typedef void (*show_cb_t)(void);

typedef struct 
{
    uint16_t x0;
    uint16_t y0;
    uint16_t width;
    uint16_t height;
    uint16_t curx;
    uint16_t cury;
    uint16_t touch_mid_x;
    uint16_t touch_mid_y;
    uint16_t border_color;
    uint16_t fill_color;
    uint16_t text_color;
    uint8_t  text_size;
    bool     refresh;
    bool     is_button;
    uint32_t value;
    //show_cb_t show_cb;
    void (*show_cb)();
    void (*press_cb)();
}  tile_st;

typedef struct 
{
    uint32_t x0;
    uint32_t y0;
    uint32_t xmax;
    uint32_t ymax;
}  touch_limit_st;


const uint8_t SPI_CS_SELECT[3] = {TFT_CS, TOUCH_CS, TFT_CS};
static tile_st tile[NUMBER_OF_TILES];
static touch_limit_st  tslim = {300,400,3800,3800};
const uint16_t line_spacing[6] = {0,10,20,30,40,50};



// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST, TFT_MISO);


void show_at_home(void)
{
      Serial.print("show_at_home(), value =");
      Serial.println(tile[0].value);
      tile_position(&tile[0],0,5+(uint16_t)tile[0].value,110,80);
      tile_set_color(&tile[0],ILI9341_LIGHTGREY,ILI9341_DARKGREY);
      tile_set_text(&tile[0],ILI9341_WHITE,2);
      tile_create(&tile[0]);
      tile_println(&tile[0],"Astrid");
      delay(500);
      tile_println(&tile[0],"at Home");
      delay(500);
      tile_println_value(&tile[0]);
      //tile_println_u32(&tile[0],42ULL);
      delay(500);
      yield();
}

void press_at_home(void)
{
    Serial.print("press_at_home(), value =");
    tile[0].value++;
    tile[0].refresh = true;
    Serial.println(tile[0].value);

}


void set_spi_cs( spi_cs_et spi_cs)
{
    for (uint8_t spi = SPI_CS_TFT; spi <= SPI_CS_TOUCH; spi++)
    {
        if (spi == spi_cs) digitalWrite(SPI_CS_SELECT[spi], LOW);
        else digitalWrite(SPI_CS_SELECT[spi], HIGH);       
    }
}



void setup() {
  Serial.begin(115200);
  Serial.println("ILI9341 Test!"); 
 
  Serial.println("initialize tile[]"); 
 
  pinMode(TFT_CS, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);

  for (uint8_t i=0; i < NUMBER_OF_TILES; i++ )
  {
      tile[i].x0 = 0;
      tile[i].y0 = 0;
      tile[i].width = 100;  //tft.width() / 2;
      tile[i].height = 100;  //tft.height() /2;
      tile[i].curx =  tile[i].x0+5;
      tile[i].cury =  tile[i].x0+5;
      tile[i].border_color = ILI9341_WHITE;
      tile[i].fill_color = ILI9341_BLUE;
      tile[i].text_color = ILI9341_YELLOW;
      tile[i].text_size = 2;
      tile[i].refresh = false;
      tile[i].is_button = false;
      tile[i].value = 0;
      tile[i].show_cb = NULL;
      tile[i].press_cb = NULL;
  }
  tile[0].width = 120;
  Serial.println("... initialize tile[] ...done");
  
  set_spi_cs(SPI_CS_TFT);
  tft.begin();

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  
  //Serial.println(F("Benchmark                Time (microseconds)"));
  //delay(10);
  //Serial.print(F("Screen fill              "));
  //Serial.println(testFillScreen());
  delay(500);
  
  

  
 
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  yield();
  tile[0].show_cb = show_at_home;
  tile[0].press_cb = press_at_home;
  tile[0].refresh = true;
  tile[0].is_button = true;

  
  /*
  tile_position(&tile[0],0,5,110,80);
  tile_set_color(&tile[0],ILI9341_LIGHTGREY,ILI9341_DARKGREY);
  tile_set_text(&tile[0],ILI9341_WHITE,2);
  tile_create(&tile[0]);
  tile_println(&tile[0],"Astrid");
  tile_println(&tile[0],"at Home");
  */
  delay(500);
  tile_position(&tile[1],120,5,180,80);
  tile_set_color(&tile[0],ILI9341_GREENYELLOW, ILI9341_DARKGREY);
  tile_set_text(&tile[1],ILI9341_WHITE,3);
  tile_set_button(&tile[1],true);
  tile_create(&tile[1]);
  tile_println(&tile[1],"Astrid");
  tile_println(&tile[1],"Automatic");
  delay(500);
 
  for (uint8_t i=0; i < NUMBER_OF_TILES; i++ )
  {
      if (tile[i].show_cb != NULL)
      {
          tile[i].show_cb();
      }
  }

  set_spi_cs(SPI_CS_TOUCH);

  ts.begin();
  ts.setRotation(3);
  boolean wastouched = true;
  pos_st xy;

  while(1)
  {
      set_spi_cs(SPI_CS_TOUCH);
      if (get_touch_pos(&xy))
      {   
          set_spi_cs(SPI_CS_TFT);
          yield();
          testText();
          //tft.fillScreen(ILI9341_BLACK);
          yield();
          Serial.print("x = ");
          Serial.print(xy.x);
          Serial.print(", y = ");
          Serial.println(xy.y);
          uint8_t btn_idx = get_pressed_button(&xy);
          if ( btn_idx < NUMBER_OF_TILES )
          {
              Serial.print("Button = ");
              Serial.println(btn_idx);
              tile_call_pressed_function(&tile[btn_idx]);
              tile[btn_idx].refresh = true;
          }

      }
      for (uint8_t i=0; i < NUMBER_OF_TILES; i++ )
      {
          tile_call_refresh_function(&tile[i]);
      }
      delay(100);
      yield();


  }

  tft.setRotation(0);
  
  Serial.print(F("Text                     "));
  Serial.println(testText());
  delay(3000);

  Serial.print(F("Lines                    "));
  Serial.println(testLines(ILI9341_CYAN));
  delay(500);

  Serial.print(F("Horiz/Vert Lines         "));
  Serial.println(testFastLines(ILI9341_RED, ILI9341_BLUE));
  delay(500);

  Serial.print(F("Rectangles (outline)     "));
  Serial.println(testRects(ILI9341_GREEN));
  delay(500);

  Serial.print(F("Rectangles (filled)      "));
  Serial.println(testFilledRects(ILI9341_YELLOW, ILI9341_MAGENTA));
  delay(500);

  Serial.print(F("Circles (filled)         "));
  Serial.println(testFilledCircles(10, ILI9341_MAGENTA));

  Serial.print(F("Circles (outline)        "));
  Serial.println(testCircles(10, ILI9341_WHITE));
  delay(500);

  Serial.print(F("Triangles (outline)      "));
  Serial.println(testTriangles());
  delay(500);

  Serial.print(F("Triangles (filled)       "));
  Serial.println(testFilledTriangles());
  delay(500);

  Serial.print(F("Rounded rects (outline)  "));
  Serial.println(testRoundRects());
  delay(500);

  Serial.print(F("Rounded rects (filled)   "));
  Serial.println(testFilledRoundRects());
  delay(500);

  Serial.println(F("Done!"));

}


uint8_t get_pressed_button(pos_st *xy)
{
    uint8_t res = NUMBER_OF_TILES + 1;
    for (uint8_t i= 0; i < NUMBER_OF_TILES; i++)
    {
        if (( xy->x > tile[i].x0 ) &&
            ( xy->x < tile[i].x0 + tile[i].width) &&
            ( xy->y > tile[i].y0 ) &&
            ( xy->y < tile[i].y0 + tile[i].height))
        {
            res = i;
            break;
        }
    }
    return res;
}


bool wastouched = false;

void loop(void) {
  for(uint8_t rotation=0; rotation<4; rotation++) {
    tft.setRotation(rotation);
    testText();
    delay(1000);
  }
}

boolean get_touch_pos(pos_st *xy)
{
    TS_Point p = ts.getPoint();
    boolean istouched = ts.touched();
    if (istouched) {
        TS_Point p = ts.getPoint();
        if (!wastouched) 
        {
            xy->x = (uint16_t) ((p.x * 320ULL) / tslim.xmax);
            xy->y = (uint16_t) ((p.y * 240ULL) / tslim.ymax);
            wastouched = true;
        }
        else 
        {
            istouched = false;
        }
    }
    else
    {
        wastouched = false;
    }
    return istouched;
}

void tile_call_pressed_function(tile_st *tp)
{
    if (tp->press_cb != NULL)
    {
        tp->press_cb();
    }
}

void tile_call_refresh_function(tile_st *tp)
{
    if ( tp->refresh)
    {
        Serial.println("refresh tile");
        if (tp->show_cb != NULL)
        { 
            Serial.println("show tile");
            tp->show_cb();
        }
        tp->refresh = false;
    }
}

void tile_position(tile_st *tp, uint16_t x0,uint16_t y0,uint16_t w,uint16_t h)
{
    tp->x0 = x0;
    tp->y0 = y0;
    tp->width = w;
    tp->height = h;
    tp->curx = x0+5;
    tp->cury = y0+5;
    // Setup Touch Point
    //tp->touch_mid_x = (uint16_t)(((uint32_t)(tp->x0 + tp->width / 2) * (tslim.xmax-tslim.xmin) ) / 320ULL);
    //tp->touch_mid_y = (uint16_t)(((uint32_t)(tp->y0 + tp->height / 2) * (tslim.ymax-tslim.ymin) ) / 320ULL);
}

void tile_set_color(tile_st *tp, uint16_t border_color, uint16_t fill_color )
{
    tp->border_color = border_color;
    tp->fill_color   = fill_color;
}

void tile_set_text(tile_st *tp, uint16_t color, uint8_t size )
{
    tp->text_color = color;
    tp->text_size  = size;
}

void tile_set_button(tile_st *tp, bool is_btn)
{
    tp->is_button = is_btn;
}
void tile_create(tile_st *tp)
{
    if (tp->is_button)
    {
        tft.fillRoundRect(tp->x0,tp->y0, tp->width, tp->height, 8, tp->fill_color );
        tft.drawRoundRect(tp->x0,tp->y0, tp->width, tp->height, 8, tp->border_color );
    }
    else
    {
        tft.fillRect(tp->x0,tp->y0, tp->width, tp->height, tp->fill_color);
        tft.drawRect(tp->x0,tp->y0, tp->width, tp->height, tp->border_color);
    }
    yield();
}

void tile_set_cursor(tile_st *tp)
//void tile_println(void)
{
    tft.setCursor(tp->x0+5,tp->y0+5);
    yield();
}

void tile_new_line(tile_st *tp)
{
    tft.setCursor(tp->curx,tp->cury);
    tp->cury += line_spacing[tp->text_size];
    yield();
}

void tile_println_value(tile_st *tp)
{
    tile_new_line(tp);
    tft.println(tp->value, HEX);
    tft.println();
    Serial.println(tp->value,HEX);

    yield();
}

void tile_println(tile_st *tp, char *txt)
{
    tft.setCursor(tp->curx,tp->cury);
    tft.setTextColor(tp->text_color);  
    tft.setTextSize(tp->text_size);
    tft.println(txt);
    tp->cury += line_spacing[tp->text_size];
    yield();
}

unsigned long testFillScreen() {
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.fillScreen(ILI9341_RED);
  yield();
  tft.fillScreen(ILI9341_GREEN);
  yield();
  tft.fillScreen(ILI9341_BLUE);
  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  return micros() - start;
}

unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  return micros() - start;
}

unsigned long testLines(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = tft.width(),
                h = tft.height();

  tft.fillScreen(ILI9341_BLACK);
  yield();
  
  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);
  t     = micros() - start; // fillScreen doesn't count against timing

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1    = w - 1;
  y1    = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1    = 0;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1    = w - 1;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);

  yield();
  return micros() - start;
}

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = tft.width(), h = tft.height();

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(y=0; y<h; y+=5) tft.drawFastHLine(0, y, w, color1);
  for(x=0; x<w; x+=5) tft.drawFastVLine(x, 0, h, color2);

  return micros() - start;
}

unsigned long testRects(uint16_t color) {
  unsigned long start;
  int           n, i, i2,
                cx = tft.width()  / 2,
                cy = tft.height() / 2;

  tft.fillScreen(ILI9341_BLACK);
  n     = min(tft.width(), tft.height());
  start = micros();
  for(i=2; i<n; i+=6) {
    i2 = i / 2;
    tft.drawRect(cx-i2, cy-i2, i, i, color);
  }

  return micros() - start;
}

unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
  unsigned long start, t = 0;
  int           n, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  n = min(tft.width(), tft.height());
  for(i=n; i>0; i-=6) {
    i2    = i / 2;
    start = micros();
    tft.fillRect(cx-i2, cy-i2, i, i, color1);
    t    += micros() - start;
    // Outlines are not included in timing results
    tft.drawRect(cx-i2, cy-i2, i, i, color2);
    yield();
  }

  return t;
}

unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(x=radius; x<w; x+=r2) {
    for(y=radius; y<h; y+=r2) {
      tft.fillCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int           x, y, r2 = radius * 2,
                w = tft.width()  + radius,
                h = tft.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for(x=0; x<w; x+=r2) {
    for(y=0; y<h; y+=r2) {
      tft.drawCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testTriangles() {
  unsigned long start;
  int           n, i, cx = tft.width()  / 2 - 1,
                      cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  n     = min(cx, cy);
  start = micros();
  for(i=0; i<n; i+=5) {
    tft.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      tft.color565(i, i, i));
  }

  return micros() - start;
}

unsigned long testFilledTriangles() {
  unsigned long start, t = 0;
  int           i, cx = tft.width()  / 2 - 1,
                   cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(i=min(cx,cy); i>10; i-=5) {
    start = micros();
    tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      tft.color565(0, i*10, i*10));
    t += micros() - start;
    tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      tft.color565(i*10, i*10, 0));
    yield();
  }

  return t;
}

unsigned long testRoundRects() {
  unsigned long start;
  int           w, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  w     = min(tft.width(), tft.height());
  start = micros();
  for(i=0; i<w; i+=6) {
    i2 = i / 2;
    tft.drawRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(i, 0, 0));
  }

  return micros() - start;
}

unsigned long testFilledRoundRects() {
  unsigned long start;
  int           i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(i=min(tft.width(), tft.height()); i>20; i-=6) {
    i2 = i / 2;
    tft.fillRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(0, i, 0));
    yield();
  }

  return micros() - start;
}