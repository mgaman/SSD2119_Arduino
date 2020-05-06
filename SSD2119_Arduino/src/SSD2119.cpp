#include <Arduino.h>
#include "SSD2119.h"
#include <SPI.h>
#include <Adafruit_GFX.h>

SSD2119::SSD2119(int RS, int CS, int RST) : Adafruit_GFX(SSD2119_LCD_HORIZONTAL_MAX  , SSD2119_LCD_VERTICAL_MAX  ) {
  _p_rs = RS;
  _p_cs = CS;
  _p_rst = RST;
  _palette = RGB565;
}

void  SSD2119::SSD2119Write(unsigned char ucDC, unsigned long ulInstruction, unsigned char ucRegType) {
  #if defined(__AVR__)
  digitalWrite(_p_cs, LOW);   //Too slow. FIX IT!
  digitalWrite(_p_rs, ucDC);  //Too slow. FIX IT!
  if (ucDC == SSD2119_DC_DATA)
  {
    //Send DATA
    if (ucRegType == SSD2119_NORMAL_REG)
    {
      //Send DATA for NORMAL_REGISTER
      SPI.transfer((ulInstruction >> 8) & 0xFF);
      SPI.transfer(ulInstruction & 0xFF);
    }
    else
    {
      //Send DATA for GRAPHICS_REGISTER
      //6 bits per color
      //SPI.transfer( (ulInstruction >> 12) & 0xFC);
      //SPI.transfer( (ulInstruction >> 6) & 0xFC);
      //SPI.transfer( (ulInstruction & 0xFC);
      //8 bits per color
      SPI.transfer((ulInstruction >> 16) & 0xFF);  //Red
      SPI.transfer((ulInstruction >> 8) & 0xFF);   //Green
      SPI.transfer(ulInstruction & 0xFF);          //Blue
    }
  }
  else
  {
    //Send INSTRUCTION
    SPI.transfer(ulInstruction & 0xFF);
  }
  digitalWrite(_p_cs, HIGH);  //Too slow. FIX IT!
  #endif

  #if defined(_LIB_SAM_)   // Due
 digitalWrite(_p_rs, ucDC);  //Too slow. FIX IT!
  if (ucDC == SSD2119_DC_DATA)
  {
    //Send DATA
    if (ucRegType == SSD2119_NORMAL_REG)
    {
      //Send DATA for NORMAL_REGISTER
      SPI.transfer(_p_cs,(ulInstruction >> 8) & 0xFF);
      SPI.transfer(_p_cs,ulInstruction & 0xFF);
    }
    else
    {
      //Send DATA for GRAPHICS_REGISTER
      //6 bits per color
      //SPI.transfer( (ulInstruction >> 12) & 0xFC);
      //SPI.transfer( (ulInstruction >> 6) & 0xFC);
      //SPI.transfer( (ulInstruction & 0xFC);
      //8 bits per color
      SPI.transfer(_p_cs,(ulInstruction >> 16) & 0xFF);  //Red
      SPI.transfer(_p_cs,(ulInstruction >> 8) & 0xFF);   //Green
      SPI.transfer(_p_cs,ulInstruction & 0xFF);          //Blue
    }
  }
  else
  {
    //Send INSTRUCTION
    SPI.transfer(_p_cs,ulInstruction & 0xFF);
  }
  #endif
}

void SSD2119::SSD2119WriteCmd(unsigned char ucCmd) {
  SSD2119Write(SSD2119_DC_COMMAND, ucCmd, SSD2119_NORMAL_REG);
}

void SSD2119::SSD2119WriteData(unsigned long ulData) {
  SSD2119Write(SSD2119_DC_DATA, ulData, SSD2119_NORMAL_REG);
}

void SSD2119::SSD2119WritePixelData(unsigned long ulData) {
  SSD2119Write(SSD2119_DC_DATA, ulData, SSD2119_GRAM_REG);
  //SSD2119Write(SSD2119_DC_DATA, ulData, SSD2119_NORMAL_REG);
}

void SSD2119::ClearDisplay(unsigned long ulBackgroundColor){
  for (unsigned long ulCount = 0; ulCount < ((SSD2119_LCD_HORIZONTAL_MAX) * long(SSD2119_LCD_VERTICAL_MAX)); ulCount++)   {
    SSD2119WritePixelData(ulBackgroundColor);
  }
}

void SSD2119::SPI_init() {
  //Setup of SPI interface on Arduino board
 // Serial.print("palette ");Serial.println(_palette);
  //Setup pin direction
  pinMode(_p_rs, OUTPUT);
  pinMode(_p_rst, OUTPUT);

#if defined(__AVR__)   // AVR code explicitly toiggles chip select so set iy up
    pinMode(_p_cs, OUTPUT);

    //Disable Display's Circuit Select.
    digitalWrite(_p_cs, HIGH);
#endif
    //SPI speed and other SPI settings common to voth
    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE2));
    //Enable SPI streaming
#if defined(__AVR__)
    SPI.begin();
#endif
#if defined(_LIB_SAM_)   // Due extended SPI has chip select pin as a parameter in all calls
    //Enable SPI streaming
    SPI.begin(_p_cs);
#endif
}

void SSD2119::initLCD() {
  //Hardware SPI initialization
  SPI_init();

  //Display reset
  digitalWrite(_p_rst, LOW);
  delay(1); //[ms]
  digitalWrite(_p_rst, HIGH);
  delay(10); //[ms]

  // Enter sleep mode
  SSD2119WriteCmd(SSD2119_SLEEP_MODE_1_REG);
  SSD2119WriteData(SSD2119_SLEEP_MODE_ENTER);

  // Set initial power parameters
  SSD2119WriteCmd(SSD2119_PWR_CTRL_5_REG);
  SSD2119WriteData(SSD2119_VCOMH_SET(SSD2119_VLCD_TIMES_0_86));
  SSD2119WriteCmd(SSD2119_VCOM_OTP_1_REG);
  SSD2119WriteData(0x0006);

  // Start the oscillator
  SSD2119WriteCmd(SSD2119_OSC_START_REG);
  SSD2119WriteData(0x0001);

  // Set pixel format and basic display orientation (scanning direction)
  SSD2119WriteCmd(SSD2119_OUTPUT_CTRL_REG);
  SSD2119WriteData(SSD2119_OUTPUTCTL_SET(SSD2119_OUTPUT_ORIGIN_DOWNRIGHT));
  SSD2119WriteCmd(SSD2119_LCD_DRIVE_AC_CTRL_REG);
  SSD2119WriteData(0x0600);

  // Exit sleep mode.
  SSD2119WriteCmd(SSD2119_SLEEP_MODE_1_REG);
  SSD2119WriteData(SSD2119_SLEEP_MODE_EXIT);

  // Delay 30mS requested by datasheet
  delay(100); //[ms]

  // Configure pixel color format and MCU interface parameters.
  SSD2119WriteCmd(SSD2119_ENTRY_MODE_REG);
  if (_palette = RGB888) {
    SSD2119WriteData(SSD2119_ENTRY_MODE_888);
    currentR11H = SSD2119_ENTRY_MODE_888;
  }
  else {
    SSD2119WriteData(SSD2119_ENTRY_MODE_POR);
    currentR11H = SSD2119_ENTRY_MODE_POR;
  }
  //SSD2119WriteData(0b0110001000110000);  //datasheet
  //SSD2119WriteData(0b0110001000110000);  //datasheet

  // Enable the display.
  SSD2119WriteCmd(SSD2119_DISPLAY_CTRL_REG);
  SSD2119WriteData(0x0033);

  // Set VCIX2 voltage to 6.1V.
  SSD2119WriteCmd(SSD2119_PWR_CTRL_2_REG);
  SSD2119WriteData(SSD2119_OUTPUT_VOLTAGE_SET(SSD2119_OUTPUT_VOLTAGE_6_1V));

  // Configure gamma correction.
  /*
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_1_REG);
    SSD2119WriteData(0x0000);
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_2_REG);
    SSD2119WriteData(0x0303);
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_3_REG);
    SSD2119WriteData(0x0407);
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_4_REG);
    SSD2119WriteData(0x0301);
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_5_REG);
    SSD2119WriteData(0x0301);
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_6_REG);
    SSD2119WriteData(0x0403);
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_7_REG);
    SSD2119WriteData(0x0707);
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_8_REG);
    SSD2119WriteData(0x0400);
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_9_REG);
    SSD2119WriteData(0x0a00);
    SSD2119WriteCmd(SSD2119_GAMMA_CTRL_10_REG);
    SSD2119WriteData(0x1000);
  */
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_1_REG);
  SSD2119WriteData(0x0000);
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_2_REG);
  SSD2119WriteData(0x0101);
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_3_REG);
  SSD2119WriteData(0x0100);
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_4_REG);
  SSD2119WriteData(0x0305);
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_5_REG);
  SSD2119WriteData(0x0707);
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_6_REG);
  SSD2119WriteData(0x0305);
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_7_REG);
  SSD2119WriteData(0x0707);
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_8_REG);
  SSD2119WriteData(0x0201);
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_9_REG);
  SSD2119WriteData(0x1200);
  SSD2119WriteCmd(SSD2119_GAMMA_CTRL_10_REG);
  SSD2119WriteData(0x0900);

  // Configure Vlcd63 and VCOMl.
  SSD2119WriteCmd(SSD2119_PWR_CTRL_3_REG);
  SSD2119WriteData(SSD2119_VLCD63_SET(SSD2119_VREF_TIMES_2_335));
  SSD2119WriteCmd(SSD2119_PWR_CTRL_4_REG);
  SSD2119WriteData(SSD2119_VCOM_SET(SSD2119_VLCD63_TIMES_1_02));

  // Set the display size and ensure that the GRAM window is set to allow
  // access to the full display buffer.
  SSD2119WriteCmd(SSD2119_V_RAM_POS_REG);
  SSD2119WriteData((SSD2119_LCD_VERTICAL_MAX - 1) << 8);
  SSD2119WriteCmd(SSD2119_H_RAM_START_REG);
  SSD2119WriteData(0x0000);
  SSD2119WriteCmd(SSD2119_H_RAM_END_REG);
  SSD2119WriteData(SSD2119_LCD_HORIZONTAL_MAX - 1);
  SSD2119WriteCmd(SSD2119_X_RAM_ADDR_REG);
  SSD2119WriteData(0x00);
  SSD2119WriteCmd(SSD2119_Y_RAM_ADDR_REG);
  SSD2119WriteData(0x00);

  SSD2119WriteCmd(SSD2119_RAM_DATA_REG);
  //delay(2000);  //Used during debug


  //Clear the contents of the display buffer...

  //...using random color
  //for(unsigned long ulCount = 0; ulCount < ((SSD2119_LCD_HORIZONTAL_MAX) * long(SSD2119_LCD_VERTICAL_MAX)); ulCount++)   {
  //  SSD2119WritePixelData(random()*0xffffff);  //Random color!
  //}


  //...using a fix color
  //for (unsigned long ulCount = 0; ulCount < ((SSD2119_LCD_HORIZONTAL_MAX) * long(SSD2119_LCD_VERTICAL_MAX)); ulCount++)   {
  //  ClearDisplay(0x000000);  //Black!
  //}

  //...using function
  ClearDisplay(0x000000);
}

void SSD2119::DrawOnePixel(unsigned short usX, unsigned short usY, unsigned long ulColor) {
  // Set the X address of the display cursor.
  SSD2119WriteCmd(SSD2119_X_RAM_ADDR_REG);
  SSD2119WriteData(MAPPED_X(usX, usY));

  // Set the Y address of the display cursor.
  SSD2119WriteCmd(SSD2119_Y_RAM_ADDR_REG);
  SSD2119WriteData(MAPPED_Y(usX, usY));

  // Write the pixel value.
  SSD2119WriteCmd(SSD2119_RAM_DATA_REG);
  SSD2119WritePixelData(ulColor);
}




  /*
  while (usStartX <= usEndX)
  {
    DrawOnePixel(usStartX, usY, ulColor);
    usStartX++;
  }*/



uint16_t last565 = 123;
uint32_t last888;
bool _first = true;
/*
  https://www.lucasgaland.com/24-bit-16-bit-color-converter-tool-for-embedded-lcd-guis/
*/
uint32_t SSD2119::color888(uint16_t rgb565)
{
  if (_first) {
    _first = false;
    last565 = rgb565 + 1;
  }
  if (rgb565 != last565){
    last565 = rgb565;
    uint32_t byte_r = (rgb565 & 0xF800) >> 11;
    uint32_t byte_g = (rgb565 & 0x07E0) >> 5;
    uint32_t byte_b = rgb565 & 0x1F;

//    Serial.println("------");
 //   Serial.println(byte_r,HEX);
 //   Serial.println(byte_g,HEX);
 //   Serial.println(byte_b,HEX);
    byte_r = (byte_r * 527 + 23) >> 6;
//    Serial.println(byte_r,HEX);
    byte_g = (byte_g * 259 + 33) >> 6;
 //   Serial.println(byte_g,HEX);
    byte_b = (byte_b * 527 + 23) >> 6;
 //   Serial.println(byte_b,HEX);

    last888 = (byte_r << 16);
   last888 = last888 | (byte_g << 8);
    last888 = last888 | (byte_b);
  }
  return last888;
}

void SSD2119::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // convert adafruit 565 RGB to 888
  if (_palette == RGB888)
    DrawOnePixel(x,y,color888(color));
  else
    DrawOnePixel(x,y,color);
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t SSD2119::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 2) | (b >> 3);
}

void SSD2119::setRotation(uint8_t r) {
  uint16_t r11h = currentR11H & 0xffc7; // mask out bits 3-5
  // note r note defined anywhere in adafruit documents
  switch (r) {
    case 0: // portrait cable on left
        r11h |= 0b000000;
      break;
    case 1:// portrait cable on right
        r11h |= 0b010000;
      break;
    case 2: // portrait cable at bottom
        r11h |= 0b100000;
      break;
    case 3: // landscape cable at top POR value
      r11h |= 0x110000;
      break;
  }
  SSD2119WriteCmd(SSD2119_ENTRY_MODE_REG);
  SSD2119WriteData(r11h);
  currentR11H = r11h;
}

void SSD2119::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    uint16_t r11h = currentR11H & 0xffc7; // mask out bits 3-5
    r11h |= 0b110000; // ID 11 AM 0
    SSD2119WriteCmd(SSD2119_ENTRY_MODE_REG);
    SSD2119WriteData(r11h);
    currentR11H = r11h;
    drawPixel(x,y,color); // set cursor position & first pixel
    for (int i=w-1;i>=0;i--) {
      // Write the remaining pixels
      SSD2119WriteCmd(SSD2119_RAM_DATA_REG);
      if (_palette == RGB888)
        SSD2119WritePixelData(color888(color));
      else
        SSD2119WritePixelData(color);
    }
 }

#if 0
  void SSD2119::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    uint16_t r11h = currentR11H & 0xffc7; // mask out bits 3-5
    r11h |= 0b11100; // ID 11 AM 1
    SSD2119WriteCmd(SSD2119_ENTRY_MODE_REG);
    SSD2119WriteData(r11h);
    currentR11H = r11h;
    drawPixel(x,y,color); // set cursor position & first pixel
    for (int i=h-1;h>=0;i--) {
      // Write the remaining pixels
      SSD2119WriteCmd(SSD2119_RAM_DATA_REG);
  //    if (_palette == RGB888)
    //    SSD2119WritePixelData(color888(color));
      //else
        SSD2119WritePixelData(color);
    }
}
#endif

void SSD2119::fillScreen(uint16_t color) {
  for (int y=0; y<SSD2119_LCD_VERTICAL_MAX; y++)
    drawFastHLine(0,y,SSD2119_LCD_HORIZONTAL_MAX,color);
}

void SSD2119::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  for (int16_t i = 0; i<h; i++)
   drawFastHLine(x,y+i,w,color);
}

void SSD2119::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  // draw each line in turn
  drawFastHLine(x,y,w,color); // top
  drawFastHLine(x,y+h,w,color); // bottom
  for (int j = 0; j<h; j++) { // left & right
    drawPixel(x,y+j,color);
    drawPixel(x+h,y+j,color);
  }
}
