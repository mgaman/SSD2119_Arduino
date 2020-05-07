# SSD2119 Driver for Adafruit GFX
## History
This is the result of enforced idleness during the Covid-19 lockdown. I have a 3.5" TFT display never used because I didn't know what IC it was based on no software to drive it. 
Eventually  <a href="https://www.alibaba.com/product-detail/3-5-TFT-LCD-Module-Display_1976721279.html">here</a> I discovered that it was an SSD2119 but that didn't help as I couldn't find an Arduino library.
Later I came upon <a href="https://github.com/TheFax/SSD2119-library">this project</a> which contains all the source needed to drive the TFT but it isn't really a library, rather a flat Arduino sketch.
## How I did it
Basically all I did was re-package the C style code as a C++ class.<p>
### What I added
Adafuit GFX has all the drawing primitives I need so seemed the obvious partner. All one has to do is provide at least one method, drawPixel. All the rest falls into place.
While that provides basic functionality it can be painfully slow. There are other GFX methods that may be overridden for better performance. So far I have implemented drawFastHLine which also helped me to implement fillScreen, fillRect and drawRect.
### What I took away
All drawing primitives and font information. These are now provided by GFX
### What I changed
Only color mode RGB565 is supported which is what GFX uses. As we are using the 4-wire SPI interface the hardware actually only runs the RGB666 color mode. Sadly this means that every pixel color needs to be translated which impacts performance.<p>
It is no longer necessary to specify the target architecture which is now recognised at compile time.<p>
For the Due platform extended SPI is compiled even though Arduino forums advise against it. I measured a small performance benefit.
## TBD
Implement GFX methods setRotation and drawFastVLine.<p>
This is still a work in progress and I have yet to develop better optimisation.<p>
## Other Devices
My board is described as having 3 other devices, SD card, TouchPanel and DataFlash. The SD card can be used via the SD library and the Touch Panel via the XPT2046 Touchscreen library. I could not find a flash device although I did find an unpopulated SMD place.
## Pin Confusion
One of the hardest problems I had was understanding the labels on the pins on the TFT board, a source of much confusion.<p>
RS   a.k.a. Register Select (Data/Command) a.k.a. DC<p>
SCL  a.k.a. SPI clock a.k.a. SCLK.  NOTE: This does NOT refer to I2C SCL<p>
SDA  a.k.a  SPI Master to Slave a.k.a. MOSI.  NOTE: This does NOT refer to I2C SDA<p>
If using just the TFT, use the SCL and SDA pins. If also using the SD and TouchPanel then SCLK, MOSI and MISO must be connected, in addition to SCL and SDA. Also make sure that all chip select pins are connected and unused devices deseleted.  
## This Repository Layout
I use Eclipse with Arduino plugin as my IDE. While convenient it has some incompatabilities for developing an Arduino library
complete with examples.
- Only 1 source file containing setup() per project. There is no mechanism for excluding files from the build process.
I get around that by renaming all examples files as XXX.txt except 1 (which gets compiled).
- The source file type **pde** or **ino** is not honored. Only **cpp** works.
- The Arduino plugin does not have any way to set include file path, so examples cannot use the **<SSD2119.h>** format.
I am forced to use the relative **"../../src/SSD2119.h"** format
