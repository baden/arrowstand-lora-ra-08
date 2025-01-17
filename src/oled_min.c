// ===================================================================================
// SSD1306 128x64 Pixels OLED Minimal Functions                               * v1.0 *
// ===================================================================================
//
// Collection of the most necessary functions for controlling an SSD1306 128x64 pixels
// I2C OLED.
//
// References:
// -----------
// - TinyOLEDdemo: https://github.com/wagiminator/attiny13-tinyoleddemo
// - Neven Boyanov: https://github.com/tinusaur/ssd1306xled
// - Stephen Denne: https://github.com/datacute/Tiny4kOLED
//
// 2022 by Stefan Wagner: https://github.com/wagiminator

#include "oled_min.h"
#include "i2c.h"

// OLED initialisation sequence
// const uint8_t OLED_INIT_CMD[] = {
//   OLED_MULTIPLEX,   0x3F,                 // set multiplex ratio  
//   OLED_CHARGEPUMP,  0x14,                 // set DC-DC enable  
//   OLED_MEMORYMODE,  0x00,                 // set horizontal addressing mode
//   OLED_COLUMNS,     0x00, 0x7F,           // set start and end column
//   OLED_PAGES,       0x00, 0x3F,           // set start and end page
//   OLED_COMPINS,     0x12,                 // set com pins
//   OLED_XFLIP, OLED_YFLIP,                 // flip screen
//   OLED_DISPLAY_ON                         // display on
// };

// // OLED initialisation sequence
// const uint8_t OLED_INIT_CMD[] = {
//   OLED_MULTIPLEX,   0x3F,                 // set multiplex ratio  
//   OLED_CHARGEPUMP,  0x14,                 // set DC-DC enable  
//   OLED_MEMORYMODE,  0x00,                 // set page addressing mode
//   OLED_COLUMNS,     0x00, 0x7F,           // set start and end column
//   OLED_PAGES,       0x00, 0x3F,           // set start and end page
//   OLED_COMPINS,     0x12,                 // set com pins
//   OLED_XFLIP, OLED_YFLIP,                 // flip screen
//   OLED_DISPLAY_ON                         // display on
// };


// По цьому вдалось адаптувати 128x32
// https://github.com/adafruit/Adafruit_SSD1306/blob/master/Adafruit_SSD1306.cpp

#define SSD1306_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

#define vccstate SSD1306_SWITCHCAPVCC


#if ((OLED_WIDTH == 128) && (OLED_HEIGHT == 32)) 
  #define comPins 0x02
  #define contrast 0x8F
#elif ((OLED_WIDTH == 128) && (OLED_HEIGHT == 64))
  #define comPins 0x12
  #define contrast  ((vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF)
#elif ((OLED_WIDTH == 96) && (OLED_HEIGHT == 16)) 
  #define comPins 0x2
  #define contrast  ((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0xAF)
#endif


// OLED initialisation sequence
const uint8_t OLED_INIT_CMD[] = {
  OLED_DISPLAY_OFF,
  OLED_CLOCKDIV,    0x80,           // set clock divide ratio
  OLED_MULTIPLEX,   (OLED_HEIGHT-1),       // set multiplex ratio (HEIGHT-1)
  OLED_OFFSET, 0x00,                // set display offset
  OLED_STARTLINE | 0x00,            // set display start line
  OLED_CHARGEPUMP, 0x14,            // set DC-DC enable (0x10-external VCC, 0x14 - internal DC-DC)
  OLED_MEMORYMODE, 0x00,            // set horizontal addressing mode
  OLED_XFLIP, OLED_YFLIP,           // flip screen

  OLED_COMPINS, comPins,
  OLED_CONTRAST, contrast,
  OLED_SETPRECHARGE, ((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1),
  OLED_SETVCOMDETECT, 0x40,
  OLED_DISPLAYALLON_RESUME,
  OLED_INVERT_OFF,
  OLED_SCROLL_OFF,
  OLED_DISPLAY_ON,                 // display on
};

// OLED init function
void OLED_init(void) {
  I2C_init();                             // initialize I2C first

  // goto end;
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  for(unsigned i = 0; i < sizeof(OLED_INIT_CMD); i++)
    I2C_write(OLED_INIT_CMD[i]);          // send the command bytes

  I2C_stop();                             // stop transmission
}

// Start sending data
void OLED_data_start(void) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_DAT_MODE);               // set data mode
}

// Start sending command
void OLED_command_start(void) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
}

// OLED send command
void OLED_send_command(uint8_t cmd) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(cmd);                         // send command
  I2C_stop();                             // stop transmission
}

// OLED set cursor position
void OLED_setpos(uint8_t x, uint8_t y) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(OLED_PAGE | y);	              // set page start address
  I2C_write(x & 0x0F);			              // set lower nibble of start column
  I2C_write(OLED_COLUMN_HIGH | (x >> 4)); // set higher nibble of start column
  I2C_stop();                             // stop transmission
}

// OLED fill screen
void OLED_fill(uint8_t p) {
  OLED_setpos(0, 0);                      // set cursor to display start
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_DAT_MODE);               // set data mode
  // for(uint16_t i=128*8; i; i--) I2C_write(p); // send pattern
  for(uint16_t i=OLED_WIDTH*OLED_HEIGHT/8; i; i--) I2C_write(p); // send pattern
  I2C_stop();                             // stop transmission
}

// OLED draw bitmap
void OLED_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t* bmp) {
  for(uint8_t y = y0; y < y1; y++) {
    OLED_setpos(x0, y);
    I2C_start(OLED_ADDR);
    I2C_write(OLED_DAT_MODE);
    for(uint8_t x = x0; x < x1; x++)
      I2C_write(*bmp++);
    I2C_stop();
  }
}

// #include "grotesk-bold-font-16x32.h"
// #include "various-symbols-font-16x32.h"
#include "ubuntu-bold-font-24x32.h"

// // OLED plot a single character
// void OLED_plotChar(char c) {
//   uint8_t i;
//   uint16_t ptr = c - 32;                  // character pointer
//   ptr += ptr << 2;                        // -> ptr = (ch - 32) * 5;
//   I2C_start(OLED_ADDR);                   // start transmission to OLED
//   I2C_write(OLED_DAT_MODE);               // set data mode
//   for(i=5 ; i; i--) I2C_write(OLED_FONT[ptr++]);
//   I2C_write(0x00);                        // write space between characters
//   I2C_stop();                             // stop transmission
// }

// // OLED print string
// void OLED_print(char* str) {
//   while(*str) OLED_plotChar(*str++);
// }

#if !defined(FONT_WC)
#define FONT_WC FONT_W
#endif

void OLED_prints(uint8_t x, uint8_t y, char* str) {
  for(unsigned j = 0; j < ((FONT_H+7)/8); j++) {
    char* s = str;
    OLED_setpos(x, y+j);
    I2C_start(OLED_ADDR);                   // start transmission to OLED
    I2C_write(OLED_DAT_MODE);               // set data mode
    while(*s) {
      uint8_t c = *s++;
      uint16_t ptr = c - 32;                  // character pointer

      // ptr += ptr << 2;                        // -> ptr = (ch - 32) * 5;
      // for(unsigned i = 5; i; i--) I2C_write(OLED_FONT[ptr++]);

      ptr *= FONT_W;
      for(unsigned i = 0; i < FONT_WC; i++) {
        if(j==0) I2C_write(font[ptr++] & 0xFF);
        else if(j==1) I2C_write((font[ptr++]>>8) & 0xFF);
        else if(j==2) I2C_write((font[ptr++]>>16) & 0xFF);
        else I2C_write((font[ptr++]>>24) & 0xFF);
      }
      I2C_write(0x00);                        // write space between characters
    }
    I2C_stop();                             // stop transmission
  }
}

// void OLED_draw_buffer(uint8_t* buffer, size_t buffer_size)
// {
//   // I2C_start(OLED_ADDR);                           // start transmission to OLED
//   // I2C_write(OLED_DAT_MODE);                       // set command mode
//   I2C_writeBuffer(buffer, buffer_size); // send screen buffer using DMA
// }
