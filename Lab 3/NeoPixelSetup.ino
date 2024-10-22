/* NeoPixelSetup.ino
 * 
 * A minimum sketch to setup NeoPixels.
 * 
 * Seth McNeill
 * 2022 September 20
 */

#include <Adafruit_NeoPixel.h> // NeoPixel library

#define NEO_PIN           17  // WARNING! THIS IS GPIO NOT D NUMBER for NeoPixels
#define NEO_COUNT         18  // number of NeoPixels

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  // Setup NeoPixels
  strip.begin();
  strip.clear();  // Set all pixel values to zero
  strip.show();   // Write values to the pixels
}

void loop() {
  for(int ii = 0; ii < NEO_COUNT; ii++) {
    // set all the pixels to purple (R,G,B) values 0-255
    strip.setPixelColor(ii, strip.Color(120,0,120));
  }
  strip.show();
  delay(1000);
  // turn off all the LEDs
  strip.clear();
  strip.show();
  delay(1000);
}
