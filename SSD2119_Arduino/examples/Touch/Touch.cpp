/*
 * TouchTest.cpp
 *
 *  Created on: May 7, 2020
 *      Author: henryd
 */

#include <Arduino.h>
#include <XPT2046_Touchscreen.h>
#include "../../src/SSD2119.h"
#include <SPI.h>

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// need to define ALL SPI devices on 3.5" TFT board
// Other devices
const int sdSelect = 4;
const int tpSelect = 6;
const int fSelect = 5;
const int sSelect = 10;
#define TIRQ_PIN  2
XPT2046_Touchscreen ts(tpSelect, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling
SSD2119 tft(12,10,11);  //RS CS RST on Due

void setup() {
  Serial.begin(115200);
  Serial.println("Touch");
  // deselect unused devices
  pinMode(sdSelect,OUTPUT);
  digitalWrite(sdSelect,HIGH);
  pinMode(fSelect,OUTPUT);
  digitalWrite(fSelect,HIGH);
//  pinMode(sSelect,OUTPUT);
//  digitalWrite(sSelect,HIGH);
  tft.initLCD();
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  ts.begin();
  ts.setRotation(3);
  tft.setCursor(20,20);
  tft.fillCircle(20,20,20,YELLOW);
}

boolean wastouched = true;

void loop() {
	bool istouched = ts.touched();
	if (istouched) {
		TS_Point p = ts.getPoint();
		if (!wastouched) {
			Serial.print("X: ");Serial.print(p.x);
			Serial.print(" Y: ");Serial.println(p.y);
		}
	} else {
	    if (wastouched) {
	    }
	}
	wastouched = istouched;
	delay(100);
}
