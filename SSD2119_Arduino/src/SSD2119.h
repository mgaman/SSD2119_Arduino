/*
  SSD2119 Class
  REV 4

    REV Log:
    1 - Basic functionality
    2 - Arduino 1.6.7 / 1.6.8 compatibility (function prototypes on one line)
    3 - Added Big Font with background and foreground colors
    4 - Added support for Arduino Due and its extended SPI functions https://www.arduino.cc/en/Reference/DueExtendedSPI


  Developed by TheFax using:
  -SSD2119 datasheet, by SOLOMON SYSTECH (Rev 1.4 - June 2009)
  -code wrote by TheFax
  -code scraps by CooCox (Copyright (c)2011, CooCox)
  -code scraps wrote by Steven Prickett (Kentec 320x240x16 BoosterPack)

  ---->  NOT FOR COMMERCIAL USE  <----

  Date:              November 2015
  Hardware target:   Arduino Mega -> 4-wire SPI bus -> SSD2119
  Developer:         TheFax
  This class contains:
  SSD2119_init()                                      --> Setup of Color LCD display based on SSD2119
  DrawOnePixel(X,Y,Color)                      --> Draw only one pixel
  This class uses the 4 wire SPI bus of the LCD display.
  Connections:
  DISPLAY    |    ARDUINO/GENUINO Mega 2560   |  Notes
  Reset      |    47                          | (LCD reset, low active)
  SCL        |    52    (MANDATORY)           | (SPI Clock)
  DC         |    49                          | (Data/Command)   RS on shield
  CS         |    48                          | (Circuit Select, low active)
  SDI        |    51    (MANDATORY)           | (SPI transfer, from uP to LCD)  // SDA on shield

  DISPLAY    |    ARDUINO/GENUINO DUE         |  Notes
  Reset      |    47                          | (LCD reset, low active)
  SCL        |    on SPI connector            | (SPI Clock)
  DC         |    49                          | (Data/Command)  RS on shield
  CS         |    10                          | (Circuit Select, low active)  // for extended SPI on Die CS MUST be 4,10 or 52
  SDI        |    on SPI connector            | (SPI transfer, from uP to LCD) // SDA on shield
*/

//Include the SPI library:
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>

#define RGB565 565
#define RGB888 888
#define RGB666 666
//Operative logic states
#define SSD2119_DC_COMMAND                      LOW
#define SSD2119_DC_DATA                         HIGH
#define SSD2119_NORMAL_REG                      HIGH
#define SSD2119_GRAM_REG                        LOW

//Operative logic states
#define SSD2119_DC_COMMAND                      LOW
#define SSD2119_DC_DATA                         HIGH
#define SSD2119_NORMAL_REG                      HIGH
#define SSD2119_GRAM_REG                        LOW

//SSD2119 datasheet
#define SSD2119_DEVICE_CODE_READ_REG            0x00
#define SSD2119_OSC_START_REG                   0x00
#define SSD2119_OUTPUT_CTRL_REG                 0x01
#define SSD2119_LCD_DRIVE_AC_CTRL_REG           0x02
#define SSD2119_PWR_CTRL_1_REG                  0x03
#define SSD2119_DISPLAY_CTRL_REG                0x07
#define SSD2119_FRAME_CYCLE_CTRL_REG            0x0B
#define SSD2119_PWR_CTRL_2_REG                  0x0C
#define SSD2119_PWR_CTRL_3_REG                  0x0D
#define SSD2119_PWR_CTRL_4_REG                  0x0E
#define SSD2119_GATE_SCAN_START_REG             0x0F
#define SSD2119_SLEEP_MODE_1_REG                0x10
#define SSD2119_ENTRY_MODE_REG                  0x11
#define SSD2119_GEN_IF_CTRL_REG                 0x15
#define SSD2119_PWR_CTRL_5_REG                  0x1E
#define SSD2119_RAM_DATA_REG                    0x22
#define SSD2119_FRAME_FREQ_REG                  0x25
#define SSD2119_VCOM_OTP_1_REG                  0x28
#define SSD2119_VCOM_OTP_2_REG                  0x29
#define SSD2119_GAMMA_CTRL_1_REG                0x30
#define SSD2119_GAMMA_CTRL_2_REG                0x31
#define SSD2119_GAMMA_CTRL_3_REG                0x32
#define SSD2119_GAMMA_CTRL_4_REG                0x33
#define SSD2119_GAMMA_CTRL_5_REG                0x34
#define SSD2119_GAMMA_CTRL_6_REG                0x35
#define SSD2119_GAMMA_CTRL_7_REG                0x36
#define SSD2119_GAMMA_CTRL_8_REG                0x37
#define SSD2119_GAMMA_CTRL_9_REG                0x3A
#define SSD2119_GAMMA_CTRL_10_REG               0x3B
#define SSD2119_V_SCROLL_CTRL_1_REG             0x41
#define SSD2119_V_SCROLL_CTRL_2_REG             0x42
#define SSD2119_V_RAM_POS_REG                   0x44
#define SSD2119_H_RAM_START_REG                 0x45
#define SSD2119_H_RAM_END_REG                   0x46
#define SSD2119_SCREEN_1_START_POS_REG          0x48
#define SSD2119_SCREEN_1_END_POS_REG            0x49
#define SSD2119_SCREEN_2_START_POS_REG          0x4A
#define SSD2119_SCREEN_2_END_POS_REG            0x4B
#define SSD2119_X_RAM_ADDR_REG                  0x4E
#define SSD2119_Y_RAM_ADDR_REG                  0x4F
#define SSD2119_OUTPUTCTL_REV                   0x30EF
#define SSD2119_OUTPUTCTL_SET(ulConfig)         (ulConfig | 0x30EF)
#define SSD2119_OUTPUT_ORIGIN_UPLEFT            0x4200
#define SSD2119_OUTPUT_ORIGIN_DOWNLEFT          0x4000
#define SSD2119_OUTPUT_ORIGIN_UPRIGHT           0x0200
#define SSD2119_OUTPUT_ORIGIN_DOWNRIGHT         0x0000
#define SSD2119_LCD_WAVEFORM_REV                0x30EF
#define SSD2119_3DIV_SCR_MODE                   0x1000
#define SSD2119_WSYNC_MODE1                     0x0800
#define SSD2119_WSYNC_MODE2                     0x0900
#define SSD2119_PC1_8COLOR_DTC_SET(ucConfig)    ((0x0f & ucConfig) << 12)
#define SSD2119_PC1_8COLOR_BT_SET(ucConfig)     ((0x07 & ucConfig) << 9)
#define SSD2119_PC1_26KCOLOR_DC_SET(ucConfig)   ((0x0F & ucConfig) << 4)
#define SSD2119_PC1_26KCOLOR_AP_SET(ucConfig)   ((0x0E & ucConfig) << 0)
#define SSD2119_DISPLAY_MODE_DEFAULT            0x0033
#define SSD2119_DISPLAY_MODE_SET(x)             ((x) | 0x0033)
#define SSD2119_SCREEN_TWODIV_ENABLE            0x0100
#define SSD2119_SCREEN_TWODIV_DATA18            0x0200
#define SSD2119_SCREEN_TWODIV_DATA9             0x0600
#define SSD2119_DISPLAY_8COLOR                  0x0008
#define SSD2119_FCC_NO_SET(ucConfig)            ((ucConfig & 0x3) << 14)
#define SSD2119_FCC_SDT_SET(ucConfig)           ((ucConfig & 0x3) << 12)
#define SSD2119_FCC_EQ_SET(ucConfig)            ((ucConfig & 0x7) << 8)
#define SSD2119_FCC_DIV_SET(ucConfig)           ((ucConfig & 0x3) << 6)
#define SSD2119_FCC_DIV_ENABLE                  0x0020
#define SSD2119_FCC_RTN_ENABLE                  0x0010
#define SSD2119_FCC_RTN_SET(ucConfig)           ((ucConfig & 0xF) << 0)
#define SSD2119_OUTPUT_VOLTAGE_SET(ulConfig)    (ulConfig & 0x7)
#define SSD2119_OUTPUT_VOLTAGE_5_1V             0x0000
#define SSD2119_OUTPUT_VOLTAGE_5_3V             0x0001
#define SSD2119_OUTPUT_VOLTAGE_5_5V             0x0002
#define SSD2119_OUTPUT_VOLTAGE_5_7V             0x0003
#define SSD2119_OUTPUT_VOLTAGE_5_9V             0x0004
#define SSD2119_OUTPUT_VOLTAGE_6_1V             0x0005
#define SSD2119_VLCD63_SET(ulConfig)            (ulConfig)
#define SSD2119_VREF_TIMES_1_78                 0x0003
#define SSD2119_VREF_TIMES_1_85                 0x0004
#define SSD2119_VREF_TIMES_1_93                 0x0005
#define SSD2119_VREF_TIMES_2_02                 0x0006
#define SSD2119_VREF_TIMES_2_09                 0x0007
#define SSD2119_VREF_TIMES_2_165                0x0008
#define SSD2119_VREF_TIMES_2_245                0x0009
#define SSD2119_VREF_TIMES_2_335                0x000A
#define SSD2119_VREF_TIMES_2_40                 0x000B
#define SSD2119_VREF_TIMES_2_45                 0x000C
#define SSD2119_VREF_TIMES_2_57                 0x000D
#define SSD2119_VREF_TIMES_2_645                0x000E
#define SSD2119_VREF_TIMES_2_725                0x000F
#define SSD2119_VREF_TIMES_2_81                 0x0000
#define SSD2119_VREF_TIMES_2_90                 0x0001
#define SSD2119_VREF_TIMES_3_00                 0x0002
#define SSD2119_VREF_VOLTAGE                    2.0
#define SSD2119_VCOM_SET(ulConfig)              (ulConfig)
#define SSD2119_VLCD63_TIMES_0_60               0x2000
#define SSD2119_VLCD63_TIMES_0_63               0x2100
#define SSD2119_VLCD63_TIMES_0_66               0x2200
#define SSD2119_VLCD63_TIMES_0_69               0x2300
#define SSD2119_VLCD63_TIMES_0_72               0x2400
#define SSD2119_VLCD63_TIMES_0_75               0x2500
#define SSD2119_VLCD63_TIMES_0_78               0x2600
#define SSD2119_VLCD63_TIMES_0_81               0x2700
#define SSD2119_VLCD63_TIMES_0_84               0x2800
#define SSD2119_VLCD63_TIMES_0_87               0x2900
#define SSD2119_VLCD63_TIMES_0_90               0x2A00
#define SSD2119_VLCD63_TIMES_0_93               0x2B00
#define SSD2119_VLCD63_TIMES_0_96               0x2C00
#define SSD2119_VLCD63_TIMES_0_99               0x2D00
#define SSD2119_VLCD63_TIMES_1_02               0x2E00
#define SSD2119_VLCD63_TIMES_1_05               0x3000
#define SSD2119_VLCD63_TIMES_1_08               0x3100
#define SSD2119_VLCD63_TIMES_1_11               0x3200
#define SSD2119_VLCD63_TIMES_1_14               0x3300
#define SSD2119_VLCD63_TIMES_1_17               0x3400
#define SSD2119_VLCD63_TIMES_1_20               0x3500
#define SSD2119_VLCD63_TIMES_1_23               0x3600
#define SSD2119_VLCD63_TIMES_EXT                0x2F00
#define SSD2119_SCN_STAART_GATE_SET(ulConfig)   (ulConfig & 0x1FF)
#define SSD2119_SLEEP_MODE_ENTER                0x0001
#define SSD2119_SLEEP_MODE_EXIT                 0x0000
#define SSD2119_ENTRY_MODE_POR              0x6230 // POR  65K
#define SSD2119_ENTRY_MODE_666              0x48B0  //  262k colors in original code
#define SSD2119_ENTRY_MODE_SET(x)               ((SSD2119_ENTRY_MODE_DEFAULT & 0xFF00) | (x))
#define SSD2119_EM_VSYNC_INPUT                  0x8000
#define SSD2119_EM_65KCOLOR                     0x6000
#define SSD2119_EM_262KCOLOR                    0x4000
#define SSD2119_EM_HVSYNC_OFF                   0x0800
#define SSD2119_EM_RAM_RGB                      0x0400
#define SSD2119_EM_RAM_SPI                      0x0000
#define SSD2119_EM_DM_CHANGE_IMMEDIATELY        0x0200
#define SSD2119_EM_DM_CHANNGE_START             0x0000
#define SSD2119_EM_CLOCK_OSC                    0x0100
#define SSD2119_EM_CLOCK_DOTCLK                 0x0000
#define SSD2119_EM_262K_TY_A                    0x0000
#define SSD2119_EM_262K_TY_B                    0x0040
#define SSD2119_EM_262K_TY_C                    0x0080
#define SSD2119_EM_ADDR_INC                     0x0010
#define SSD2119_EM_ADDR_DEC                     0x0000
#define SSD2119_EM_DIRECTION_H                  0x0000
#define SSD2119_EM_DIRECTION_V                  0x0008
#define SSD2119_GIC_LATCH_POS                   0x0000
#define SSD2119_GIC_LATCH_NEG                   0x0008
#define SSD2119_GIC_DEN_H                       0x0000
#define SSD2119_GIC_DEN_L                       0x0004
#define SSD2119_GIC_VHS_H                       0x0002
#define SSD2119_GIC_VHS_L                       0x0000
#define SSD2119_GIC_VVS_H                       0x0001
#define SSD2119_GIC_VVS_L                       0x0000
#define SSD2119_VCOMH_SET(ulConfig)             (ulConfig | 0xFF)
#define SSD2119_VCOMH_OTP                       0x0000
#define SSD2119_VLCD_TIMES_0_36                 0x0080
#define SSD2119_VLCD_TIMES_0_37                 0x0081
#define SSD2119_VLCD_TIMES_0_38                 0x0082
#define SSD2119_VLCD_TIMES_0_39                 0x0083
#define SSD2119_VLCD_TIMES_0_40                 0x0084
#define SSD2119_VLCD_TIMES_0_41                 0x0085
#define SSD2119_VLCD_TIMES_0_42                 0x0086
#define SSD2119_VLCD_TIMES_0_43                 0x0087
#define SSD2119_VLCD_TIMES_0_44                 0x0088
#define SSD2119_VLCD_TIMES_0_45                 0x0089
#define SSD2119_VLCD_TIMES_0_46                 0x008A
#define SSD2119_VLCD_TIMES_0_47                 0x008B
#define SSD2119_VLCD_TIMES_0_48                 0x008C
#define SSD2119_VLCD_TIMES_0_49                 0x008D
#define SSD2119_VLCD_TIMES_0_50                 0x008E
#define SSD2119_VLCD_TIMES_0_51                 0x008F
#define SSD2119_VLCD_TIMES_0_52                 0x0090
#define SSD2119_VLCD_TIMES_0_53                 0x0091
#define SSD2119_VLCD_TIMES_0_54                 0x0092
#define SSD2119_VLCD_TIMES_0_55                 0x0093
#define SSD2119_VLCD_TIMES_0_56                 0x0094
#define SSD2119_VLCD_TIMES_0_57                 0x0095
#define SSD2119_VLCD_TIMES_0_58                 0x0096
#define SSD2119_VLCD_TIMES_0_59                 0x0097
#define SSD2119_VLCD_TIMES_0_60                 0x0098
#define SSD2119_VLCD_TIMES_0_61                 0x0099
#define SSD2119_VLCD_TIMES_0_62                 0x009A
#define SSD2119_VLCD_TIMES_0_63                 0x009B
#define SSD2119_VLCD_TIMES_0_64                 0x009C
#define SSD2119_VLCD_TIMES_0_65                 0x009D
#define SSD2119_VLCD_TIMES_0_66                 0x009E
#define SSD2119_VLCD_TIMES_0_67                 0x009F
#define SSD2119_VLCD_TIMES_0_68                 0x00A0
#define SSD2119_VLCD_TIMES_0_69                 0x00A1
#define SSD2119_VLCD_TIMES_0_70                 0x00A2
#define SSD2119_VLCD_TIMES_0_71                 0x00A3
#define SSD2119_VLCD_TIMES_0_72                 0x00A4
#define SSD2119_VLCD_TIMES_0_73                 0x00A5
#define SSD2119_VLCD_TIMES_0_74                 0x00A6
#define SSD2119_VLCD_TIMES_0_75                 0x00A7
#define SSD2119_VLCD_TIMES_0_76                 0x00A8
#define SSD2119_VLCD_TIMES_0_77                 0x00A9
#define SSD2119_VLCD_TIMES_0_78                 0x00AA
#define SSD2119_VLCD_TIMES_0_79                 0x00AB
#define SSD2119_VLCD_TIMES_0_80                 0x00AC
#define SSD2119_VLCD_TIMES_0_81                 0x00AD
#define SSD2119_VLCD_TIMES_0_82                 0x00AE
#define SSD2119_VLCD_TIMES_0_83                 0x00AF
#define SSD2119_VLCD_TIMES_0_84                 0x00B0
#define SSD2119_VLCD_TIMES_0_85                 0x00B1
#define SSD2119_VLCD_TIMES_0_86                 0x00B2
#define SSD2119_VLCD_TIMES_0_87                 0x00B3
#define SSD2119_VLCD_TIMES_0_88                 0x00B4
#define SSD2119_VLCD_TIMES_0_89                 0x00B5
#define SSD2119_VLCD_TIMES_0_90                 0x00B6
#define SSD2119_VLCD_TIMES_0_91                 0x00B7
#define SSD2119_VLCD_TIMES_0_92                 0x00B8
#define SSD2119_VLCD_TIMES_0_93                 0x00B9
#define SSD2119_VLCD_TIMES_0_94                 0x00BA
#define SSD2119_VLCD_TIMES_0_95                 0x00BB
#define SSD2119_VLCD_TIMES_0_96                 0x00BC
#define SSD2119_VLCD_TIMES_0_97                 0x00BD
#define SSD2119_VLCD_TIMES_0_98                 0x00BE
#define SSD2119_VLCD_TIMES_0_99                 0x00BF
#define SSD2119_WRITE_DATA_M                    0x3FFFF
#define SSD2119_WRITE_DATA_S                    0
#define SSD2119_READ_DATA_M                     0x3FFFF
#define SSD2119_READ_DATA_S                     0
#define SSD2119_FRAME_FREQ_SET(ulConfig)        (ulConfig)
#define SSD2119_FRAME_FREQ_50HZ                 0x0000
#define SSD2119_FRAME_FREQ_55HZ                 0x2000
#define SSD2119_FRAME_FREQ_60HZ                 0x5000
#define SSD2119_FRAME_FREQ_65HZ                 0x8000
#define SSD2119_FRAME_FREQ_70HZ                 0xA000
#define SSD2119_FRAME_FREQ_75HZ                 0xC000
#define SSD2119_FRAME_FREQ_80HZ                 0xE000
#define SSD2119_LCD_VERTICAL_MAX                240
#define SSD2119_LCD_HORIZONTAL_MAX              320
#define HORIZ_DIRECTION                         0x30
#define VERT_DIRECTION                          0x38
#define MAPPED_X(x, y)                          (x)
#define MAPPED_Y(x, y)                          (y)


//class SSD2119 : public Adafruit_SPITFT
class SSD2119 :public  Adafruit_GFX
{
  public:
    SSD2119(int RS, int CS, int RST);
    void begin(uint32_t freq = 0);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void fillScreen(uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void setRotation(uint8_t r);
    // Transaction API not used by GFX
//    void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
private:
    int _p_rs;
    int _p_cs;
    int _p_rst;
  //  int _p_scl;
   // int _p_sda;
    void ClearDisplay(unsigned long ulBackgroundColor);
    void SSD2119WriteCmd(unsigned char ucCmd);
    void SSD2119WriteData(unsigned long ulData);
    void SSD2119WritePixelData(unsigned long ulData);
    void SSD2119Write(unsigned char ucDC, unsigned long ulInstruction, unsigned char ucRegType);
    void SPI_init();
    void DrawOnePixel(unsigned short usX, unsigned short usY, unsigned long ulColor);
    int _palette;
    uint16_t currentR11H;
    //   void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
 //   uint32_t color888(uint16_t rgb565);
    uint32_t color666(uint16_t rgb565);
    uint16_t last565;
//    uint32_t last888;
    uint32_t last666;
};
